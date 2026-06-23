#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include <vector>
#include <optional>

enum class OperationMode {
    HELP,
    GENERATE_KEY,
    ENCRYPT,
    DECRYPT
};

struct ProgramArguments {
    bool show_help = false;
    std::string algorithm;
    OperationMode mode = OperationMode::HELP;
    std::optional<std::string> key_file;
    std::optional<std::string> input_file;
    std::optional<std::string> output_file;
    std::optional<std::string> save_key_file;
    bool generate_key = false;
    bool write_key_to_stdout = false;
};

ProgramArguments parse_arguments(int argc, char* argv[]);
void print_help();

#endif
