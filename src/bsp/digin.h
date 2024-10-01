#ifndef DIGIN_H_INCLUDED
#define DIGIN_H_INCLUDED


#include <stdint.h>


typedef enum {
    BSP_DIGIN_IN1 = 0,
    BSP_DIGIN_IN2,
    BSP_DIGIN_IN3,
    BSP_DIGIN_IN4,
    BSP_DIGIN_IN5,
    BSP_DIGIN_IN6,
    BSP_DIGIN_IN7,
} bsp_digin_t;


void     bsp_digin_init(void);
void     bsp_digin_manage(void);
uint8_t  bsp_digin_get(bsp_digin_t digin);
uint16_t bsp_digin_get_bitmap(void);


#endif
