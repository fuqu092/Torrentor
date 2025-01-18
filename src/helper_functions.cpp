#include "helper_functions.h"
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>

uint32_t convert(std::vector<uint8_t>& payload, int j){
    uint32_t res = 0;
    for(int i=0;i<4;i++)
        res = res | (payload[i+j] << (i * 8));
    
    return res;
}

std::vector<std::pair<uint32_t, uint32_t>> get_peer_info(std::vector<uint8_t>& buffer, int bytes_read){
    std::vector<std::pair<uint32_t, uint32_t>> peer_info;
    for(int i=0;i<bytes_read;i+=8){
        uint32_t addr = 0;
        uint32_t port = 0;
        for(int j=0;j<4;j++)
            addr = addr | (buffer[i+j] << (j * 8));
        for(int j=0;j<4;j++)
            port = port | (buffer[i+j+4] << (j * 8));
        peer_info.push_back({addr, port});
    }

    return peer_info; 
}

void send_message(Message& m, int socket){
    std::vector<uint8_t> buffer = m.serialize();
    send(socket, buffer.data(), buffer.size(), 0);
    return ;
}

uint32_t get_personal_ip(){
    char buffer[128];
    std::string ip;

    FILE* fp = popen("hostname -I", "r");
    while (fgets(buffer, sizeof(buffer), fp) != nullptr)
        ip += buffer;
    fclose(fp);

    ip.erase(0, ip.find_first_not_of(" \t\n\r"));
    ip.erase(ip.find_last_not_of(" \t\n\r") + 1);

    return inet_addr(ip.c_str());
}