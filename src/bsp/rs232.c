#include <services/timestamp.h>
#include <stdint.h>
#include <stdlib.h>
#include "hal_data.h"
#include "system.h"
#include "rs232.h"


static size_t      receive_index                            = 0;
static uint8_t     receive_buffer[BSP_RS232_MAX_PACKET_LEN] = {0};
static timestamp_t last_reception_timestamp                 = 0;

void bsp_rs232_init(void) {
    g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);
}

void bsp_rs232_write(uint8_t *data, uint16_t len) {
    g_uart0.p_api->write(g_uart0.p_ctrl, data, len);
}

size_t bsp_rs232_read(uint8_t *data, uint16_t max_len) {
    size_t total_bytes_read = receive_index > max_len ? max_len : receive_index;
    memcpy(data, receive_buffer, total_bytes_read);
    return total_bytes_read;
}

void bsp_rs232_flush(void) {
    receive_index = 0;
}

void rs232_callback(uart_callback_args_t *args) {
    switch (args->event) {
        case UART_EVENT_RX_COMPLETE:
            __NOP();
            __NOP();
            __NOP();
            break;

        case UART_EVENT_RX_CHAR: {
            if (receive_index < BSP_RS232_MAX_PACKET_LEN) {
                last_reception_timestamp      = bsp_system_get_millis();
                receive_buffer[receive_index] = (uint8_t)args->data;
                receive_index++;
            }
            break;
        }

        case UART_EVENT_TX_DATA_EMPTY:
            break;

        default:
            break;
    }
}

uint8_t bsp_rs232_timed_out(timestamp_t period_ms) {
    uint8_t result = timestamp_is_expired(last_reception_timestamp, period_ms);
    return result;
}
