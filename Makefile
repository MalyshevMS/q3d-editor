BUILD_DIR = build
TARGET_NAME = q3d-editor
TARGET_BIN = $(BUILD_DIR)/bin/$(TARGET_NAME)
CMAKE_GENERATOR = Ninja
RES_DIR = res/pak001
PAK_NAME = res/pak001.q3d.tar

.PHONY: all build run clean pack

all: build

build:
	@cmake -B $(BUILD_DIR) -S . -G "$(CMAKE_GENERATOR)"
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

pack:
	@if [ -d "$(RES_DIR)" ]; then \
		echo "Упаковка $(RES_DIR) -> $(PAK_NAME).zst..."; \
		tar -cvf $(PAK_NAME) -C $(RES_DIR) . > /dev/null ; \
		zstd -q -f --rm $(PAK_NAME) -o $(PAK_NAME).zst; \
		echo "Успешно упаковано в $(PAK_NAME).zst"; \
	else \
		echo "Ошибка: Директория $(RES_DIR) не найдена!"; \
		exit 1; \
	fi
