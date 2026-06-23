#include "file_io.h"
#include <fstream>
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

std::vector<uint8_t> read_data(const std::string& source, bool is_file) {
    if (is_file) {
        std::ifstream file(source, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл " << source << std::endl;
            return {};
        }
        
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return data;
    } else {
        std::vector<uint8_t> data;
        uint8_t buffer[4096];
        while (std::cin.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
            data.insert(data.end(), buffer, buffer + std::cin.gcount());
        }
        data.insert(data.end(), buffer, buffer + std::cin.gcount());
        return data;
    }
}

bool write_data(const std::string& dest, const std::vector<uint8_t>& data, bool is_file) {
    if (is_file) {
        std::ofstream file(dest, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Ошибка: Не удалось создать файл " << dest << std::endl;
            return false;
        }
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    } else {
        std::cout.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }
}

void secure_clear(std::vector<uint8_t>& data) {
    if (data.empty()) return;
    
#ifdef _WIN32
    SecureZeroMemory(data.data(), data.size());
#else
    explicit_bzero(data.data(), data.size());
#endif
    
    data.clear();
}
