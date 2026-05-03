#include "objects.h"

void printPacketField(std::shared_ptr<ASTField> field, blockDepth_t indent) {
    std::string indentStr = std::string(indent * PRINT_INDENT_SIZE, ' ');
    if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<ASTPrimitiveValue>(field);
        std::string printSizeInBits = (primField->datatype == "string") ? "?" : std::to_string(primField->sizeInBits);
        std::cout << indentStr << primField->datatype << ' ' << primField->name << " (" << printSizeInBits << " bits);" << std::endl;
    } else if (field->type == NodeType::BITFIELD) {
        auto bitField = std::static_pointer_cast<ASTBitfield>(field);
        std::cout << indentStr << "Bitfield " << bitField->name << " {" << std::endl;
        for (const auto& subfield : bitField->subfields) {
            printPacketField(subfield, indent + 1);
        }
        std::cout << indentStr << "}" << std::endl;
    } else if (field->type == NodeType::UNION) {
        auto unionfield = std::static_pointer_cast<ASTUnion>(field);
        std::cout << indentStr << "Union " << unionfield->name << " {" << std::endl;
        for (const auto& subfield : unionfield->subfields) {
            printPacketField(subfield, indent + 1);
        }
        std::cout << indentStr << "}" << std::endl;
    } else if (field->type == NodeType::ARRAY) {
        auto arrayfield = std::static_pointer_cast<ASTArray>(field);
        std::string bitSizeStr = arrayfield->length != 0 ? std::to_string(arrayfield->elementSchema->sizeInBits * arrayfield->length) : "?";
        std::string insideBracketStr = arrayfield->length == 0 ? "" : std::to_string(arrayfield->length);
        std::cout << indentStr << arrayfield->elementSchema->datatype << ' ' << arrayfield->elementSchema->name << "[" << insideBracketStr << "] (" << bitSizeStr << " bits);" << std::endl;
    } else if (field->type == NodeType::SEGMENT) {
        auto segDef = std::static_pointer_cast<ASTPacket>(field);
        std::cout << indentStr << segDef->name << " {" << std::endl;
        for (const auto& subfield : segDef->fields) {
            printPacketField(subfield, indent + 1);
        }
        std::cout << indentStr << "}" << std::endl;
    }
}

void adapters::objects::printPacket(std::shared_ptr<ASTPacket>& packet) {
    std::cout << packet->name << ':' << std::endl;
    for (const auto& pktField : packet->fields) {
        printPacketField(pktField, 1);
    }
}

void adapters::objects::printSegment(std::shared_ptr<ASTPacket>& packet) {
    std::cout << packet->name << ':' << std::endl;
    for (const auto& pktField : packet->fields) {
        printPacketField(pktField, 1);
    }
}

void adapters::objects::printSchema(std::shared_ptr<ASTNode>& rootNode) {
    for (const auto& [name, field] : rootNode->properties) {
        if (field && field->type == NodeType::PACKET) {
            auto packet = std::static_pointer_cast<ASTPacket>(field);
            printPacket(packet);
            std::cout << std::endl;
        }
    }
}
