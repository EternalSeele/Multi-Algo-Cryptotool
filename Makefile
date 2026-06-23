# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -O2 -fPIC
LDFLAGS = -ldl

# Пути
SRC_DIR = src
MODULES_DIR = modules
TESTS_DIR = tests
BUILD_DIR = build
LIB_DIR = lib

# Целевые файлы
TARGET = multi_algo_cryptotool
TEST_TARGET = test_crypto

# Модули (короткие имена, совпадают с именами в коде)
MODULES = polybius perm rsa

# Правила сборки

.PHONY: all clean modules tests run help

all: modules $(TARGET)

help:
	@echo "Доступные команды:"
	@echo "  make all       - Собрать все (модули + приложение)"
	@echo "  make modules   - Собрать динамические библиотеки"
	@echo "  make tests     - Собрать тесты"
	@echo "  make run       - Запустить приложение"
	@echo "  make clean     - Очистить сборку"


# Создание необходимых директорий

$(BUILD_DIR) $(LIB_DIR):
	mkdir -p $@


# Сборка динамических библиотек (модулей)

modules: $(BUILD_DIR) $(LIB_DIR)
	@echo "=== Сборка модулей ==="
	@for module in $(MODULES); do \
		echo "  Сборка $$module..."; \
		$(CXX) $(CXXFLAGS) -c $(MODULES_DIR)/$$module/$$module.cpp -o $(BUILD_DIR)/$$module.o; \
		$(CXX) -shared -o $(LIB_DIR)/lib$$module.so $(BUILD_DIR)/$$module.o; \
		echo "    -> $(LIB_DIR)/lib$$module.so"; \
	done


# Сборка основного приложения

$(TARGET): $(BUILD_DIR) modules
	@echo "=== Сборка приложения ==="
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/arguments.cpp -o $(BUILD_DIR)/arguments.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/crypto_engine.cpp -o $(BUILD_DIR)/crypto_engine.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/file_io.cpp -o $(BUILD_DIR)/file_io.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o
	$(CXX) $(BUILD_DIR)/arguments.o $(BUILD_DIR)/crypto_engine.o $(BUILD_DIR)/file_io.o $(BUILD_DIR)/main.o -o $(TARGET) $(LDFLAGS)
	@echo "  -> $(TARGET)"


# Сборка тестов

tests: $(BUILD_DIR) modules
	@echo "=== Сборка тестов ==="
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/crypto_engine.cpp -o $(BUILD_DIR)/test_crypto_engine.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/file_io.cpp -o $(BUILD_DIR)/test_file_io.o
	$(CXX) $(CXXFLAGS) -c $(TESTS_DIR)/test_crypto.cpp -o $(BUILD_DIR)/test_crypto.o
	$(CXX) $(BUILD_DIR)/test_crypto_engine.o $(BUILD_DIR)/test_file_io.o $(BUILD_DIR)/test_crypto.o -o $(TEST_TARGET) $(LDFLAGS)
	@echo "  -> $(TEST_TARGET)"


# Запуск

run: $(TARGET)
	./$(TARGET) --help


# Очистка

clean:
	@echo "=== Очистка ==="
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)
	@echo "Готово"
