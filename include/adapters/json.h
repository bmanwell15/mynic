#ifndef JSON_H
#define JSON_H

#include "../core/Interpreter.h"

namespace adapters::json {
    // Encodes a decoded packet into a JSON string.
    std::string encode(const DecodedPacket& decodedPacket);
}

#endif