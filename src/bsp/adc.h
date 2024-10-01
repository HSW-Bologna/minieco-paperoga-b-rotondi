/*
 * adc.h
 *
 *  Created on: 11 Jul 2024
 *      Author: Maldus
 */

#ifndef BSP_ADC_H_
#define BSP_ADC_H_

#include <stdint.h>

typedef enum {
    BSP_ADC_PRESS1 = 0,
    BSP_ADC_TEMP1,
    BSP_ADC_TEMP,
#define BSP_ADC_NUM 3
} bsp_adc_t;

void     bsp_adc_init(void);
void     bsp_adc_manage(void);
uint16_t bsp_adc_get(bsp_adc_t adc);

#endif /* BSP_ADC_H_ */
