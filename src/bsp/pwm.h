#ifndef PWM_H_INCLUDED
#define PWM_H_INCLUDED


#include <stdint.h>


#define BSP_PWM_DRUM BSP_PWM_1
#define BSP_PWM_FAN  BSP_PWM_2


typedef enum {
    BSP_PWM_1,
    BSP_PWM_2,
} bsp_pwm_t;


void bsp_pwm_init(void);
void bsp_pwm_update(bsp_pwm_t pwm, uint8_t percentage);


#endif
