#include "cycle.h"
#include "services/timestamp.h"
#include "model.h"
#include "state_machine.h"
#include "stopwatch.h"
#include "stopwatch_timer.h"
// #include "ventilazione.h"
// #include "riscaldamento.h"


STATE_MACHINE_DEFINE(cycle, cycle_event_code_t);

static int  stopped_event_manager(cycle_event_code_t event, void *arg);
static int  active_event_manager(cycle_event_code_t event, void *arg);
static int  wait_start_event_manager(cycle_event_code_t event, void *arg);
static int  running_event_manager(cycle_event_code_t event, void *arg);
static int  paused_event_manager(cycle_event_code_t event, void *arg);
static void timer_event_callback(stopwatch_timer_t *timer, void *user_ptr);
static void stop_everything(model_t *model);
static void start_everything(model_t *model);
static void pause_everything(model_t *model);
static void fix_timer(stopwatch_timer_t *timer, unsigned long period, int arg);


static const cycle_node_t managers[] = {
    // No program loaded, everything off
    [CYCLE_STATE_STOPPED] = STATE_MACHINE_EVENT_MANAGER(stopped_event_manager),
    // In between steps
    [CYCLE_STATE_ACTIVE] = STATE_MACHINE_EVENT_MANAGER(active_event_manager),
    // Delayed start
    [CYCLE_STATE_WAIT_START] = STATE_MACHINE_EVENT_MANAGER(wait_start_event_manager),
    // Running
    [CYCLE_STATE_RUNNING] = STATE_MACHINE_EVENT_MANAGER(running_event_manager),
    // Paused
    [CYCLE_STATE_PAUSED] = STATE_MACHINE_EVENT_MANAGER(paused_event_manager),
};


void cycle_init(mut_model_t *model) {
    model->run.cycle.state_machine.nodes      = (cycle_node_t *)managers;
    model->run.cycle.state_machine.node_index = CYCLE_STATE_STOPPED;

    stopwatch_timer_init(&model->run.cycle.timer_cycle, 0, timer_event_callback, NULL);
    stopwatch_timer_init(&model->run.cycle.timer_rotation, 0, timer_event_callback, NULL);
}


uint8_t cycle_manage(mut_model_t *model) {
    stopwatch_timer_manage(&model->run.cycle.timer_cycle, timestamp_get(), model);
    stopwatch_timer_manage(&model->run.cycle.timer_rotation, timestamp_get(), model);
    return 0;
}


void cycle_change_remaining_time(mut_model_t *model, uint16_t seconds) {
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    stopwatch_timer_pause(timer_cycle, timestamp_get());
    stopwatch_timer_set_period(timer_cycle, seconds * 1000UL);

    if (timer_cycle->stopwatch.paused) {
        stopwatch_timer_reset(timer_cycle, timestamp_get());
    } else {
        stopwatch_timer_resume(timer_cycle, timestamp_get());
    }
}


void cycle_start(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_START);
}


static int stopped_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model       = arg;
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_START:
            // Do not start if there are alarms active
            if (model_get_alarms(model) > 0) {
                break;
            }

            // controller_update_pwoff(model);

            if (model->run.parmac.cycle_delay_time > 0) {
                model_fan_on(model);
                fix_timer(timer_cycle, model->run.parmac.cycle_delay_time * 1000UL, CYCLE_EVENT_CODE_STEP_DONE);
                stopwatch_timer_resume(timer_cycle, timestamp_get());
                return CYCLE_STATE_WAIT_START;
            } else {
                start_everything(model);

                fix_timer(timer_cycle, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
                stopwatch_timer_resume(timer_cycle, timestamp_get());

                return CYCLE_STATE_RUNNING;
            }

        case CYCLE_EVENT_CODE_RESUME:
            fix_timer(timer_cycle, model->pwoff.remaining_time * 1000UL, CYCLE_EVENT_CODE_STEP_DONE);
            stopwatch_timer_pause(timer_cycle, timestamp_get());
            // controller_update_pwoff(model);
            return CYCLE_STATE_PAUSED;

        default:
            break;
    }

    return -1;
}



static int running_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model          = arg;
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;

    switch (event) {
        case CYCLE_EVENT_CODE_STOP:
            stop_everything(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_STEP_DONE:
            return CYCLE_STATE_ACTIVE;

        case CYCLE_EVENT_CODE_ALARM:
        case CYCLE_EVENT_CODE_PAUSE:
            pause_everything(model);
            return CYCLE_STATE_PAUSED;

        case CYCLE_EVENT_CODE_MOTION_PAUSE:
            fix_timer(timer_rotation, model->run.parmac.rotation_pause_time * 1000UL,
                      model_is_drum_running_forward(model) ? CYCLE_EVENT_CODE_BACKWARD : CYCLE_EVENT_CODE_FORWARD);
            stopwatch_timer_resume(timer_rotation, timestamp_get());

            if (model_is_step_unfolding(model)) {
                // unsigned long tempo_ventilazione = ventilazione_off();
                // model_add_ventilation_time_ms(model, tempo_ventilazione);
            }

            model_drum_stop(model);
            break;

        case CYCLE_EVENT_CODE_FORWARD:
            model_drum_forward(model);
            model_fan_on(model);

            fix_timer(timer_rotation, model->run.parmac.rotation_running_time * 1000UL, CYCLE_EVENT_CODE_MOTION_PAUSE);
            stopwatch_timer_resume(timer_rotation, timestamp_get());
            break;

        case CYCLE_EVENT_CODE_BACKWARD:
            model_drum_backward(model);
            model_fan_on(model);

            fix_timer(timer_rotation, model->run.parmac.rotation_running_time * 1000UL, CYCLE_EVENT_CODE_MOTION_PAUSE);
            stopwatch_timer_resume(timer_rotation, timestamp_get());
            break;

        default:
            break;
    }

    return -1;
}


