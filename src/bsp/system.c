#include <stdint.h>
#include "hal_data.h"
#include "core_cm23.h"


static uint32_t millis = 0;


void SysTick_Handler(void)
{ /* SysTick interrupt Handler. */
    millis++; /* See startup file startup_LPC17xx.s for SysTick vector */
}


void bsp_system_init(void) {
    SysTick_Config(SystemCoreClock / 1000); //1000 is the number of ticks per second
    NVIC_SetPriority(SysTick_IRQn, 0); // Set User configured Priority for Systick Interrupt, 0 is the interrupt priority
}


uint32_t bsp_system_get_millis(void) {
    return millis;
}
