#ifndef RISCALDAMENTO_H_INCLUDED
#define RISCALDAMENTO_H_INCLUDED


#include "model/model.h"
#include "state_machine.h"


typedef enum {
    RISCALDAMENTO_EVENT_CODE_ON,
    RISCALDAMENTO_EVENT_CODE_OFF, 
    RISCALDAMENTO_EVENT_CODE_CHECK,
} heating_event_code_t;


int  riscaldamento_ok(void);

void riscaldamento_manage_callbacks(model_t *pmodel);
void riscaldamento_on(model_t *pmodel);
void riscaldamento_off(model_t *pmodel);
int riscaldamento_timeout(model_t *pmodel);
void riscaldamento_refresh(model_t *pmodel);


#endif
