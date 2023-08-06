CC = aarch64-linux-gnu-gcc

TARGET = main

BUILD_DIR = build
SUBDIRS = $(BUILD_DIR) $(BUILD_DIR)/tcp $(BUILD_DIR)/rs485

INCLUDE = -I/usr/local/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/include/cjson \
          -I/usr/local/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/include/modbus \
          -I./tcp \
          -I./rs485 \
		  -I./

SOURCES = ./main.c\
          ./tcp/tcpController.c\
          ./tcp/tcpServer.c\
          ./rs485/rs485Sensors.c

CFLAGS = -Wall -Wextra
CFLAGS += $(INCLUDE)
ifdef DEBUG
CFLAGS += -DDEBUG
endif

LDFLAGS = -L/usr/local/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/lib
LDFLAGS += -lcjson
LDFLAGS += -lmodbus
LDFLAGS += -pthread

# Generate the list of object files with their paths in the build directory
OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))

all: $(BUILD_DIR) $(SUBDIRS) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Update the rule to generate object files in the build directory
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/tcp:
	mkdir -p $(BUILD_DIR)/tcp

$(BUILD_DIR)/rs485:
	mkdir -p $(BUILD_DIR)/rs485

clean:
	rm -rf $(BUILD_DIR)