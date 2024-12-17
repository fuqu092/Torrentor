#include "message_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <string.h>
#include <map>
#include <set>
#include <mutex>
#include <iostream>
#include <cstring>

#define PORT 8080

std::map<uint32_t, std::set<uint32_t>> database;
std::mutex db_mutex;

int success = 1;
int error = 0;

uint32_t convert(std::vector<uint8_t>& payload, int j){
    uint32_t res = 0;
    for(int i=0;i<4;i++)
        res = res | (payload[i+j] << (i * 8));
    
    return res;
}

void handle_file_upload(Message& m, int socket){
    uint32_t filename = convert(m.payload, 0);
    uint32_t port = convert(m.payload, 4);

    std::lock_guard<std::mutex> lock(db_mutex);
    database[filename].insert(port);

    send(socket, &success, sizeof(success), 0);

    return ;
}

void handle_file_delete(Message& m, int socket){
    uint32_t filename = convert(m.payload, 0);
    uint32_t port = convert(m.payload, 4);

    std::lock_guard<std::mutex> lock(db_mutex);
    if(database[filename].find(port) != database[filename].end())
        database[filename].erase(database[filename].find(port));

    send(socket, &success, sizeof(success), 0);

    return ;
}

void handle_file_download(Message& m, int socket){
    uint32_t filename = convert(m.payload, 0);

    std::vector<uint8_t> buffer;
    int j = 0;
    {
        size_t required_size = 4 * database[filename].size();
        buffer.resize(required_size);
        std::lock_guard<std::mutex> lock(db_mutex);
        for(auto i:database[filename]){
            std::memcpy(buffer.data() + 4 * j, &i, sizeof(i));
            j++;
        }
    }
    
    if(buffer.size() == 0){
        send(socket, &error, sizeof(error), 0);
        return ;
    }

    send(socket, buffer.data(), buffer.size(), 0);
    // send(socket, &success, sizeof(success), 0);

    return ;
}

void handle_request(int socket){
    std::vector<uint8_t> buffer(20);
    int bytes_read = read(socket, buffer.data(), 20);
    buffer.resize(bytes_read);
    Message curr = Message::deserialize(buffer);

    int type = static_cast<int>(curr.type);
    
    switch(type){
        case 0:
            std::cout<<"Recieved a request for file upload"<<std::endl;
            handle_file_upload(curr, socket);
            std::cout<<"Completed request for file upload"<<std::endl;
            break;
        case 1:
            std::cout<<"Recieved a request for file delete"<<std::endl;
            handle_file_delete(curr, socket);
            std::cout<<"Completed request for file delete"<<std::endl;
            break;
        case 2:
            std::cout<<"Recieved a request for file download"<<std::endl;
            handle_file_download(curr, socket);
            std::cout<<"Completed request for file download"<<std::endl;    
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

    std::cout<<"Server Started"<<std::endl;

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