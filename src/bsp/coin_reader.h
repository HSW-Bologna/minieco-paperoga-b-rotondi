#ifndef BSP_COIN_READER_H_INCLUDED
#define BSP_COIN_READER_H_INCLUDED


#include <stdint.h>


typedef enum {
    BSP_COIN_READER_PAYMENT = 0,
    BSP_COIN_READER_LINE_1,
    BSP_COIN_READER_LINE_2,
    BSP_COIN_READER_LINE_3,
    BSP_COIN_READER_LINE_4,
    BSP_COIN_READER_LINE_5,
} bsp_coin_reader_line_t;


void     bsp_coin_reader_init(void);
void     bsp_coin_reader_enable(uint8_t enable);
void     bsp_coin_reader_manage(void);
uint16_t bsp_coin_reader_read(bsp_coin_reader_line_t line);
void     bsp_coin_reader_clear(void);


#endif
