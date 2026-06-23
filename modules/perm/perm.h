#ifndef PERM_H
#define PERM_H

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

struct AlgorithmInfo {
    const char* algorithm_name;
    size_t key_size;
};

const AlgorithmInfo* get_algorithm_info();
size_t get_output_size(size_t input_size, int operation_type);
int encrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size);
int decrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size);
int generate_key(uint8_t* key, size_t* key_size);

#ifdef __cplusplus
}
#endif

#endif
