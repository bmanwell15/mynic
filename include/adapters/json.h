#ifndef JSON_H
#define JSON_H

#include "../core/Interpreter.h"

namespace adapters::json {
    std::string encode(const DecodedPacket& decodedPacket);
}

#endif