static int wait_start_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model       = arg;
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_STOP:
            stop_everything(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_STEP_DONE: {
            model_fan_off(model);

            fix_timer(timer_cycle, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
            stopwatch_timer_resume(timer_cycle, timestamp_get());

            start_everything(model);
            return CYCLE_STATE_RUNNING;
        }

        case CYCLE_EVENT_CODE_ALARM:
        case CYCLE_EVENT_CODE_PAUSE:
            pause_everything(model);
            return CYCLE_STATE_PAUSED;

        default:
            break;
    }

    return -1;
}


static int paused_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model       = arg;
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_START:
            // Do not start if there are alarms active
            if (model_get_alarms(model) > 0) {
                break;
            }

            start_everything(model);
            stopwatch_timer_resume(timer_cycle, timestamp_get());
            return CYCLE_STATE_RUNNING;

        case CYCLE_EVENT_CODE_STOP:
            stop_everything(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_STEP_DONE:
            stop_everything(model);
            return CYCLE_STATE_ACTIVE;

        default:
            break;
    }

    return -1;
}


static int active_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model       = arg;
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_PAUSE:
            fix_timer(timer_cycle, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);

            if (model->run.parmac.stop_time_in_pause) {
                stopwatch_timer_pause(timer_cycle, timestamp_get());
            } else {
                stopwatch_timer_resume(timer_cycle, timestamp_get());
            }
            return CYCLE_STATE_PAUSED;


        case CYCLE_EVENT_CODE_START:
            // Do not start if there are alarms active
            if (model_get_alarms(model) > 0) {
                break;
            }

            start_everything(model);

            fix_timer(timer_cycle, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
            stopwatch_timer_resume(timer_cycle, timestamp_get());

            return CYCLE_STATE_RUNNING;

        case CYCLE_EVENT_CODE_STOP:
            stop_everything(model);
            // controller_update_pwoff(model);
            return CYCLE_STATE_STOPPED;

        default:
            break;
    }

    return -1;
}


static void timer_event_callback(stopwatch_timer_t *timer, void *user_ptr) {
    mut_model_t *model = user_ptr;

    cycle_send_event(&model->run.cycle.state_machine, model, (cycle_event_code_t)(uintptr_t)timer->arg);
}


static void stop_everything(model_t *model) {
    stopwatch_timer_t *timer_cycle    = &model->run.cycle.timer_cycle;
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;

    model_drum_stop(model);

    model_add_work_time_ms(model, stopwatch_get_elapsed(&timer_cycle->stopwatch, timestamp_get()));

    stopwatch_timer_reset(timer_cycle, timestamp_get());
    stopwatch_timer_pause(timer_cycle, timestamp_get());

    stopwatch_timer_reset(timer_rotation, timestamp_get());
    stopwatch_timer_pause(timer_rotation, timestamp_get());

    model_fan_off(model);

    // riscaldamento_off(model);
}


static void start_everything(model_t *model) {
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;

    if (model_heating_enabled(model)) {
        // riscaldamento_on(model);
    } else {
        // riscaldamento_off(model);
    }

    if (!model_ciclo_fermo(model)) {
        model_drum_forward(model);
        model_fan_on(model);
    } else {
        model_fan_off(model);
    }

    if (!model_ciclo_continuo(model)) {
        fix_timer(timer_rotation, model->run.parmac.rotation_running_time * 1000UL, CYCLE_EVENT_CODE_MOTION_PAUSE);
        stopwatch_timer_resume(timer_rotation, timestamp_get());
    }
}


static void pause_everything(model_t *model) {
    stopwatch_timer_t *timer_cycle    = &model->run.cycle.timer_cycle;
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;
    model_drum_stop(model);

    model_fan_off(model);

    // riscaldamento_off(model);
    if (model->run.parmac.stop_time_in_pause) {
        stopwatch_timer_pause(timer_cycle, timestamp_get());
    }
    stopwatch_timer_pause(timer_rotation, timestamp_get());
}


static void fix_timer(stopwatch_timer_t *timer, unsigned long period, int arg) {
    stopwatch_timer_set_period(timer, period);
    stopwatch_timer_set_arg(timer, (void *)(uintptr_t)arg);
    stopwatch_timer_reset(timer, timestamp_get());
    stopwatch_timer_pause(timer, timestamp_get());
}
