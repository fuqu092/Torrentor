#include "message_handler.h"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <cstring>
#include <mutex>

std::map<uint32_t, bool> uploading;
std::mutex up_mutex;

uint32_t convert(std::vector<uint8_t>& payload, int j){
    uint32_t res = 0;
    for(int i=0;i<4;i++)
        res = res | (payload[i+j] << (i * 8));
    
    return res;
}

std::vector<uint32_t> get_peer_ports(std::vector<uint8_t>& buffer, int bytes_read){
    std::vector<uint32_t> peer_ports;
    for(int i=0;i<bytes_read;i+=4){
        uint32_t res = 0;
        for(int j=0;j<4;j++)
            res = res | (buffer[i+j] << (j * 8));
        peer_ports.push_back(res);
    }

    return peer_ports; 
}

void send_message(Message& m, int socket){
    std::vector<uint8_t> buffer = m.serialize();
    send(socket, buffer.data(), buffer.size(), 0);
    return ;
}

void upload_file(uint32_t filename, uint32_t port){
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        perror("Socket Creation Failed!!!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connect_status = connect(client_fd, (struct sockaddr*) &server_address, server_address_len);
    if(connect_status < 0){
        perror("Connection Failed!!!");
        exit(EXIT_FAILURE);
    }

    Message m = generate_upload_file_message(filename, port);
    send_message(m, client_fd);

    int res;
    int bytes_read = read(client_fd, &res, sizeof(res));
    
    if(res == 1){
        std::lock_guard<std::mutex> lock(up_mutex);
        uploading[filename] = true;
        std::cout<<"Server communicated about uploading file "<<filename<<std::endl;
        std::cout<<"Started seeding file: "<<filename<<std::endl;
    }
    else
        std::cout<<"Error in server"<<std::endl;

    close(client_fd);

    return ;
}

void upload_to_peer(std::vector<uint8_t>& bitfields, uint32_t filename, int socket){
    std::vector<uint8_t> buffer(1024);
    std::vector<uint8_t> temp;

    Message m = generate_bitfields_message(bitfields);
    send_message(m, socket);

    int bytes_read = read(socket, buffer.data(), 1024);
    temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
    m = Message::deserialize(temp);

    if(static_cast<int>(m.type) == 7){
        close(socket);
        return ;
    }

    while(true){
        {
            std::lock_guard<std::mutex> lock(up_mutex);
            if(!uploading[filename]){
                m = generate_close_message();
                send_message(m, socket);
                break;
            }
        }

        m = generate_unchoke_message();
        send_message(m, socket);

        int bytes_read = read(socket, buffer.data(), 1024);
        temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
        m = Message::deserialize(temp);

        if(static_cast<int>(m.type) == 7)
            break;

        uint32_t piece_index = convert(m.payload, 0);
        m = generate_piece_message(piece_index);
        send_message(m, socket);
    }

    close(socket);

    return ;
}

void seed(uint32_t port){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("Socket Creation Failed!!!");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(server_fd, (struct sockaddr*) &address, address_len);
    if(bind_status < 0){
        perror("Binding Failed!!!");
        exit(EXIT_FAILURE);
    }

    int listen_status = listen(server_fd, 50);
    if(listen_status < 0){
        perror("Listening Failed!!!");
        exit(EXIT_FAILURE);
    }

    std::vector<uint8_t> buffer(1024);
    std::vector<uint8_t> bitfields(100, 1);

    while(true){
        int new_socket = accept(server_fd, (struct sockaddr*) &address, &address_len);
        if(new_socket < 0){
            perror("Accept Failed!!!");
            exit(EXIT_FAILURE);
        }

        int bytes_read = read(new_socket, buffer.data(), 1024);
        std::vector<std::uint8_t> temp(buffer.begin(), buffer.begin() + bytes_read);
        Message m = Message::deserialize(temp);

        m = generate_handshake_message();
        send_message(m, new_socket);

        bytes_read = read(new_socket, buffer.data(), 1024);
        temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
        m = Message::deserialize(temp);

        uint32_t filename = convert(m.payload, 0);

        bool check = false;

        {
            std::lock_guard<std::mutex> lock(up_mutex);
            if(uploading[filename])
                check = true;
        }

        if(check){
            std::thread t(upload_to_peer, std::ref(bitfields), filename, new_socket);
            t.detach();
        }
        else{
            m = generate_close_message();
            send_message(m, new_socket);
            close(new_socket);
        }
    }

    close(server_fd);

    return ;
}

void delete_file(uint32_t filename, uint32_t port){
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        perror("Socket Creation Failed!!!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connect_status = connect(client_fd, (struct sockaddr*) &server_address, server_address_len);
    if(connect_status < 0){
        perror("Connection Failed!!!");
        exit(EXIT_FAILURE);
    }

    Message m = generate_delete_file_message(filename, port);
    send_message(m, client_fd);

    int res;
    int bytes_read = read(client_fd, &res, sizeof(res));

    if(res == 1){
        std::lock_guard<std::mutex> lock(up_mutex);
        uploading[filename] = false;
        std::cout<<"Server communicated about not uploading file "<<filename<<std::endl;
        std::cout<<"Stopped seeding file: "<<filename<<std::endl;
    }
    else
        std::cout<<"Error in server"<<std::endl;

    close(client_fd);

    return ;
}

void download_from_peer(std::vector<uint8_t>& bitfields, std::mutex& file_mutex, uint32_t filename, uint32_t port){
    int new_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(new_socket < 0){
        perror("Socket Creation Failed!!!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connect_status = connect(new_socket, (struct sockaddr*) &server_address, server_address_len);
    if(connect_status < 0){
        perror("Connection Failed!!!");
        exit(EXIT_FAILURE);
    }

    std::vector<uint8_t> buffer(1024);
    std::vector<std::uint8_t> temp;

    Message m = generate_handshake_message();
    send_message(m, new_socket);

    int bytes_read = read(new_socket, buffer.data(), 1024);
    temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
    m = Message::deserialize(temp);

    m = generate_file_request_message(filename);
    send_message(m, new_socket);

    bytes_read = read(new_socket, buffer.data(), 1024);
    temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
    m = Message::deserialize(temp);

    if(static_cast<int> (m.type) == 13){
        close(new_socket);
        return ;
    }

    std::vector<uint8_t> peer_bitfields = m.payload;

    bool check = false;

    for(int i=0;i<bitfields.size();i++){
        std::lock_guard<std::mutex> lock(file_mutex);
        if(static_cast<int> (bitfields[i]) == 0 && static_cast<int> (peer_bitfields[i]) == 1){
            check = true;
            break;
        }
    }

    if(check){
        m = generate_interested_message();
        send_message(m, new_socket);
    }
    else{
        m = generate_not_interested_message();
        send_message(m, new_socket);
        close(new_socket);
        return ;
    }

    while(true){
        int bytes_read = read(new_socket, buffer.data(), 1024);
        temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
        m = Message::deserialize(temp);

        if(static_cast<int> (m.type) == 13)
            break;

        uint32_t piece_index = -1;

        for(int i=0;i<bitfields.size();i++){
            std::lock_guard<std::mutex> lock(file_mutex);
            if(static_cast<int> (bitfields[i]) == 0 && static_cast<int> (peer_bitfields[i]) == 1){
                piece_index = i;
                bitfields[i] = 2;
                break;
            }
        }

        if(piece_index == (uint32_t)-1){
            m = generate_not_interested_message();
            send_message(m, new_socket);
            break;
        }
        else{
            m = generate_piece_request_message(piece_index);
            send_message(m, new_socket);
        }

        bytes_read = read(new_socket, buffer.data(), 1024);
        temp = std::vector<uint8_t> (buffer.begin(), buffer.begin() + bytes_read);
        m = Message::deserialize(temp);

        {
            std::lock_guard<std::mutex> lock(file_mutex);
            bitfields[piece_index] = 1;
        }
    }

    close(new_socket);
    
    return ;
}

void download_file(uint32_t filename){
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        perror("Socket Creation Failed!!!");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connect_status = connect(client_fd, (struct sockaddr*) &server_address, server_address_len);
    if(connect_status < 0){
        perror("Connection Failed!!!");
        exit(EXIT_FAILURE);
    }

    Message m = generate_download_file_message(filename);
    send_message(m, client_fd);

    std::vector<uint8_t> buffer(1024);
    int bytes_read = read(client_fd, buffer.data(), 1023);

    if(bytes_read == 4 && convert(buffer, 0) == 0){
        std::cout<<"No one has the file"<<std::endl;
        close(client_fd);
        return ;
    }

    std::vector<uint32_t> peer_ports = get_peer_ports(buffer, bytes_read);

    std::cout<<"Port retrieval complete"<<std::endl;

    close(client_fd);

    std::vector<uint8_t> bitfields(100, 0);
    std::mutex file_mutex;

    std::vector<std::thread> downloaders;
    
    for(auto i:peer_ports)
        downloaders.push_back(std::thread(download_from_peer, std::ref(bitfields), std::ref(file_mutex), filename, i));

    for(auto& t:downloaders)
        t.join();

    bool check = true;

    for(int i=0;i<bitfields.size();i++){
        if(static_cast<int> (bitfields[i]) == 0){
            check = false;
            break;
        }
    }

    if(check)
        std::cout<<"File: "<<filename<<" downloaded successfully."<<std::endl;
    else
        std::cout<<"File: "<<filename<<" couldn't be downloaded."<<std::endl;

    return ;
}

int main(){
    uint32_t port;
    std::cout<<"Enter the port number: ";
    std::cin>>port;

    std::thread t(seed, port);
    t.detach();

    while(true){
        int choice;
        std::cout<<"Press 1 to Upload a file, 2 to stop uploading a file and 3 for downloading a file, 4 to shutdown"<<std::endl;;
        std::cin>>choice;
        if(choice == 1){
            uint32_t filename;
            std::cout<<"Enter the file index: ";
            std::cin>>filename;
            bool check = false;
            {
                std::lock_guard<std::mutex> lock(up_mutex);
                if(uploading[filename])
                    check = true;
            }
            if(check){
                std::cout<<"You are already uploading the file"<<std::endl;
                continue;
            }
            upload_file(filename, port);
        }
        else if(choice == 2){
            uint32_t filename;
            std::cout<<"Enter the file index: ";
            std::cin>>filename;
            bool check = false;
            {
                std::lock_guard<std::mutex> lock(up_mutex);
                if(!uploading[filename])
                    check = true;
            }
            if(check){
                std::cout<<"You are not Uploading the file"<<std::endl;
                continue;
            }
            delete_file(filename, port);
        }
        else if(choice == 3){
            uint32_t filename;
            std::cout<<"Enter the file index: ";
            std::cin>>filename;
            std::thread t(download_file, filename);
            t.detach();
        }
        else if(choice == 4)
            break;
        else
            std::cout<<"Enter a valid choice"<<std::endl;
    }

    return 0;
}