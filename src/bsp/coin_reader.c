#include <stdint.h>
#include "hal_data.h"
#include "digin.h"
#include "pulsecounter.h"
#include "services/timestamp.h"
#include "coin_reader.h"


#define DEBOUNCE_TIMES 5


static pulse_filter_t filter = {0};


void bsp_coin_reader_init(void) {
    pulse_filter_init(&filter, COUNT_HIGH_PULSE, 0);
    bsp_coin_reader_enable(0);
}


void bsp_coin_reader_enable(uint8_t enable) {
    pulse_filter_init(&filter, COUNT_HIGH_PULSE, 0);
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_INIBIT, enable ? BSP_IO_LEVEL_LOW : BSP_IO_LEVEL_HIGH);
}


void bsp_coin_reader_manage(void) {
    static timestamp_t ts = 0;

    if (timestamp_is_expired(ts, 2)) {
        unsigned int bitmap = 0;

        const bsp_io_port_pin_t pins[] = {
            BSP_PIN_IN6, BSP_PIN_P_G1, BSP_PIN_P_G2, BSP_PIN_P_G3, BSP_PIN_P_G4, BSP_PIN_P_G5,
        };

        for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
            bsp_io_level_t level = BSP_IO_LEVEL_LOW;
            g_ioport.p_api->pinRead(g_ioport.p_ctrl, pins[i], &level);
            if (level == BSP_IO_LEVEL_HIGH) {
                bitmap |= 1 << i;
            }
        }

        pulse_filter(&filter, bitmap, DEBOUNCE_TIMES);

        ts = timestamp_get();
    }
}


uint16_t bsp_coin_reader_read(bsp_coin_reader_line_t line) {
    return (uint16_t)pulse_count(&filter, (int)line);
}


void bsp_coin_reader_clear(void) {
    for (bsp_coin_reader_line_t line = BSP_COIN_READER_LINE_1; line <= BSP_COIN_READER_LINE_5; line++) {
        pulse_clear(&filter, (int)line);
    }
}
