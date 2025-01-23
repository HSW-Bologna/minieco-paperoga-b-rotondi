#ifndef DIGIN_H_INCLUDED
#define DIGIN_H_INCLUDED


#include <stdint.h>


#define BSP_DIGIN_BLOCCO_BRUCIATORE    BSP_DIGIN_IN8
#define BSP_DIGIN_FILTRO_APERTO        BSP_DIGIN_IN7
#define BSP_DIGIN_PAGAMENTO            BSP_DIGIN_IN6
#define BSP_DIGIN_ALLARMI_INVERTER     BSP_DIGIN_IN5
#define BSP_DIGIN_FLUSSO_ARIA          BSP_DIGIN_IN4
#define BSP_DIGIN_EMERGENZA            BSP_DIGIN_IN3
#define BSP_DIGIN_OBLO                 BSP_DIGIN_IN2
#define BSP_DIGIN_TERMOSTATO_SICUREZZA BSP_DIGIN_IN1


typedef enum {
    BSP_DIGIN_IN1 = 0,
    BSP_DIGIN_IN2,
    BSP_DIGIN_IN3,
    BSP_DIGIN_IN4,
    BSP_DIGIN_IN5,
    BSP_DIGIN_IN6,
    BSP_DIGIN_IN7,
    BSP_DIGIN_IN8,
} bsp_digin_t;


void     bsp_digin_init(void);
void     bsp_digin_manage(void);
uint8_t  bsp_digin_get(bsp_digin_t digin);
uint16_t bsp_digin_get_bitmap(void);
uint8_t  bsp_digin_is_ready(void);


#endif
