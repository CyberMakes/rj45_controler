CC=gcc
CFLAGS=-I/usr/include/modbus
LIBS=-lmodbus

sensors: sensors.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
