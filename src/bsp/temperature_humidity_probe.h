#ifndef BSP_TEMPERATURE_HUMIDITY_PROBE_H_INCLUDED
#define BSP_TEMPERATURE_HUMIDITY_PROBE_H_INCLUDED


#include <stdint.h>


int temperature_humidity_probe_read(int16_t *temperature, uint16_t *humidity);


#endif
