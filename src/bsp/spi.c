#include <stdint.h>
#include "hal_data.h"
#include "digin.h"
#include "debounce.h"
#include "services/timestamp.h"
#include "spi.h"


uint8_t spi_master_exchange(uint8_t send) {
    size_t  i;
    uint8_t val, valToSend;
    uint8_t byte = 0;

    for (i = 0; i < 8; i++) {
        valToSend = send & 0x80;
        g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_MOSI, valToSend ? 1 : 0);
        g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_CLK, 1);
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);

        bsp_io_level_t level = BSP_IO_LEVEL_HIGH;
        g_ioport.p_api->pinRead(g_ioport.p_ctrl, BSP_PIN_MISO, &level);
        val  = level == BSP_IO_LEVEL_HIGH;
        byte = byte | val;

        if (i != 7) {
            byte = (byte << 1) & 0xFF;
            send = (send << 1) & 0xFF;
        }

        g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_CLK, 0);
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
    }
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_MOSI, 0);
    return byte;
}
