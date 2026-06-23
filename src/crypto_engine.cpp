#include "crypto_engine.h"
#include <iostream>
#include <dlfcn.h>

#ifdef _WIN32
    #include <windows.h>
    #define LIB_EXT ".dll"
    #define LIB_PREFIX ""
#else
    #include <dlfcn.h>
    #define LIB_EXT ".so"
    #define LIB_PREFIX "lib"
#endif

static void* g_library_handle = nullptr;
static bool g_library_loaded = false;

// Указатели на функции из библиотеки
static const AlgorithmInfo* (*g_get_info_func)() = nullptr;
static size_t (*g_get_output_size_func)(size_t, int) = nullptr;
static int (*g_encrypt_func)(const uint8_t*, size_t,
                             const uint8_t*, size_t,
                             uint8_t*, size_t*) = nullptr;
static int (*g_decrypt_func)(const uint8_t*, size_t,
                             const uint8_t*, size_t,
                             uint8_t*, size_t*) = nullptr;
static int (*g_generate_key_func)(uint8_t*, size_t*) = nullptr;

// Реализация функций

void init_crypto_engine() {
    g_library_handle = nullptr;
    g_library_loaded = false;
    g_get_info_func = nullptr;
    g_get_output_size_func = nullptr;
    g_encrypt_func = nullptr;
    g_decrypt_func = nullptr;
    g_generate_key_func = nullptr;
}

void cleanup_crypto_engine() {
    unload_algorithm();
}

bool load_algorithm(const std::string& name) {
    // Выгружаем предыдущую библиотеку, если она была загружена
    unload_algorithm();
    
    std::string lib_path = std::string("./lib/") + LIB_PREFIX + name + LIB_EXT;
    
#ifdef _WIN32
    g_library_handle = LoadLibraryA(lib_path.c_str());
    if (!g_library_handle) {
        std::cerr << "Ошибка загрузки библиотеки: " << GetLastError() << std::endl;
        return false;
    }
    
    g_get_info_func = (const AlgorithmInfo* (*)())GetProcAddress(
        (HMODULE)g_library_handle, "get_algorithm_info");
    g_get_output_size_func = (size_t (*)(size_t, int))GetProcAddress(
        (HMODULE)g_library_handle, "get_output_size");
    g_encrypt_func = (int (*)(const uint8_t*, size_t, const uint8_t*, size_t, uint8_t*, size_t*))GetProcAddress(
        (HMODULE)g_library_handle, "encrypt");
    g_decrypt_func = (int (*)(const uint8_t*, size_t, const uint8_t*, size_t, uint8_t*, size_t*))GetProcAddress(
        (HMODULE)g_library_handle, "decrypt");
    g_generate_key_func = (int (*)(uint8_t*, size_t*))GetProcAddress(
        (HMODULE)g_library_handle, "generate_key");
#else
    g_library_handle = dlopen(lib_path.c_str(), RTLD_LAZY);
    if (!g_library_handle) {
        std::cerr << "Ошибка загрузки библиотеки: " << dlerror() << std::endl;
        return false;
    }
    
    g_get_info_func = (const AlgorithmInfo* (*)())dlsym(g_library_handle, "get_algorithm_info");
    g_get_output_size_func = (size_t (*)(size_t, int))dlsym(g_library_handle, "get_output_size");
    g_encrypt_func = (int (*)(const uint8_t*, size_t, const uint8_t*, size_t, uint8_t*, size_t*))dlsym(g_library_handle, "encrypt");
    g_decrypt_func = (int (*)(const uint8_t*, size_t, const uint8_t*, size_t, uint8_t*, size_t*))dlsym(g_library_handle, "decrypt");
    g_generate_key_func = (int (*)(uint8_t*, size_t*))dlsym(g_library_handle, "generate_key");
#endif
    
    // Проверяем, что все необходимые функции загружены
    if (!g_get_info_func || !g_get_output_size_func || 
        !g_encrypt_func || !g_decrypt_func) {
        std::cerr << "Ошибка: Не удалось получить все необходимые функции из библиотеки" << std::endl;
        unload_algorithm();
        return false;
    }
    
    g_library_loaded = true;
    return true;
}

void unload_algorithm() {
    if (g_library_handle) {
#ifdef _WIN32
        FreeLibrary((HMODULE)g_library_handle);
#else
        dlclose(g_library_handle);
#endif
        g_library_handle = nullptr;
    }
    
    g_library_loaded = false;
    g_get_info_func = nullptr;
    g_get_output_size_func = nullptr;
    g_encrypt_func = nullptr;
    g_decrypt_func = nullptr;
    g_generate_key_func = nullptr;
}

AlgorithmInfo get_algorithm_info() {
    if (!g_library_loaded || !g_get_info_func) {
        return {"", 0};
    }
    return *g_get_info_func();
}

size_t get_output_size(size_t input_size, int operation_type) {
    if (!g_library_loaded || !g_get_output_size_func) {
        return 0;
    }
    return g_get_output_size_func(input_size, operation_type);
}

bool encrypt_data(const std::vector<uint8_t>& key,
                  const std::vector<uint8_t>& input,
                  std::vector<uint8_t>& output) {
    if (!g_library_loaded || !g_encrypt_func) {
        return false;
    }
    
    size_t output_size = get_output_size(input.size(), 0);
    if (output_size == 0 && input.size() > 0) {
        return false;
    }
    
    output.resize(output_size);
    size_t actual_size = output_size;
    
    int result = g_encrypt_func(key.data(), key.size(),
                                input.data(), input.size(),
                                output.data(), &actual_size);
    
    if (result != 0) {
        output.clear();
        return false;
    }
    
    output.resize(actual_size);
    return true;
}

bool decrypt_data(const std::vector<uint8_t>& key,
                  const std::vector<uint8_t>& input,
                  std::vector<uint8_t>& output) {
    if (!g_library_loaded || !g_decrypt_func) {
        return false;
    }
    
    size_t output_size = get_output_size(input.size(), 1);
    if (output_size == 0 && input.size() > 0) {
        return false;
    }
    
    output.resize(output_size);
    size_t actual_size = output_size;
    
    int result = g_decrypt_func(key.data(), key.size(),
                                input.data(), input.size(),
                                output.data(), &actual_size);
    
    if (result != 0) {
        output.clear();
        return false;
    }
    
    output.resize(actual_size);
    return true;
}

bool generate_key(std::vector<uint8_t>& key) {
    if (!g_library_loaded || !g_generate_key_func) {
        return false;
    }
    
    size_t key_size = get_algorithm_info().key_size;
    if (key_size == 0) {
        // Алгоритм не требует ключа
        key.clear();
        return true;
    }
    
    key.resize(key_size);
    size_t actual_size = key_size;
    
    int result = g_generate_key_func(key.data(), &actual_size);
    
    if (result != 0) {
        key.clear();
        return false;
    }
    
    key.resize(actual_size);
    return true;
}

bool is_library_loaded() {
    return g_library_loaded;
}
