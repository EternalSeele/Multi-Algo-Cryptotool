#include "perm.h"
#include <cstring>
#include <random>
#include <vector>
#include <algorithm>

static const char* NAME = "perm";
static const size_t KEY_SIZE = 16;

const AlgorithmInfo* get_algorithm_info() {
    static AlgorithmInfo info = {NAME, KEY_SIZE};
    return &info;
}

size_t get_output_size(size_t input_size, int operation_type) {
    (void)operation_type;
    return input_size;
}

static void apply_permutation(const uint8_t* input, size_t input_size,
                              const uint8_t* key, size_t key_size,
                              uint8_t* output, bool inverse) {
    size_t block_size = 4;
    size_t num_blocks = input_size / block_size;
    size_t remainder = input_size % block_size;
    
    // Извлекаем индексы из ключа
    std::vector<int> perm(block_size);
    const uint32_t* key_data = reinterpret_cast<const uint32_t*>(key);
    
    for (size_t i = 0; i < block_size && i < key_size / sizeof(uint32_t); ++i) {
        perm[i] = key_data[i] % block_size;
    }
    
    // Перестановка корректна (все индексы уникальны)
    std::vector<bool> used(block_size, false);
    bool valid = true;
    for (size_t i = 0; i < block_size; ++i) {
        if (perm[i] < 0 || perm[i] >= (int)block_size || used[perm[i]]) {
            valid = false;
            break;
        }
        used[perm[i]] = true;
    }
    
    // Если перестановка невалидна, используем стандартную
    if (!valid) {
        for (size_t i = 0; i < block_size; ++i) {
            perm[i] = i;
        }
    }
    
    // Oбратн. перестановкa для дешифрования
    std::vector<int> inv_perm(block_size);
    for (size_t i = 0; i < block_size; ++i) {
        inv_perm[perm[i]] = i;
    }
    
    const std::vector<int>& current_perm = inverse ? inv_perm : perm;
    
    // Перестановкa к каждому блоку
    for (size_t b = 0; b < num_blocks; ++b) {
        const uint8_t* src = input + b * block_size;
        uint8_t* dst = output + b * block_size;
        
        for (size_t i = 0; i < block_size; ++i) {
            dst[i] = src[current_perm[i]];
        }
    }
    
    // Копируем остаток
    if (remainder > 0) {
        memcpy(output + num_blocks * block_size,
               input + num_blocks * block_size,
               remainder);
    }
}

int encrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size) {
    *output_size = input_size;
    apply_permutation(input, input_size, key, key_size, output, false);
    return 0;
}

int decrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size) {
    *output_size = input_size;
    apply_permutation(input, input_size, key, key_size, output, true);
    return 0;
}

int generate_key(uint8_t* key, size_t* key_size) {
    *key_size = KEY_SIZE;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Cлуч. перестановкa
    std::vector<int> perm = {0, 1, 2, 3};
    std::shuffle(perm.begin(), perm.end(), gen);
    
    uint32_t* key_data = reinterpret_cast<uint32_t*>(key);
    for (size_t i = 0; i < 4; ++i) {
        key_data[i] = perm[i];
    }
    
    return 0;
}
