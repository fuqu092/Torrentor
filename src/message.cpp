#include "message.h"
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

Message::Message(const uint8_t type, const std::vector<uint8_t>& payload) : type(type), payload(payload){
    this->len = static_cast<uint32_t>(1 + payload.size());
}

std::vector<uint8_t> Message::serialize() const{
    std::vector<uint8_t> buffer(4);

    uint32_t len = htonl(this->len);
    std::memcpy(buffer.data(), &len, sizeof(len));
    buffer.push_back(type);
    buffer.insert(buffer.end(), payload.begin(), payload.end());

    return buffer;
}

Message Message::deserialize(const std::vector<uint8_t>& buffer){
    if(buffer.size() < 5)
        throw std::runtime_error("Invalid Message");
    
    uint32_t len;
    std::memcpy(&len, buffer.data(), sizeof(len));
    len = ntohl(len);

    uint8_t type = buffer[4];
    
    if(buffer.size() - 4 != len)
        throw std::runtime_error("Invalid Payload Length");

    std::vector<uint8_t> payload(buffer.begin()+5, buffer.end());

    return Message(type, payload);
}