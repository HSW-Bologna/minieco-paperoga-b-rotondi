#include <stdint.h>
#include "hal_data.h"
#include "digin.h"
#include "debounce.h"
#include "services/timestamp.h"


#define DEBOUNCE_TIMES 5


static debounce_filter_t filter = {0};


void bsp_digin_init(void) {
    debounce_filter_init(&filter);
}


void bsp_digin_manage(void) {
    static timestamp_t ts = 0;

    if (timestamp_is_expired(ts, 5)) {
        unsigned int bitmap = 0;

        const bsp_io_port_pin_t pins[] = {
            BSP_PIN_IN1, BSP_PIN_IN2, BSP_PIN_IN3, BSP_PIN_IN4, BSP_PIN_IN5, BSP_PIN_IN6, BSP_PIN_IN7,
        };

        for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
            bsp_io_level_t level = BSP_IO_LEVEL_HIGH;
            g_ioport.p_api->pinRead(g_ioport.p_ctrl, pins[i], &level);
            if (level == BSP_IO_LEVEL_LOW) {
                bitmap |= 1 << i;
            }
        }

        debounce_filter(&filter, bitmap, DEBOUNCE_TIMES);

        ts = timestamp_get();
    }
}


uint8_t bsp_digin_get(bsp_digin_t digin) {
    return (uint8_t)debounce_read(&filter, digin);
}


uint16_t bsp_digin_get_bitmap(void) {
    return (uint8_t)debounce_value(&filter);
}
