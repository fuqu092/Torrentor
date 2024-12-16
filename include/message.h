#pragma once
#include <cstdint>
#include <vector>

class Message{
    public:

    uint32_t len;
    uint8_t type;
    std::vector<uint8_t> payload;

    explicit Message(const uint8_t type, const std::vector<uint8_t>& payload = {});
    ~Message() = default;
    std::vector<uint8_t> serialize() const;
    static Message deserialize(const std::vector<uint8_t>& buffer);
};