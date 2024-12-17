#include "message.h"
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>

Message::Message(const uint8_t type, const std::vector<uint8_t>& payload) : type(type), payload(payload){
    this->len = static_cast<uint32_t>(1 + payload.size());
}

std::vector<uint8_t> Message::serialize() const{
    std::vector<uint8_t> buffer(5 + payload.size());

    std::memcpy(buffer.data(), &len, sizeof(len));
    std::memcpy(buffer.data()+4, &type, sizeof(type));
    std::memcpy(buffer.data()+5, payload.data(), payload.size());

    return buffer;
}

Message Message::deserialize(const std::vector<uint8_t>& buffer){
    if(buffer.size() < 5)
        throw std::runtime_error("Invalid Message");
    
    uint32_t len = 0;
    for(int i=0;i<4;i++)
        len = len | (buffer[i] << (i * 8));
    
    if(buffer.size() - 4 != len)
        throw std::runtime_error("Invalid Payload Length");

    uint8_t type;
    std::memcpy(&type, buffer.data()+4, sizeof(type));
    
    std::vector<uint8_t> payload(buffer.begin()+5, buffer.end());

    return Message(type, payload);
}