CC = aarch64-linux-gnu-gcc

TARGET = main

BUILD_DIR = build
SUBDIRS = $(BUILD_DIR) $(BUILD_DIR)/tcpController

INCLUDE = -I/usr/local/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/include/cjson \
          -I/usr/local/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/include/modbus \
          -I./tcpController

SOURCES = ./tcpServerSelect.c \
          ./tcpController/tcpController.c

CFLAGS = -Wall -Wextra
CFLAGS += $(INCLUDE)

LDFLAGS = -L/usr/local/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/lib
LDFLAGS += -lcjson   # Add this line to link the libcjson library

# Update the OBJECTS variable to put .o files in the build directory
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(SUBDIRS) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Update the rule to generate object files in the build directory
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/tcpController:
	mkdir -p $(BUILD_DIR)/tcpController

clean:
	rm -rf $(BUILD_DIR)/$(TARGET) $(OBJECTS)
