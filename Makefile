BUILD_DIR = build
TARGET_NAME = q3d-editor
TARGET_BIN = $(BUILD_DIR)/bin/$(TARGET_NAME)

.PHONY: all build run clean

all: build

build:
	@cmake -B $(BUILD_DIR) -S .
	@cmake --build $(BUILD_DIR)

run: build
	@if [ -f $(TARGET_BIN) ]; then \
		clear; \
		./$(TARGET_BIN); \
	else \
		echo "Ошибка: Бинарный файл $(TARGET_BIN) не найден!"; \
		exit 1; \
	fi

clean:
	rm -rf $(BUILD_DIR)
