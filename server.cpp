#include "message_handler.h"
#include "helper_functions.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <utility>

#define PORT 8080

std::map<uint32_t, std::set<std::pair<uint32_t, uint32_t>>> database;

std::mutex db_mutex;
std::mutex cout_mutex;

int error = 0;
int success = 1;
int courrpt = 2;

void safe_print(std::string msg){
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout<<msg<<std::endl;
    return ;
}

void handle_file_upload(Message& m, int socket){
    uint32_t filename = convert(m.payload, 0);
    uint32_t addr = convert(m.payload, 4);
    uint32_t port = convert(m.payload, 8);

    {
        std::lock_guard<std::mutex> lock(db_mutex);
        database[filename].insert({addr, port});
    }

    send(socket, &success, sizeof(success), 0);

    return ;
}

void handle_file_delete(Message& m, int socket){
    uint32_t filename = convert(m.payload, 0);
    uint32_t addr = convert(m.payload, 4);
    uint32_t port = convert(m.payload, 8);

    {
        std::lock_guard<std::mutex> lock(db_mutex);
        if(database[filename].find({addr, port}) != database[filename].end())
            database[filename].erase(database[filename].find({addr, port}));
    }

    send(socket, &success, sizeof(success), 0);

    return ;
}

void handle_file_download(Message& m, int socket){
    uint32_t filename = convert(m.payload, 0);

    std::vector<uint8_t> buffer;
    int j = 0;
    {
        std::lock_guard<std::mutex> lock(db_mutex);
        size_t required_size = 8 * database[filename].size();
        buffer.resize(required_size);
        for(auto i:database[filename]){
            std::memcpy(buffer.data() + 8 * j, &(i.first), sizeof(i.first));
            std::memcpy(buffer.data() + 8 * j + 4, &(i.second), sizeof(i.second));
            j++;
        }
    }
    
    if(buffer.size() == 0){
        send(socket, &error, sizeof(error), 0);
        return ;
    }

    send(socket, buffer.data(), buffer.size(), 0);

    return ;
}

void handle_request(int socket){
    std::vector<uint8_t> buffer(20);
    int bytes_read = read(socket, buffer.data(), 20);
    buffer.resize(bytes_read);
    bool check = validate_message(buffer);
    if(!check){
        safe_print("Corrupted Message Recieved");
        send(socket, &courrpt, sizeof(courrpt), 0);
        close(socket);
        return ;
    }

    Message curr = Message::deserialize(buffer);
    int type = static_cast<int>(curr.type);
    
    switch(type){
        case 0:
            safe_print("Recieved a request for file upload");
            handle_file_upload(curr, socket);
            safe_print("Completed request for file upload");
            break;
        case 1:
            safe_print("Recieved a request for file delete");
            handle_file_delete(curr, socket);
            safe_print("Completed request for file delete");
            break;
        case 2:
            safe_print("Recieved a request for file download");
            handle_file_download(curr, socket);
            safe_print("Completed request for file download");
            break;
        default:
            send(socket, &error, sizeof(error), 0);
            break;
    }

    close(socket);

    return ;
}

int main(){
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
    address.sin_port = htons(PORT);
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

    safe_print("Server Started");

    while(true){
        int new_socket = accept(server_fd, (struct sockaddr*) &address, &address_len);
        if(new_socket < 0){
            perror("Accept Failed!!!");
            exit(EXIT_FAILURE);
        }

        std::thread t(handle_request, new_socket);
        t.detach();
    }

    return 0;
}