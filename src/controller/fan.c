#include "bsp/relay.h"
#include "fan.h"
#include "bsp/pwm.h"


void ventilazione_on_full(void) {
    bsp_pwm_update(BSP_PWM_FAN, 30);
    bsp_relay_update(BSP_RELAY_FAN, 1);
}


void ventilazione_off(void) {
    bsp_pwm_update(BSP_PWM_FAN, 0);
    bsp_relay_update(BSP_RELAY_FAN, 0);
}
