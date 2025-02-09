#include "message_handler.h"
#include "helper_functions.h"
#include <cstring>
#include <string>
#include <iostream>

Message generate_upload_file_message(const std::string filename, const uint32_t num_bitfields, uint32_t addr, const uint32_t port){
    int msg_len = 8 + filename.length() + 8;
    uint32_t filename_len = filename.length();
    std::vector<uint8_t> payload(msg_len);
    std::memcpy(payload.data(), &num_bitfields, sizeof(num_bitfields));
    std::memcpy(payload.data()+4, &filename_len, sizeof(filename_len));
    std::memcpy(payload.data()+8, filename.c_str(), filename_len);
    std::memcpy(payload.data()+8+filename_len, &addr, sizeof(addr));
    std::memcpy(payload.data()+12+filename_len, &port, sizeof(port));
    Message m(0, payload);
    return m;
}

Message generate_delete_file_message(const std::string filename, const uint32_t addr, const uint32_t port){
    uint32_t msg_len = 4 + filename.length() + 8;
    uint32_t filename_len = filename.length();
    std::vector<uint8_t> payload(msg_len);
    std::memcpy(payload.data(), &filename_len, sizeof(filename_len));
    std::memcpy(payload.data()+4, filename.c_str(), filename_len);
    std::memcpy(payload.data()+4+filename_len, &addr, sizeof(addr));
    std::memcpy(payload.data()+8+filename_len, &port, sizeof(port));
    Message m(1, payload);
    return m;
}

Message generate_download_file_message(const std::string filename){
    uint32_t msg_len = 4 + filename.length();
    uint32_t filename_len = filename.length();
    std::vector<uint8_t> payload(msg_len);
    std::memcpy(payload.data(), &filename_len, sizeof(filename_len));
    std::memcpy(payload.data()+4, filename.c_str(), filename_len);
    Message m(2, payload);
    return m;
}

Message generate_handshake_message(){
    Message m(3);
    return m;
}

Message generate_file_request_message(const std::string filename){
    uint32_t msg_len = 4 + filename.length();
    uint32_t filename_len = filename.length();
    std::vector<uint8_t> payload(msg_len);
    std::memcpy(payload.data(), &filename_len, sizeof(filename_len));
    std::memcpy(payload.data()+4, filename.c_str(), filename_len);
    Message m(4, payload);
    return m;
}

Message generate_bitfields_message(const std::vector<uint8_t>& bitfield){
    Message m(5, bitfield);
    return m;
}

Message generate_interested_message(){
    Message m(6);
    return m;
}

Message generate_not_interested_message(){
    Message m(7);
    return m;
}

Message generate_choke_message(){
    Message m(8);
    return m;
}

Message generate_unchoke_message(){
    Message m(9);
    return m;
}

Message generate_piece_request_message(const uint32_t piece_index){
    std::vector<uint8_t> payload(4);
    std::memcpy(payload.data(), &piece_index, sizeof(piece_index));
    Message m(10, payload);
    return m;
}

Message generate_piece_message(const uint32_t piece_index, const std::vector<uint8_t>& content){
    std::vector<uint8_t> payload(4 + content.size());
    std::memcpy(payload.data(), &piece_index, sizeof(piece_index));
    std::memcpy(payload.data() + 4, content.data(), content.size());
    Message m(11, payload);
    return m;
}

Message generate_close_message(){
    Message m(12);
    return m;
}

bool validate_message(std::vector<uint8_t>& buffer){
    if(buffer.size() < 5)
        return false;
    
    uint32_t len = 0;
    for(int i=0;i<4;i++)
        len = len | (buffer[i] << (i * 8));
    
    if(buffer.size() - 4 != len)
        return false;

    int type = static_cast<int> (buffer[4]);

    switch (type){
    case 0:
        return validate_upload_file_message(buffer);
        break;
    case 1:
        return validate_delete_file_message(buffer);
        break;
    case 2:
        return validate_download_file_message(buffer);
        break;
    case 3:
        return validate_handshake_message(buffer);
        break;
    case 4:
        return validate_file_request_message(buffer);
        break;
    case 5:
        return validate_bitfields_message(buffer);
        break;
    case 6:
        return validate_interested_message(buffer);
        break;
    case 7:
        return validate_not_interested_message(buffer);
        break;
    case 8:
        return validate_choke_message(buffer);
        break;
    case 9:
        return validate_unchoke_message(buffer);
        break;
    case 10:
        return validate_piece_request_message(buffer);
        break;
    case 11:
        return validate_piece_message(buffer);
        break;
    case 12:
        return validate_close_message(buffer);
        break;
    default:
        return false;
        break;
    }

    return false;
}

bool validate_upload_file_message(std::vector<uint8_t>& buffer){
    uint32_t filename_len = convert(buffer, 9);
    if(buffer.size() != 21 + filename_len)
        return false;
    return true;
}

bool validate_delete_file_message(std::vector<uint8_t>& buffer){
    uint32_t filename_len = convert(buffer, 5);
    if(buffer.size() != 17 + filename_len)
        return false;
    return true;
}

bool validate_download_file_message(std::vector<uint8_t>& buffer){
    uint32_t filename_len = convert(buffer, 5);
    if(buffer.size() != 9 + filename_len)
        return false;
    return true;
}

bool validate_handshake_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 5)
        return false;
    return true;
}

bool validate_file_request_message(std::vector<uint8_t>& buffer){
    uint32_t filename_len = convert(buffer, 5);
    if(buffer.size() != 9 + filename_len)
        return false;
    return true;
}

bool validate_bitfields_message(std::vector<uint8_t>& buffer){
    return true;
}

bool validate_interested_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 5)
        return false;
    return true;
}

bool validate_not_interested_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 5)
        return false;
    return true;
}

bool validate_choke_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 5)
        return false;
    return true;
}

bool validate_unchoke_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 5)
        return false;
    return true;
}

bool validate_piece_request_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 9)
        return false;
    return true;
}

bool validate_piece_message(std::vector<uint8_t>& buffer){
    if(buffer.size() < 4)
        return false;
    return true;
}

bool validate_close_message(std::vector<uint8_t>& buffer){
    if(buffer.size() != 5)
        return false;
    return true;
}