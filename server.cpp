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

#define PORT 8080

std::map<uint8_t, std::set<uint8_t>> database;
std::mutex db_mutex;

void handle_file_upload(const uint8_t filename, const uint8_t port){
    std::lock_guard<std::mutex> lock(db_mutex);
    database[filename].insert(port);
    return ;
}

void handle_file_delete(const uint8_t filename, const uint8_t port){
    std::lock_guard<std::mutex> lock(db_mutex);
    if(database[filename].find(port) != database[filename].end())
        database[filename].erase(database[filename].find(port));
    return ;
}

void handle_file_download(const uint8_t filename, int socket){
    std::vector<uint8_t> buffer;
    {
        std::lock_guard<std::mutex> lock(db_mutex);
        for(auto i:database[filename])
            buffer.push_back(i);
    }
    send(socket, buffer.data(), buffer.size(), 0);
    return ;
}

void handle_request(int* socket_ptr){
    char* error_msg = "Error";
    int socket = *socket_ptr;
    free(socket_ptr);
    std::vector<uint8_t> buffer(20);
    int bytes_read = read(socket, buffer.data(), 20);
    Message curr = Message::deserialize(buffer);
    switch(curr.type){
        case 0:
            handle_file_upload(curr.payload[0], curr.payload[1]);
            break;
        case 1:
            handle_file_delete(curr.payload[0], curr.payload[1]);
            break;
        case 2:
            handle_file_download(curr.payload[0], socket);
            break;
        default:
            send(socket, error_msg, strlen(error_msg), 0);
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

    while(true){
        int new_socket = accept(server_fd, (struct sockaddr*) &address, &address_len);
        if(new_socket < 0){
            perror("Accept Failed!!!");
            exit(EXIT_FAILURE);
        }
        
        int* socket_ptr = &new_socket;
        std::thread t(handle_request, socket_ptr);
        t.detach();
    }
    return 0;
}