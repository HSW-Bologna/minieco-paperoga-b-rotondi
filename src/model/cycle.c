#include "cycle.h"
#include "services/timestamp.h"
#include "model.h"
#include "state_machine.h"
#include "stopwatch.h"
#include "stopwatch_timer.h"
// #include "ventilazione.h"
#include "heating.h"


STATE_MACHINE_DEFINE(cycle, cycle_event_code_t);

static void    stopped_entry(void *arg);
static int     stopped_event_manager(cycle_event_code_t event, void *arg);
static int     standby_event_manager(cycle_event_code_t event, void *arg);
static int     wait_start_event_manager(cycle_event_code_t event, void *arg);
static int     running_event_manager(cycle_event_code_t event, void *arg);
static void    paused_entry(void *arg);
static void    paused_exit(void *arg);
static int     paused_event_manager(cycle_event_code_t event, void *arg);
static void    timer_event_callback(stopwatch_timer_t *timer, void *user_ptr);
static void    stop_everything(model_t *model);
static void    start_everything(model_t *model);
static void    fix_timer(stopwatch_timer_t *timer, unsigned long period, int arg);
static uint8_t should_time_be_running(model_t *model);
static void    prepare_cycle_time(mut_model_t *model, uint32_t duration, int code);


static const cycle_node_t managers[] = {
    // No program loaded, everything off
    [CYCLE_STATE_STOPPED] = STATE_MACHINE_MANAGER(stopped_event_manager, stopped_entry, NULL),
    // In between steps
    [CYCLE_STATE_STANDBY] = STATE_MACHINE_MANAGER(standby_event_manager, stopped_entry, NULL),
    // Delayed start
    [CYCLE_STATE_WAIT_START] = STATE_MACHINE_EVENT_MANAGER(wait_start_event_manager),
    // Running
    [CYCLE_STATE_RUNNING] = STATE_MACHINE_EVENT_MANAGER(running_event_manager),
    // Paused
    [CYCLE_STATE_PAUSED] = STATE_MACHINE_MANAGER(paused_event_manager, paused_entry, paused_exit),
};


void cycle_init(mut_model_t *model) {
    model->run.cycle.state_machine.nodes      = (cycle_node_t *)managers;
    model->run.cycle.state_machine.node_index = CYCLE_STATE_STOPPED;

    stopwatch_timer_init(&model->run.cycle.timer_cycle, 0, timer_event_callback, NULL);
    stopwatch_timer_init(&model->run.cycle.timer_rotation, 0, timer_event_callback, NULL);
    stopwatch_timer_set_autoreload(&model->run.cycle.timer_rotation, 1);
    stopwatch_timer_init(&model->run.cycle.timer_reset, 0, timer_event_callback, NULL);
}


uint8_t cycle_manage(mut_model_t *model) {
    stopwatch_timer_manage(&model->run.cycle.timer_cycle, timestamp_get(), model);
    stopwatch_timer_manage(&model->run.cycle.timer_rotation, timestamp_get(), model);
    stopwatch_timer_manage(&model->run.cycle.timer_reset, timestamp_get(), model);
    cycle_check(model);
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


void cycle_cold_start(mut_model_t *model, uint16_t remaining_time) {
    cycle_change_remaining_time(model, remaining_time);
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_COLD_START);
}


void cycle_check(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_CHECK);
}


void cycle_increase_duration(mut_model_t *model, uint16_t seconds) {
    switch (model->run.cycle.state_machine.node_index) {
        case CYCLE_STATE_RUNNING:
        case CYCLE_STATE_PAUSED: {
            stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;
            unsigned long      total       = stopwatch_get_total_time(&timer_cycle->stopwatch);
            stopwatch_timer_set_period(timer_cycle, total + seconds * 1000UL);
            break;
        }

        default:
            break;
    }
}


static void stopped_entry(void *arg) {
    model_t *model = arg;
    stop_everything(model);
}


