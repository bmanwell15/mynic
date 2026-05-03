#ifndef OBJECTS_H
#define OBJECTS_H

#include "Mynic.h"

namespace adapters::objects {
    void printSchema(std::shared_ptr<ASTNode>& rootNode);
    void printPacket(std::shared_ptr<ASTPacket>& packet);
    void printSegment(std::shared_ptr<ASTPacket>& packet);
}

#endif