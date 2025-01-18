#pragma once
#include "message.h"
#include <stdint.h>

uint32_t convert(std::vector<uint8_t>& payload, int j);

std::vector<std::pair<uint32_t, uint32_t>> get_peer_info(std::vector<uint8_t>& buffer, int bytes_read);

void send_message(Message& m, int socket);

uint32_t get_personal_ip();