static int stopped_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case CYCLE_EVENT_CODE_COLD_START:
            model_reset_burner(model);
            return CYCLE_STATE_PAUSED;

        case CYCLE_EVENT_CODE_START:
            // Do not start if there are alarms active or in test
            if (model->run.alarms || model->run.test.on) {
                break;
            }

            // controller_update_pwoff(model);
            model_reset_burner(model);
            model->run.cycle.completed = 0;

            if (model->run.parmac.cycle_delay_time > 0) {
                prepare_cycle_time(model, model->run.parmac.cycle_delay_time * 60UL * 1000UL,
                                   CYCLE_EVENT_CODE_WAIT_DONE);
                return CYCLE_STATE_WAIT_START;
            } else {
                model->run.cycle.start_ts = timestamp_get();
                start_everything(model);

                if (!model_is_step_endless(model)) {
                    prepare_cycle_time(model, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
                }

                model_clear_coins(model);
                return CYCLE_STATE_RUNNING;
            }

        default:
            break;
    }

    return -1;
}


static int running_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model          = arg;
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;
    stopwatch_timer_t *timer_cycle    = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_STOP:
            model_cycle_over(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_STEP_DONE:
            return CYCLE_STATE_STANDBY;

        case CYCLE_EVENT_CODE_PAUSE:
            return CYCLE_STATE_PAUSED;

        case CYCLE_EVENT_CODE_CHECK:
            if (should_time_be_running(model) && stopwatch_is_paused(&timer_cycle->stopwatch)) {
                stopwatch_timer_resume(timer_cycle, timestamp_get());
            } else if (!should_time_be_running(model) && !stopwatch_is_paused(&timer_cycle->stopwatch)) {
                stopwatch_timer_pause(timer_cycle, timestamp_get());
            }

            if (model->run.alarms) {
                if (model_is_porthole_open(model) && model->run.step_type == DRYER_PROGRAM_STEP_TYPE_ANTIFOLD) {
                    model_cycle_over(model);
                    return CYCLE_STATE_STOPPED;
                } else {
                    return CYCLE_STATE_PAUSED;
                }
            }
            break;

        case CYCLE_EVENT_CODE_MOTION_PAUSE:
            fix_timer(timer_rotation, model->run.parmac.rotation_pause_time * 1000UL,
                      model_is_drum_running_forward(model) ? CYCLE_EVENT_CODE_BACKWARD : CYCLE_EVENT_CODE_FORWARD);

            model_drum_stop(model);
            stopwatch_timer_resume(timer_rotation, timestamp_get());
            break;

        case CYCLE_EVENT_CODE_FORWARD:
            model_drum_forward(model);

            fix_timer(timer_rotation, model->run.parmac.rotation_running_time * 1000UL, CYCLE_EVENT_CODE_MOTION_PAUSE);
            stopwatch_timer_resume(timer_rotation, timestamp_get());
            break;

        case CYCLE_EVENT_CODE_BACKWARD:
            model_drum_backward(model);

            fix_timer(timer_rotation, model->run.parmac.rotation_running_time * 1000UL, CYCLE_EVENT_CODE_MOTION_PAUSE);
            stopwatch_timer_resume(timer_rotation, timestamp_get());
            break;

        default:
            break;
    }

    return -1;
}


static int wait_start_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t *model = arg;

    switch (event) {
        case CYCLE_EVENT_CODE_STOP:
            model_cycle_over(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_STEP_DONE:
            return CYCLE_STATE_STANDBY;

        case CYCLE_EVENT_CODE_WAIT_DONE: {
            model->run.cycle.start_ts = timestamp_get();

            if (!model_is_step_endless(model)) {
                prepare_cycle_time(model, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
            }

            model_clear_coins(model);
            start_everything(model);
            return CYCLE_STATE_RUNNING;
        }

        case CYCLE_EVENT_CODE_PAUSE:
            return CYCLE_STATE_PAUSED;

        case CYCLE_EVENT_CODE_CHECK:
            if (model->run.alarms) {
                if (model_is_porthole_open(model) && model->run.step_type == DRYER_PROGRAM_STEP_TYPE_ANTIFOLD) {
                    model_cycle_over(model);
                    return CYCLE_STATE_STOPPED;
                } else {
                    return CYCLE_STATE_PAUSED;
                }
            }
            break;

        default:
            break;
    }

    return -1;
}


static void paused_entry(void *arg) {
    model_t           *model          = arg;
    stopwatch_timer_t *timer_cycle    = &model->run.cycle.timer_cycle;
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;
    stopwatch_timer_t *timer_reset    = &model->run.cycle.timer_reset;
    model_drum_stop(model);

    heating_off(model);
    if (model->run.parmac.stop_time_in_pause) {
        stopwatch_timer_pause(timer_cycle, timestamp_get());

        if (model->run.parmac.cycle_reset_time > 0) {
            fix_timer(timer_reset, model->run.parmac.cycle_reset_time * 1000UL, CYCLE_EVENT_CODE_STOP);
            stopwatch_timer_resume(timer_reset, timestamp_get());
        }
    }
    stopwatch_timer_pause(timer_rotation, timestamp_get());
}


static void paused_exit(void *arg) {
    model_t           *model       = arg;
    stopwatch_timer_t *timer_reset = &model->run.cycle.timer_reset;
    stopwatch_timer_pause(timer_reset, timestamp_get());
}


static int paused_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model       = arg;
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_START:
            // Do not start if there are alarms active or in test
            if (model->run.alarms || model->run.test.on) {
                break;
            }

            start_everything(model);

            // Restart unless we have to wait for temperature
            if (should_time_be_running(model)) {
                stopwatch_timer_resume(timer_cycle, timestamp_get());
            }

            return CYCLE_STATE_RUNNING;

        case CYCLE_EVENT_CODE_STOP:
            model_cycle_over(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_STEP_DONE:
            return CYCLE_STATE_STANDBY;

        default:
            break;
    }

    return -1;
}


