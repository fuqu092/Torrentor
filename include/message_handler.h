#pragma once
#include "message.h"

Message generate_upload_file_message(const uint32_t filename, const uint32_t port);

Message generate_delete_file_message(const uint32_t filename, const uint32_t port);

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