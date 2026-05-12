#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <iostream>
#include <fstream>

namespace adapters::file {
    // Reads the full contents of a file and returns it as a string.
    std::string read(std::string filepath);

    // Writes content to a file, replacing any existing contents.
    bool write(std::string filepath, std::string content);

    // Appends content to the end of an existing file.
    bool append(std::string filepath, std::string content);
};

#endif