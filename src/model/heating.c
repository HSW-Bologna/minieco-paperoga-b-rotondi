#include "cycle.h"
#include "services/timestamp.h"
#include "model.h"
#include "state_machine.h"
#include "stopwatch.h"
#include "stopwatch_timer.h"
#include "heating.h"


STATE_MACHINE_INTERNAL(heating, heating_event_code_t);


static int off_event_manager(heating_event_code_t event, void *arg);
static int on_event_manager(heating_event_code_t event, void *arg);
static int midway_event_manager(heating_event_code_t event, void *arg);
static int setpoint_reached_event_manager(heating_event_code_t event, void *arg);


static const heating_node_t managers[] = {
    [HEATING_STATE_OFF]              = STATE_MACHINE_EVENT_MANAGER(off_event_manager),
    [HEATING_STATE_ON]               = STATE_MACHINE_EVENT_MANAGER(on_event_manager),
    [HEATING_STATE_MIDWAY]           = STATE_MACHINE_EVENT_MANAGER(midway_event_manager),
    [HEATING_STATE_SETPOINT_REACHED] = STATE_MACHINE_EVENT_MANAGER(setpoint_reached_event_manager),
};

static heating_state_machine_t state_machine = {
    .nodes      = (heating_node_t *)managers,
    .node_index = HEATING_STATE_OFF,
};


void heating_refresh(mut_model_t *model) {
    heating_send_event(&state_machine, model, RISCALDAMENTO_EVENT_CODE_CHECK);
    model_set_heating_state(model, (heating_state_t)state_machine.node_index);
}


void heating_on(mut_model_t *model) {
    heating_send_event(&state_machine, model, RISCALDAMENTO_EVENT_CODE_ON);
    model_set_heating_state(model, (heating_state_t)state_machine.node_index);
}


void heating_off(mut_model_t *model) {
    heating_send_event(&state_machine, model, RISCALDAMENTO_EVENT_CODE_OFF);
    model_set_heating_state(model, (heating_state_t)state_machine.node_index);
}


static int off_event_manager(heating_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case RISCALDAMENTO_EVENT_CODE_ON: {
            switch (model->run.parmac.drying_type) {
                case TIPO_RISCALDAMENTO_GAS:
                    if (model_get_default_temperature(model) > model_get_setpoint(model)) {
                        return HEATING_STATE_SETPOINT_REACHED;
                    } else {
                        return HEATING_STATE_ON;
                    }

                case TIPO_RISCALDAMENTO_ELETTRICO:
                    if (model_get_default_temperature(model) > model_get_setpoint(model)) {
                        return HEATING_STATE_SETPOINT_REACHED;
                    } else {
                        return HEATING_STATE_ON;
                    }
            }
            break;
        }

        default:
            break;
    }

    return -1;
}


static int on_event_manager(heating_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case RISCALDAMENTO_EVENT_CODE_CHECK:
            if (model_get_default_temperature(model) > model_get_setpoint(model)) {
                if (model->run.parmac.drying_type == TIPO_RISCALDAMENTO_ELETTRICO) {
                    return HEATING_STATE_MIDWAY;
                } else {
                    return HEATING_STATE_SETPOINT_REACHED;
                }
            }
            break;

        case RISCALDAMENTO_EVENT_CODE_OFF:
            return HEATING_STATE_OFF;

        default:
            break;
    }

    return -1;
}


static int midway_event_manager(heating_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case RISCALDAMENTO_EVENT_CODE_CHECK:
            if (model_get_default_temperature(model) >
                model_get_setpoint(model) + model->isteresi_temperatura_off_res1) {
                return HEATING_STATE_SETPOINT_REACHED;
            } else if (model_get_default_temperature(model) <
                       model_get_setpoint(model) - model->isteresi_temperatura_on_res2) {
                return HEATING_STATE_ON;
            }
            break;

        case RISCALDAMENTO_EVENT_CODE_OFF:
            return HEATING_STATE_OFF;

        default:
            break;
    }

    return -1;
}


static int setpoint_reached_event_manager(heating_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case RISCALDAMENTO_EVENT_CODE_CHECK: {
            if (model_get_default_temperature(model) < model_get_setpoint(model)) {
                if (model->run.parmac.drying_type == TIPO_RISCALDAMENTO_ELETTRICO) {
                    return HEATING_STATE_MIDWAY;
                } else {
                    return HEATING_STATE_ON;
                }
            }
            break;
        }

        case RISCALDAMENTO_EVENT_CODE_OFF:
            return HEATING_STATE_OFF;

        default:
            break;
    }

    return -1;
}
