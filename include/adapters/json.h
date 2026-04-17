#ifndef JSON_H
#define JSON_H

#include "../core/DecodedPacket.h"

namespace adapters::json {
    std::string encode(const DecodedPacket& decodedPacket);
}

#endif