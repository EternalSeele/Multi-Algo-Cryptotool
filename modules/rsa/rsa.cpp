#include "rsa.h"
#include <cstring>
#include <random>
#include <iostream>

static const char* NAME = "rsa";
static const size_t KEY_SIZE = 16;

struct RSAKey {
    uint64_t n;
    uint64_t d;
};

const AlgorithmInfo* get_algorithm_info() {
    static AlgorithmInfo info = {NAME, KEY_SIZE};
    return &info;
}

size_t get_output_size(size_t input_size, int operation_type) {
    (void)operation_type;
    return input_size * 8;
}

static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    if (mod == 1) return 0;
    uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (static_cast<unsigned __int128>(result) * base) % mod;
        }
        exp >>= 1;
        base = (static_cast<unsigned __int128>(base) * base) % mod;
    }
    return result;
}

int encrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size) {
    if (key_size < sizeof(RSAKey)) return 1;
    
    const RSAKey* rsa_key = reinterpret_cast<const RSAKey*>(key);
    *output_size = input_size * 8;
    
    if (rsa_key->n <= 255) {
        std::cerr << "RSA Error: n <= 255" << std::endl;
        return 1;
    }
    
    const uint64_t e = 17;
    
    for (size_t i = 0; i < input_size; ++i) {
        uint64_t encrypted = mod_pow(input[i], e, rsa_key->n);
        memcpy(output + i * 8, &encrypted, 8);
    }
    
    return 0;
}

int decrypt(const uint8_t* key, size_t key_size,
            const uint8_t* input, size_t input_size,
            uint8_t* output, size_t* output_size) {
    if (key_size < sizeof(RSAKey)) return 1;
    if (input_size % 8 != 0) return 1;
    
    const RSAKey* rsa_key = reinterpret_cast<const RSAKey*>(key);
    *output_size = input_size / 8;
    
    for (size_t i = 0; i < *output_size; ++i) {
        uint64_t cipher;
        memcpy(&cipher, input + i * 8, 8);
        output[i] = static_cast<uint8_t>(mod_pow(cipher, rsa_key->d, rsa_key->n));
    }
    
    return 0;
}

static bool is_prime(uint64_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (uint64_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static uint64_t mod_inverse(uint64_t a, uint64_t m) {
    int64_t m0 = m;
    int64_t y = 0, x = 1;
    if (m == 1) return 0;
    while (a > 1) {
        int64_t q = a / m;
        int64_t t = m;
        m = a % m;
        a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}

int generate_key(uint8_t* key, size_t* key_size) {
    *key_size = KEY_SIZE;
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(50, 300);
    
    RSAKey* rsa_key = reinterpret_cast<RSAKey*>(key);
    
    for (int attempt = 0; attempt < 200; ++attempt) {
        uint64_t p, q;
        
        do {
            p = dist(gen);
            if (p % 2 == 0) p++;
        } while (!is_prime(p));
        
        do {
            q = dist(gen);
            if (q % 2 == 0) q++;
        } while (!is_prime(q) || q == p);
        
        uint64_t n = p * q;
        uint64_t phi = (p - 1) * (q - 1);
        
        uint64_t e = 17;
        while (gcd(e, phi) != 1) {
            e += 2;
        }
        
        uint64_t d = mod_inverse(e, phi);
        
        // Проверяем корректность ключа
        if (n > 255 && d > 0 && d < n) {
            // Тест на случайном байте (проверяем несколько значений)
            bool valid = true;
            for (uint8_t test_val = 0; test_val < 10; ++test_val) {
                uint64_t enc = mod_pow(test_val, 17, n);
                uint64_t dec = mod_pow(enc, d, n);
                if (test_val != dec) {
                    valid = false;
                    break;
                }
            }
            
            if (valid) {
                rsa_key->n = n;
                rsa_key->d = d;
                return 0;
            }
        }
    }
    
    // Запасной вариант (фиксированные числа)
    const uint64_t p = 61;
    const uint64_t q = 53;
    uint64_t n = p * q;
    uint64_t phi = (p - 1) * (q - 1);
    
    uint64_t e = 17;
    while (gcd(e, phi) != 1) {
        e += 2;
    }
    
    uint64_t d = mod_inverse(e, phi);
    
    rsa_key->n = n;
    rsa_key->d = d;
    
    return 0;
}
