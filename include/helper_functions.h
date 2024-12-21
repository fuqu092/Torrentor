#pragma once
#include "message.h"
#include <stdint.h>

uint32_t convert(std::vector<uint8_t>& payload, int j);

std::vector<uint32_t> get_peer_ports(std::vector<uint8_t>& buffer, int bytes_read);

void send_message(Message& m, int socket);