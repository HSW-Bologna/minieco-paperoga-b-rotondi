#ifndef DRUM_H_INCLUDED
#define DRUM_H_INCLUDED


#include <stdint.h>


void drum_run_forward(uint8_t speed);
void drum_run_backward(uint8_t speed);
void drum_stop(void);


#endif
