#ifndef RELAY_H_INCLUDED
#define RELAY_H_INCLUDED


#include <stdint.h>


#define BSP_RELAY_FORWARD  BSP_RELAY_6
#define BSP_RELAY_BACKWARD BSP_RELAY_5
#define BSP_RELAY_FAN      BSP_RELAY_4


typedef enum {
    BSP_RELAY_1 = 0,
    BSP_RELAY_2,
    BSP_RELAY_3,
    BSP_RELAY_4,
    BSP_RELAY_5,
    BSP_RELAY_6,
#define BSP_RELAY_NUM 6
} bsp_relay_t;


void bsp_relay_update(bsp_relay_t relay, uint8_t level);


#endif
