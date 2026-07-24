BUILD_DIR := build

.PHONY: all clean install

all: $(BUILD_DIR)/vsks

$(BUILD_DIR)/vsks: $(BUILD_DIR)/Makefile
	cmake --build $(BUILD_DIR) --parallel

$(BUILD_DIR)/Makefile:
	cmake \
		-B $(BUILD_DIR) \
		-DCMAKE_CXX_COMPILER=g++ \
		-DCMAKE_BUILD_TYPE=Release

clean:
	rm -rf $(BUILD_DIR)

install: $(BUILD_DIR)/vsks
	cmake --install $(BUILD_DIR) --prefix /usr/local