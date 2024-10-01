#include <stdint.h>
#include "hal_data.h"
#include "digin.h"
#include "debounce.h"
#include "services/timestamp.h"
#include "relay.h"


void bsp_relay_update(bsp_relay_t relay, uint8_t level) {
    const bsp_io_port_pin_t pins[] = {
        BSP_PIN_RL1, BSP_PIN_RL2, BSP_PIN_RL3, BSP_PIN_RL4, BSP_PIN_RL5, BSP_PIN_RL6,
    };

    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, pins[relay], level ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
}

