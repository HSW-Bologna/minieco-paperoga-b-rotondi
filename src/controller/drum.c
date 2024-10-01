#include "bsp/relay.h"
#include "bsp/pwm.h"
#include "services/timestamp.h"
#include "model/model.h"


void drum_run_forward(uint8_t speed) {
    bsp_relay_update(BSP_RELAY_BACKWARD, 0);
    bsp_relay_update(BSP_RELAY_FORWARD, 1);
    bsp_pwm_update(BSP_PWM_DRUM, speed);
}


void drum_run_backward(uint8_t speed) {
    bsp_relay_update(BSP_RELAY_FORWARD, 0);
    bsp_relay_update(BSP_RELAY_BACKWARD, 1);
    bsp_pwm_update(BSP_PWM_DRUM, speed);
}


void drum_stop(void) {
    bsp_relay_update(BSP_RELAY_BACKWARD, 0);
    bsp_relay_update(BSP_RELAY_FORWARD, 0);
}
