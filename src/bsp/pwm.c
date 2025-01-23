#include <stdint.h>
#include "hal_data.h"
#include "pwm.h"


static uint32_t pwm_periph_period;


void bsp_pwm_init(void) {
    fsp_err_t    err = FSP_SUCCESS;
    timer_info_t info;

    /* Initializes the module. */
    err = R_GPT_Open(&g_timer_pwm_ctrl, &g_timer_pwm_cfg);
    assert(FSP_SUCCESS == err);

    /* Start the timer. */
    R_GPT_Start(&g_timer_pwm_ctrl);

    /* Get the current period setting. */
    R_GPT_InfoGet(&g_timer_pwm_ctrl, &info);
    pwm_periph_period = info.period_counts;
}


void bsp_pwm_update(bsp_pwm_t pwm, uint8_t percentage) {
    if (percentage > 100) {
        percentage = 100;
    }

    fsp_err_t    err = FSP_SUCCESS;
    uint32_t     width_cycles;
    gpt_io_pin_t io_name = pwm == BSP_PWM_FAN ? GPT_IO_PIN_GTIOCB : GPT_IO_PIN_GTIOCA;
    // gpt_io_pin_t io_name = GPT_IO_PIN_GTIOCA_AND_GTIOCB;

    width_cycles = (uint32_t)(((uint64_t)pwm_periph_period * (100 - percentage)) / 100);

    /* Set the calculated duty cycle. */
    err = R_GPT_DutyCycleSet(&g_timer_pwm_ctrl, width_cycles, io_name);
    assert(FSP_SUCCESS == err);
}
