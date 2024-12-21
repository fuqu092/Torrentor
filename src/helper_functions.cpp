#include "helper_functions.h"
#include <sys/socket.h>

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