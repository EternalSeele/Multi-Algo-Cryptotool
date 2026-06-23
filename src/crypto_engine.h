#ifndef CRYPTO_ENGINE_H
#define CRYPTO_ENGINE_H

#include <string>
#include <vector>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Информация об алгоритме, имя и размер ключа
struct AlgorithmInfo {
    const char* algorithm_name;
    size_t key_size;
};

#ifdef __cplusplus
}
#endif

// Инициализация engine
void init_crypto_engine();

// Освобождение ресурсов engine
void cleanup_crypto_engine();

// Загрузка библиотеки с алгоритмом
bool load_algorithm(const std::string& name);

// Выгрузка текущей библиотеки
void unload_algorithm();

// Получение информации о загруженном алгоритме
AlgorithmInfo get_algorithm_info();

// Вычисление размера выходного буфера
size_t get_output_size(size_t input_size, int operation_type);

// Шифрование данных
bool encrypt_data(const std::vector<uint8_t>& key,
                  const std::vector<uint8_t>& input,
                  std::vector<uint8_t>& output);

// Дешифрование данных
bool decrypt_data(const std::vector<uint8_t>& key,
                  const std::vector<uint8_t>& input,
                  std::vector<uint8_t>& output);

// Генерация ключа для загруженного алгоритма
bool generate_key(std::vector<uint8_t>& key);

// Проверка, загружена ли библиотека
bool is_library_loaded();

#endif
