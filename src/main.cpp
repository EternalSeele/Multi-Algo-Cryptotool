#include "arguments.h"
#include "crypto_engine.h"
#include "file_io.h"
#include <iostream>
#include <vector>

static const std::vector<std::string> SUPPORTED_ALGORITHMS = {
    "polybius", "perm", "rsa"
};

bool is_supported_algorithm(const std::string& name) {
    for (const auto& algo : SUPPORTED_ALGORITHMS) {
        if (algo == name) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    auto args = parse_arguments(argc, argv);
    
    if (args.show_help) {
        print_help();
        return 0;
    }
    
    if (args.algorithm.empty() || !is_supported_algorithm(args.algorithm)) {
        std::cerr << "Ошибка: Неподдерживаемый алгоритм или не указан алгоритм.\n";
        std::cerr << "Используйте --help для получения справки.\n";
        return 1;
    }
    
    // Инициализация движка
    init_crypto_engine();
    
    if (!load_algorithm(args.algorithm)) {
        std::cerr << "Ошибка: Не удалось загрузить библиотеку алгоритма " 
                  << args.algorithm << std::endl;
        cleanup_crypto_engine();
        return 1;
    }
    
    auto info = get_algorithm_info();
    
    // Режим генерации ключа
    if (args.mode == OperationMode::GENERATE_KEY || args.generate_key) {
        if (info.key_size == 0) {
            std::cerr << "Ошибка: Алгоритм " << args.algorithm 
                      << " не поддерживает генерацию ключа (key_size=0)\n";
            cleanup_crypto_engine();
            return 1;
        }
        
        std::vector<uint8_t> key;
        if (!generate_key(key)) {
            std::cerr << "Ошибка генерации ключа\n";
            cleanup_crypto_engine();
            return 1;
        }
        
        if (args.save_key_file.has_value()) {
            write_data(args.save_key_file.value(), key, true);
            std::cout << "Ключ сохранен в " << args.save_key_file.value() << std::endl;
        } else if (args.write_key_to_stdout) {
            write_data("", key, false);
        } else {
            std::cout << "Сгенерирован ключ (" << key.size() << " байт): ";
            for (uint8_t b : key) {
                printf("%02x", b);
            }
            std::cout << std::endl;
        }
        
        secure_clear(key);
        cleanup_crypto_engine();
        return 0;
    }
    
    // Проверка режима
    if (args.mode != OperationMode::ENCRYPT && args.mode != OperationMode::DECRYPT) {
        std::cerr << "Ошибка: Не указан режим работы. Используйте --mode encrypt/decrypt\n";
        cleanup_crypto_engine();
        return 1;
    }
    
    // Управление ключом в зависимости от алгоритма
    std::vector<uint8_t> key;
    
    if (info.key_size > 0) {
        // Алгоритм требует ключ (perm, rsa)
        if (args.key_file.has_value()) {
            key = read_data(args.key_file.value(), true);
            if (key.empty()) {
                std::cerr << "Ошибка: Не удалось прочитать ключ из файла " 
                          << args.key_file.value() << std::endl;
                cleanup_crypto_engine();
                return 1;
            }
        } else {
            std::cerr << "Ошибка: Алгоритм " << args.algorithm 
                      << " требует ключ (" << info.key_size << " байт).\n";
            std::cerr << "Используйте -k или --key для указания файла ключа.\n";
            cleanup_crypto_engine();
            return 1;
        }
        
        if (key.size() != info.key_size) {
            std::cerr << "Ошибка: Размер ключа (" << key.size() 
                      << " байт) не соответствует требуемому (" << info.key_size << " байт)\n";
            secure_clear(key);
            cleanup_crypto_engine();
            return 1;
        }
    } else {
        // Алгоритм не требует ключ (polybius)
        if (args.key_file.has_value()) {
            std::cout << "Предупреждение: Алгоритм " << args.algorithm 
                      << " не использует ключ. Параметр -k игнорируется.\n";
        }
        // key остается пустым
    }
    
    // Чтение входных данных
    bool input_from_stdin = !args.input_file.has_value();
    auto input = read_data(args.input_file.value_or(""), !input_from_stdin);
    if (input.empty()) {
        std::cerr << "Ошибка: Входные данные пусты или не удалось прочитать\n";
        secure_clear(key);
        cleanup_crypto_engine();
        return 1;
    }
    
    // Выполнение операции
    std::vector<uint8_t> output;
    bool success = false;
    
    if (args.mode == OperationMode::ENCRYPT) {
        success = encrypt_data(key, input, output);
    } else {
        success = decrypt_data(key, input, output);
    }
    
    secure_clear(key);
    secure_clear(input);
    
    if (!success) {
        std::cerr << "Ошибка выполнения криптографической операции\n";
        cleanup_crypto_engine();
        return 1;
    }
    
    // Запись результата
    bool output_to_stdout = !args.output_file.has_value();
    if (!write_data(args.output_file.value_or(""), output, !output_to_stdout)) {
        std::cerr << "Ошибка записи результата\n";
        cleanup_crypto_engine();
        return 1;
    }
    
    secure_clear(output);
    cleanup_crypto_engine();
    
    if (!output_to_stdout) {
        std::cout << "Операция успешно завершена. Результат в " 
                  << args.output_file.value() << std::endl;
    }
    
    return 0;
}
