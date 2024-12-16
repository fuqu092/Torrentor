#include "message_handler.h"
#include <cstring>

Message generate_upload_file_message(const uint8_t filename, const uint8_t port){
    std::vector<uint8_t> payload(2);
    std::memcpy(payload.data(), &filename, sizeof(filename));
    std::memcpy(payload.data()+1, &port, sizeof(port));
    Message m(0, payload);
    return m;
}

Message generate_delete_file_message(const uint8_t filename, const uint8_t port){
    std::vector<uint8_t> payload(2);
    std::memcpy(payload.data(), &filename, sizeof(filename));
    std::memcpy(payload.data()+1, &port, sizeof(port));
    Message m(1, payload);
    return m;
}

Message generate_download_file_message(const uint8_t filename){
    std::vector<uint8_t> payload(1);
    std::memcpy(payload.data(), &filename, sizeof(filename));
    Message m(2, payload);
    return m;
}

Message generate_handshake_message(){
    Message m(3);
    return m;
}

Message generate_file_request_message(const uint8_t filename){
    std::vector<uint8_t> payload(1);
    std::memcpy(payload.data(), &filename, sizeof(filename));
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