static int standby_event_manager(cycle_event_code_t event, void *arg) {
    mut_model_t       *model       = arg;
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;

    switch (event) {
        case CYCLE_EVENT_CODE_PAUSE:
            if (!model_is_step_endless(model)) {
                fix_timer(timer_cycle, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
            }

            if (model->run.parmac.stop_time_in_pause) {
                stopwatch_timer_pause(timer_cycle, timestamp_get());
            } else if (should_time_be_running(model)) {
                stopwatch_timer_resume(timer_cycle, timestamp_get());
            }
            return CYCLE_STATE_PAUSED;

        case CYCLE_EVENT_CODE_START:
            // Do not start if there are alarms active or in test
            if (model->run.alarms || model->run.test.on) {
                break;
            }

            if (!model_is_step_endless(model)) {
                prepare_cycle_time(model, model_get_step_duration_seconds(model), CYCLE_EVENT_CODE_STEP_DONE);
            }

            // When we reach the antifold step we consider the cycle to be complete
            if (model->run.step_type == DRYER_PROGRAM_STEP_TYPE_ANTIFOLD) {
                model->run.cycle.completed = 1;
            }

            start_everything(model);
            return CYCLE_STATE_RUNNING;

        case CYCLE_EVENT_CODE_STOP:
            model_cycle_over(model);
            return CYCLE_STATE_STOPPED;

        case CYCLE_EVENT_CODE_CHECK:
            if (model->run.alarms) {
                return CYCLE_STATE_PAUSED;
            }
            break;

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

    heating_off(model);
    model->run.heating.temperature_was_reached = 0;
}


static void start_everything(model_t *model) {
    stopwatch_timer_t *timer_rotation = &model->run.cycle.timer_rotation;

    if (model_heating_enabled(model)) {
        heating_on(model);
    } else {
        heating_off(model);
    }

    if (!model_ciclo_continuo(model)) {
        model_drum_forward(model);

        fix_timer(timer_rotation, model->run.parmac.rotation_running_time * 1000UL, CYCLE_EVENT_CODE_MOTION_PAUSE);
        stopwatch_timer_resume(timer_rotation, timestamp_get());
    }
}


static void fix_timer(stopwatch_timer_t *timer, unsigned long period, int arg) {
    stopwatch_timer_set_period(timer, period);
    timer->arg = (void *)(uintptr_t)arg;
    // stopwatch_timer_set_arg(timer, (void *)(uintptr_t)arg);
    stopwatch_timer_reset(timer, timestamp_get());
    stopwatch_timer_pause(timer, timestamp_get());
}


static uint8_t should_time_be_running(model_t *model) {
    if (model->run.parmac.wait_for_temperature) {
        // Only if we already reached the setpoint at least once
        return model->run.heating.temperature_was_reached;
    } else {
        return 1;
    }
}


static void prepare_cycle_time(mut_model_t *model, uint32_t duration, int code) {
    stopwatch_timer_t *timer_cycle = &model->run.cycle.timer_cycle;
    fix_timer(timer_cycle, duration, code);

    if (should_time_be_running(model)) {
        stopwatch_timer_resume(timer_cycle, timestamp_get());
    }
}
