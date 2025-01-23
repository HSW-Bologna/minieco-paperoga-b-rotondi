#ifndef RISCALDAMENTO_H_INCLUDED
#define RISCALDAMENTO_H_INCLUDED


#include "model/model.h"
#include "state_machine.h"


void heating_init(mut_model_t *pmodel);
void heating_on(mut_model_t *pmodel);
void heating_off(mut_model_t *pmodel);
void heating_manage(mut_model_t *pmodel);


#endif
