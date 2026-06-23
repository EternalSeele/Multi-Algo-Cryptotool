#include "polybius.h"

static const char* NAME = "polybius";
static const size_t KEY_SIZE = 0;

const AlgorithmInfo* get_algorithm_info() {
    static AlgorithmInfo info = {NAME, KEY_SIZE};
    return &info;
}

size_t get_output_size(size_t input_size, int operation_type) {
    if (operation_type == 0) {
        return input_size * 2;
    } else {
        return input_size / 2;
    }
}

int encrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size) {
    // void для подавления предупреждений о неиспользуемых параметрах
    (void)key;
    (void)key_size;
    
    *output_size = input_size * 2;
    
    for (size_t i = 0; i < input_size; ++i) {
        output[i * 2] = input[i] >> 4;
        output[i * 2 + 1] = input[i] & 0x0F;
    }
    
    return 0;
}

int decrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size) {
    (void)key;
    (void)key_size;
    
    if (input_size % 2 != 0) {
        return 1;
    }
    
    *output_size = input_size / 2;
    
    for (size_t i = 0; i < *output_size; ++i) {
        output[i] = (input[i * 2] << 4) | input[i * 2 + 1];
    }
    
    return 0;
}

int generate_key(uint8_t* key, size_t* key_size) {
    (void)key;
    
    *key_size = KEY_SIZE;
    return 0;
}
