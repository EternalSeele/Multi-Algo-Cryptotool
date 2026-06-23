#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include "../src/crypto_engine.h"
#include "../src/file_io.h"

// генерация случайных тестовых данных заданного размера
static std::vector<uint8_t> generate_test_data(size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<uint8_t>(dist(gen));
    }
    return data;
}

// тестирование одного алгоритма
bool test_algorithm(const std::string& name, size_t data_size = 1024) {
    std::cout << "Тестирование " << name << "... ";
    
    // инициализация движка
    init_crypto_engine();
    
    // загрузка библиотеки
    if (!load_algorithm(name)) {
        std::cout << "ОШИБКА: не удалось загрузить библиотеку" << std::endl;
        cleanup_crypto_engine();
        return false;
    }
    
    auto info = get_algorithm_info();
    std::cout << "(" << info.algorithm_name << ", key_size=" << info.key_size << ") ";
    
    // генерация ключа
    std::vector<uint8_t> key;
    if (!generate_key(key)) {
        std::cout << "ОШИБКА: не удалось сгенерировать ключ" << std::endl;
        cleanup_crypto_engine();
        return false;
    }
    
    // генерация тестовых данных
    auto original = generate_test_data(data_size);
    
    // шифрование
    std::vector<uint8_t> encrypted;
    if (!encrypt_data(key, original, encrypted)) {
        std::cout << "ОШИБКА: не удалось зашифровать" << std::endl;
        cleanup_crypto_engine();
        return false;
    }
    
    // дешифрование
    std::vector<uint8_t> decrypted;
    if (!decrypt_data(key, encrypted, decrypted)) {
        std::cout << "ОШИБКА: не удалось расшифровать" << std::endl;
        cleanup_crypto_engine();
        return false;
    }
    
    // проверка: сравниваем исходные и восстановленные данные
    if (original.size() != decrypted.size()) {
        std::cout << "ОШИБКА: размеры не совпадают (" 
                  << original.size() << " != " << decrypted.size() << ")" << std::endl;
        cleanup_crypto_engine();
        return false;
    }
    
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i] != decrypted[i]) {
            std::cout << "ОШИБКА: данные не совпадают на позиции " << i << std::endl;
            cleanup_crypto_engine();
            return false;
        }
    }
    
    // очистка чувствительных данных
    secure_clear(key);
    secure_clear(original);
    secure_clear(encrypted);
    secure_clear(decrypted);
    
    cleanup_crypto_engine();
    std::cout << "УСПЕХ" << std::endl;
    return true;
}

int main() {
    std::cout << "=== Multi-Algo Cryptotool - Тестирование ===\n\n";
    
    // список алгоритмов и размеров данных для тестов
    const std::vector<std::string> algorithms = {"polybius", "perm", "rsa"};
    const std::vector<size_t> sizes = {16, 128, 1024};
    
    int passed = 0;
    int total = 0;
    
    for (const auto& algo : algorithms) {
        for (size_t size : sizes) {
            total++;
            if (test_algorithm(algo, size)) {
                passed++;
            }
        }
    }
    
    std::cout << "\n=== Результаты ===" << std::endl;
    std::cout << "Пройдено: " << passed << " из " << total << std::endl;
    
    return (passed == total) ? 0 : 1;
}
