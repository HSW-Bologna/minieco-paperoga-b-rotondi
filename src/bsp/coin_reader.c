#include <stdint.h>
#include "hal_data.h"
#include "digin.h"
#include "pulsecounter.h"
#include "services/timestamp.h"


#define DEBOUNCE_TIMES 5


static pulse_filter_t filter = {0};


void bsp_coin_reader_init(void) {
    pulse_filter_init(&filter, COUNT_HIGH_PULSE, 0);
}


void bsp_coin_reader_manage(void) {
    static timestamp_t ts = 0;

    if (timestamp_is_expired(ts, 2)) {
        unsigned int bitmap = 0;

        const bsp_io_port_pin_t pins[] = {
            BSP_PIN_IN1, BSP_PIN_IN2, BSP_PIN_IN3, BSP_PIN_IN4, BSP_PIN_IN5, BSP_PIN_IN6, BSP_PIN_IN7,
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
