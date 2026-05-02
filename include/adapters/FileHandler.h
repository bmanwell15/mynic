#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <iostream>
#include <fstream>

namespace adapters::file {
    std::string read(std::string filepath);
    bool write(std::string filepath, std::string content);
    bool append(std::string filepath, std::string content);
};

#endif