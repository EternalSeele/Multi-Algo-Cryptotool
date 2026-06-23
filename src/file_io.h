#ifndef FILE_IO_H
#define FILE_IO_H

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> read_data(const std::string& source, bool is_file);
bool write_data(const std::string& dest, const std::vector<uint8_t>& data, bool is_file);
void secure_clear(std::vector<uint8_t>& data);

#endif
