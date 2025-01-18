#pragma once
#include "message.h"

Message generate_upload_file_message(const uint32_t filename, const uint32_t addr, const uint32_t port);

Message generate_delete_file_message(const uint32_t filename, const uint32_t addr, const uint32_t port);

Message generate_download_file_message(const uint32_t filename);

Message generate_handshake_message();

Message generate_file_request_message(const uint32_t filename);

Message generate_bitfields_message(const std::vector<uint8_t>& bitfield);

Message generate_interested_message();

Message generate_not_interested_message();

Message generate_choke_message();

Message generate_unchoke_message();

Message generate_piece_request_message(const uint32_t piece_index);

Message generate_piece_message(const uint32_t piece_index, const std::vector<uint8_t>& content = {});

Message generate_close_message();

bool validate_message(std::vector<uint8_t>& buffer);

bool validate_upload_file_message(std::vector<uint8_t>& buffer);

bool validate_delete_file_message(std::vector<uint8_t>& buffer);

bool validate_download_file_message(std::vector<uint8_t>& buffer);

bool validate_handshake_message(std::vector<uint8_t>& buffer);

bool validate_file_request_message(std::vector<uint8_t>& buffer);

bool validate_bitfields_message(std::vector<uint8_t>& buffer);

bool validate_interested_message(std::vector<uint8_t>& buffer);

bool validate_not_interested_message(std::vector<uint8_t>& buffer);

bool validate_choke_message(std::vector<uint8_t>& buffer);

bool validate_unchoke_message(std::vector<uint8_t>& buffer);

bool validate_piece_request_message(std::vector<uint8_t>& buffer);

bool validate_piece_message(std::vector<uint8_t>& buffer);

bool validate_close_message(std::vector<uint8_t>& buffer);