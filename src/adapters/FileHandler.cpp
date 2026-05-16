#include "FileHandler.h"

std::string adapters::file::read(std::string filepath) {
    std::ifstream file(filepath);
    std::string out = "";
    if (!file) return "";
    while (getline(file, out)) {}
    return out;
}

bool adapters::file::write(std::string filepath, std::string content) {
    std::ofstream file(filepath);
    if (!file) return false;
    file << content;
    return true;
}

bool adapters::file::append(std::string filepath, std::string content) {
    std::ofstream file(filepath, std::ios::app);
    if (!file) return false;
    file << content;
    return true;
}

bool adapters::file::exists(std::string filepath) {
    std::ifstream file(filepath);
    if (!file) return false;
    return true;
}