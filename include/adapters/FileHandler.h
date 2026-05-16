#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <iostream>
#include <fstream>

namespace adapters::file {
    // Reads the full contents of a file and returns it as a string.
    std::string read(std::string filepath);

    /** Writes `content` to the specified `filepath`. Returns true if the file was found and writted to, false otherwise. */
    bool write(std::string filepath, std::string content);

    /** Appends `content` to the specified `filepath`. Returns true if the file was found and writted to, false otherwise. */
    bool append(std::string filepath, std::string content);

    /** returns true if the file exists, false otherwise */
    bool exists(std::string filepath);
};

#endif