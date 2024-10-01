#include <stdint.h>
#include "hal_data.h"
#include "pwm.h"


static uint16_t period_counter     = 0;
static uint16_t duty_cycle_1_start = 0;
static uint16_t duty_cycle_2_start = 0;


void bsp_pwm_init(void) {
    R_AGT_Open(&g_timer0_ctrl, &g_timer0_cfg);
    R_AGT_Start(&g_timer0_ctrl);
}


void bsp_pwm_update(bsp_pwm_t pwm, uint8_t percentage) {
    if (percentage > 100) {
        percentage = 100;
    }

    switch (pwm) {
        case BSP_PWM_1:
            duty_cycle_1_start = 100 - percentage;
            break;

        case BSP_PWM_2:
            duty_cycle_2_start = 100 - percentage;
            break;
    }
}


void bsp_pwm_interrupt_callback(timer_callback_args_t *p_args) {
    (void)p_args;

    period_counter = (period_counter + 1) % 100;

    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_PWM1,
                             duty_cycle_1_start >= period_counter ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl, BSP_PIN_PWM2,
                             duty_cycle_2_start >= period_counter ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
}
