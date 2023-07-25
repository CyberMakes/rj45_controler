CC = gcc
TARGET = tcpServerSelect
BUILD_DIR = build

INCLUDE = -I/usr/local/include/cjson -I/usr/local/include/modbus -I./tcpController

SOURCES = ./*.c

CFLAGS = -Wall -Wextra
CFLAGS += $(INCLUDE)

LDFLAGS = -L/usr/local/lib

LIBS = -lmodbus -lcjson

OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o)))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
