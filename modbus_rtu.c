#include <stdio.h>
#include <modbus.h>
#include <errno.h>

int main() {
    modbus_t *ctx;
    uint16_t tab_reg[1];
    int rc;

    ctx = modbus_new_rtu("/dev/ttyS9", 4800, 'N', 8, 1);
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create the Modbus context\n");
        return -1;
    }

    if (modbus_set_slave(ctx, 0x01) == -1) {
        fprintf(stderr, "Failed to set Modbus slave address\n");
        modbus_free(ctx);
        return -1;
    }

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Modbus connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    rc = modbus_read_registers(ctx, 0x00, 1, tab_reg);
    if (rc == -1) {
        fprintf(stderr, "Modbus read failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    printf("噪声值: %.1f dB\n", (float)tab_reg[0] / 10.0);

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}