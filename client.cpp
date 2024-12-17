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

std::map<uint32_t, uint32_t> uploading;
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
        uploading[filename] = port;
        std::cout<<"Server communicated about uploading file "<<filename<<std::endl;
    }
    else
        std::cout<<"Error in server"<<std::endl;

    close(client_fd);

    return ;
}

void seed(uint32_t filename, uint32_t port){
    std::cout<<"Started seeding file "<<filename<<std::endl;
    while(true){
        {
            std::lock_guard<std::mutex> lock(up_mutex);
            if(uploading[filename] == 0)
                break;
        }
    }
    std::cout<<"Finished seeding file "<<filename<<std::endl;
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
        uploading[filename] = 0;
        std::cout<<"Server communicated about not uploading file "<<filename<<std::endl;
    }
    else
        std::cout<<"Error in server"<<std::endl;

    close(client_fd);

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

    return ;
}

int main(){
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
                if(uploading[filename] != 0)
                    check = true;
            }
            if(check){
                std::cout<<"You are already uploading the file"<<std::endl;
                continue;
            }
            uint32_t port;
            std::cout<<"Enter the port number: ";
            std::cin>>port;
            upload_file(filename, port);
            std::thread t(seed, filename, port);
            t.detach();
        }
        else if(choice == 2){
            uint32_t filename;
            std::cout<<"Enter the file index: ";
            std::cin>>filename;
            bool check = false;
            {
                std::lock_guard<std::mutex> lock(up_mutex);
                if(uploading[filename] == 0)
                    check = true;
            }
            if(check){
                std::cout<<"You are not Uploading the file"<<std::endl;
                continue;
            }
            delete_file(filename, uploading[filename]);
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