#ifndef BSP_POWER_OFF_H_INCLUDED
#define BSP_POWER_OFF_H_INCLUDED


#include <stdint.h>


void    bsp_power_off_init(void (*callback)(void *), void *data);
void    bsp_power_off_save(uint8_t *data, uint16_t len);
uint8_t bsp_power_off_load(uint8_t **const data, uint32_t *const len);


#endif
