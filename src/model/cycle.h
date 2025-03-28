#ifndef CYCLE_H_INCLUDED
#define CYCLE_H_INCLUDED


#include "model/model.h"
#include "state_machine.h"


void    cycle_init(mut_model_t *model);
uint8_t cycle_manage(mut_model_t *model);
void    cycle_start(mut_model_t *model);
void    cycle_check(mut_model_t *model);
void    cycle_cold_start(mut_model_t *model, uint16_t remaining_time);
void    cycle_increase_duration(mut_model_t *model, uint16_t seconds);
void    cycle_set_duration(mut_model_t *model, uint16_t seconds);


#endif
