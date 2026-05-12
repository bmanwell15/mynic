#ifndef OBJECTS_H
#define OBJECTS_H

#include "Mynic.h"

namespace adapters::objects {
    // Prints the loaded AST schema for all definitions.
    void printSchema(std::shared_ptr<ASTNode>& rootNode);

    // Prints the structure of a packet definition.
    void printPacket(std::shared_ptr<ASTPacket>& packet);

    // Prints the structure of a segment definition.
    void printSegment(std::shared_ptr<ASTPacket>& packet);
}

#endif