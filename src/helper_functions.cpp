#include "helper_functions.h"
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>

uint32_t convert(std::vector<uint8_t>& payload, int j){
    uint32_t res = 0;
    for(int i=0;i<4;i++)
        res = res | ((static_cast<uint32_t>(payload[i+j])) << (i * 8));
    
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
    pclose(fp);

    ip.erase(0, ip.find_first_not_of(" \t\n\r"));
    ip.erase(ip.find_last_not_of(" \t\n\r") + 1);

    return inet_addr(ip.c_str());
}

uint32_t get_num_bitfields(std::string filepath){
    int piece_size = 100;

    std::ifstream file(filepath, std::ios::binary);

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    uint32_t fileSize32 = static_cast<uint32_t>(fileSize);

    file.close();

    uint32_t num_bitfields = (fileSize32 + piece_size - 1)/piece_size; 

    return num_bitfields;
}

std::vector<uint8_t> generate_piece_payload(const std::string filepath, uint32_t piece_index) {
    std::streampos start_byte = piece_index * 100;
    std::ifstream file(filepath, std::ios::binary);

    file.seekg(start_byte);
    std::vector<uint8_t> buffer(100);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    uint32_t bytesRead = file.gcount();
    buffer.resize(bytesRead);
    
    return buffer;
}

void write_to_file(std::string filepath, uint32_t piece_index, Message m){
    uint32_t skip_bytes = piece_index * 100;
    std::ofstream file(filepath, std::ios::binary | std::ios::in | std::ios::out);
    file.seekp(skip_bytes);

    const char* payload_data = reinterpret_cast<const char*>(&m.payload[4]);
    size_t data_size = m.payload.size() - 4;
    file.write(payload_data, data_size);

    file.close();
}