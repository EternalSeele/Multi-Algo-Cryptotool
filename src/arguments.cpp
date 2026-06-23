#include "arguments.h"
#include <iostream>
#include <cstring>

static bool is_short_option(const char* arg, char c) {
    return arg[0] == '-' && arg[1] == c && arg[2] == '\0';
}

static bool is_long_option(const char* arg, const char* name) {
    return strcmp(arg, name) == 0;
}

static std::string get_value(int& i, int argc, char* argv[]) {
    if (i + 1 < argc) {
        return argv[++i];
    }
    return "";
}

ProgramArguments parse_arguments(int argc, char* argv[]) {
    ProgramArguments args;
    
    if (argc == 1) {
        args.show_help = true;
        return args;
    }
    
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        
        if (is_short_option(arg, 'h') || is_long_option(arg, "--help")) {
            args.show_help = true;
            return args;
        }
        else if (is_short_option(arg, 'a') || is_long_option(arg, "--algorithm")) {
            args.algorithm = get_value(i, argc, argv);
        }
        else if (is_short_option(arg, 'm') || is_long_option(arg, "--mode")) {
            std::string mode = get_value(i, argc, argv);
            if (mode == "generate-key") {
                args.mode = OperationMode::GENERATE_KEY;
            } else if (mode == "encrypt") {
                args.mode = OperationMode::ENCRYPT;
            } else if (mode == "decrypt") {
                args.mode = OperationMode::DECRYPT;
            }
        }
        else if (is_short_option(arg, 'k') || is_long_option(arg, "--key")) {
            args.key_file = get_value(i, argc, argv);
        }
        else if (is_short_option(arg, 'i') || is_long_option(arg, "--input")) {
            args.input_file = get_value(i, argc, argv);
        }
        else if (is_short_option(arg, 'o') || is_long_option(arg, "--output")) {
            args.output_file = get_value(i, argc, argv);
        }
        else if (is_short_option(arg, 's') || is_long_option(arg, "--save-key")) {
            args.save_key_file = get_value(i, argc, argv);
        }
        else if (is_short_option(arg, 'g') || is_long_option(arg, "--generate-key")) {
            args.generate_key = true;
        }
        else if (is_long_option(arg, "--write-key")) {
            args.write_key_to_stdout = true;
        }
    }
    
    return args;
}

void print_help() {
    std::cout << "Multi-Algo Cryptotool - Криптографическая утилита\n\n";
    std::cout << "Использование:\n";
    std::cout << "  multi_algo_cryptotool --help\n";
    std::cout << "  multi_algo_cryptotool -a <алгоритм> -m <режим> [опции]\n\n";
    std::cout << "Алгоритмы:\n";
    std::cout << "  polybius    - Квадрат Полибия (16x16)\n";
    std::cout << "  perm        - Блочная перестановка\n";
    std::cout << "  rsa         - Алгоритм RSA\n\n";
    std::cout << "Режимы:\n";
    std::cout << "  encrypt         - Шифрование\n";
    std::cout << "  decrypt         - Дешифрование\n";
    std::cout << "  generate-key    - Генерация ключа\n\n";
    std::cout << "Опции:\n";
    std::cout << "  -a, --algorithm  <алг>   Алгоритм шифрования\n";
    std::cout << "  -m, --mode       <режим> Режим работы\n";
    std::cout << "  -k, --key        <файл>  Файл с ключом\n";
    std::cout << "  -i, --input      <файл>  Входной файл\n";
    std::cout << "  -o, --output     <файл>  Выходной файл\n";
    std::cout << "  -g, --generate-key       Генерировать ключ\n";
    std::cout << "  -s, --save-key   <файл>  Сохранить ключ\n";
    std::cout << "  --write-key              Вывести ключ в stdout\n";
    std::cout << "  -h, --help               Показать справку\n";
}
