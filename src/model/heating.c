#include "cycle.h"
#include "services/timestamp.h"
#include "model.h"
#include "state_machine.h"
#include "stopwatch.h"
#include "stopwatch_timer.h"
#include "heating.h"


STATE_MACHINE_DEFINE(heating, heating_event_code_t);


static int  off_event_manager(heating_event_code_t event, void *arg);
static int  on_event_manager(heating_event_code_t event, void *arg);
static void on_entry(void *arg);
static int  setpoint_reached_event_manager(heating_event_code_t event, void *arg);


static const heating_node_t managers[] = {
    [HEATING_STATE_OFF]              = STATE_MACHINE_EVENT_MANAGER(off_event_manager),
    [HEATING_STATE_ON]               = STATE_MACHINE_MANAGER(on_event_manager, on_entry, NULL),
    [HEATING_STATE_SETPOINT_REACHED] = STATE_MACHINE_EVENT_MANAGER(setpoint_reached_event_manager),
};


void heating_init(mut_model_t *model) {
    model->run.heating.state_machine.nodes      = (heating_node_t *)managers;
    model->run.heating.state_machine.node_index = HEATING_STATE_OFF;
}


void heating_manage(mut_model_t *model) {
    heating_send_event(&model->run.heating.state_machine, model, RISCALDAMENTO_EVENT_CODE_CHECK);
}


void heating_on(mut_model_t *model) {
    heating_send_event(&model->run.heating.state_machine, model, RISCALDAMENTO_EVENT_CODE_ON);
}


void heating_off(mut_model_t *model) {
    heating_send_event(&model->run.heating.state_machine, model, RISCALDAMENTO_EVENT_CODE_OFF);
}


static int off_event_manager(heating_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case RISCALDAMENTO_EVENT_CODE_ON: {
            if (model_get_default_temperature(model) >
                model_get_setpoint(model) + model->run.parmac.temperature_heating_hysteresis) {
                return HEATING_STATE_SETPOINT_REACHED;
            } else {
                return HEATING_STATE_ON;
            }
            break;
        }

        default:
            break;
    }

    return -1;
}


static void on_entry(void *arg) {
    mut_model_t *model           = arg;
    model->run.heating.timestamp = timestamp_get();
}


static int on_event_manager(heating_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case RISCALDAMENTO_EVENT_CODE_CHECK:
            if (model_get_default_temperature(model) >
                model_get_setpoint(model) + model->run.parmac.temperature_heating_hysteresis) {
                return HEATING_STATE_SETPOINT_REACHED;
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
            if (model_get_default_temperature(model) <
                model_get_setpoint(model) - model->run.parmac.temperature_cooling_hysteresis) {
                return HEATING_STATE_ON;
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
