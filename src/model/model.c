#include "hal_data.h"
#include <string.h>
#include <assert.h>
#include "model.h"
#include "services/serializer.h"
#include "services/timestamp.h"
#include "services/crc16-ccitt.h"
#include "cycle.h"
#include "heating.h"
#include "config/app_config.h"


static uint8_t is_payment_present(model_t *model);
static int16_t ptc_temperature_from_adc_value(uint16_t adc);
static void    set_drum_timestamp(mut_model_t *model);
static uint8_t get_pwm_drum_percentage(model_t *model);
static uint8_t get_pwm_fan_percentage(model_t *model);
static uint8_t is_input_active(model_t *model, input_t input, direction_t direction);
static uint8_t communication_is_missing(model_t *model);


void model_init(model_t *model) {
    assert(model != NULL);
    memset(model, 0, sizeof(model_t));

    model->run.sensors.inputs = (1 << INPUT_EMERGENCY) | (1 << INPUT_PORTHOLE) | (1 << INPUT_FILTER_FEEDBACK);

    model->run.parmac.safety_temperature = 50;

    heating_init(model);
    cycle_init(model);
}


void model_manage(mut_model_t *model) {
    assert(model != NULL);

    heating_manage(model);
    cycle_manage(model);

    if (model->run.parmac.temperature_probe == TEMPERATURE_PROBE_SHT) {
        if (model->run.sensors.humidity_probe / 100 <= model->run.parmac.setpoint_humidity) {
            if (timestamp_is_expired(model->run.humidity_reached_ts, 10000UL)) {
                model->run.humidity_was_reached = 1;
            }
        } else {
            model->run.humidity_reached_ts = timestamp_get();
        }
    }

    // Reset burner state while not running
    if (!model_is_cycle_active(model)) {
        model->run.humidity_reached_ts   = timestamp_get();
        model->run.burner_ts             = timestamp_get();
        model->run.burner_reset_attempts = 0;
    }
    switch (model->run.burner_state) {
        case BURNER_STATE_OK: {
            if (model->run.parmac.heating_type == HEATING_TYPE_GAS &&
                model->run.sensors.inputs & (1 << INPUT_BURNER_ALARM)) {
                if (timestamp_is_expired(model->run.burner_ts, APP_CONFIG_BURNER_DELAY_TIME_MS)) {
                    if (model->run.burner_reset_attempts >= model->run.parmac.gas_ignition_attempts) {
                        model->run.burner_alarm          = 1;
                        model->run.burner_reset_attempts = 0;
                        model->run.burner_state          = BURNER_STATE_LOCKED;
                    } else {
                        model->run.burner_reset_attempts++;
                        model->run.burner_state = BURNER_STATE_RESETTING;
                    }

                    model->run.burner_ts = timestamp_get();
                }
            } else {
                model->run.burner_ts = timestamp_get();
            }
            break;
        }

        case BURNER_STATE_RESETTING: {
            if (model->run.sensors.inputs & (1 << INPUT_BURNER_ALARM)) {
                if (timestamp_is_expired(model->run.burner_ts, APP_CONFIG_BURNER_RESET_TIME_MS)) {
                    model->run.burner_ts    = timestamp_get();
                    model->run.burner_state = BURNER_STATE_DEBOUNCE;
                }
            } else {
                model->run.burner_state = BURNER_STATE_OK;
                model->run.burner_ts    = timestamp_get();
            }
            break;
        }

        case BURNER_STATE_DEBOUNCE: {
            if (timestamp_is_expired(model->run.burner_ts, APP_CONFIG_BURNER_DEBOUNCE_TIME_MS)) {
                model->run.burner_ts    = timestamp_get();
                model->run.burner_state = BURNER_STATE_OK;
            }
            break;
        }

        case BURNER_STATE_LOCKED: {
            break;
        }
    }

    uint8_t fan_on = model_is_fan_on(model);
    if (model->run.fan.previously_on && !fan_on) {
        model_add_ventilation_time_ms(model, timestamp_elapsed(model->run.fan.timestamp));
    } else if (!model->run.fan.previously_on && fan_on) {
        model->run.fan.timestamp = timestamp_get();
    }
    model->run.fan.previously_on = fan_on;

    // Air not active or active and flowing (either checked by the mechanical flap or the pressostat)
    if (!model_is_fan_on(model) ||
        (!model->run.parmac.pressostat && (model->run.sensors.inputs & (1 << INPUT_AIR_FLOW)) > 0) ||
        (model->run.parmac.pressostat && model_get_pressure(model) >= model->run.parmac.air_flow_maximum_pressure)) {
        model->run.air_flow_stopped_ts = timestamp_get();
    }
}


void model_clear_alarms(model_t *model) {
    assert(model != NULL);
    model->run.alarms                                   = 0;
    model->run.burner_alarm                             = 0;
    model->run.temperature_not_reached_alarm            = 0;
    model->run.sensors.temperature_humidity_probe_error = 0;
    model_fix_alarms(model);
}


void model_fix_alarms(model_t *model) {
    assert(model != NULL);
    if (model->run.parmac.disable_alarms) {
        model->run.alarms = 0;
    }
    // In test mode alarms are self-resetting
    else if (model->run.test.on) {
        model->run.alarms = model_get_active_alarms(model);
    }
    // Otherwise they stick
    else {
        model->run.alarms |= model_get_active_alarms(model);
    }

    // The porthole is the only self-clearing alarm
    if (!model_is_porthole_open(model)) {
        model->run.alarms &= ~((uint16_t)(1 << ALARM_CODE_OBLO_APERTO));
    }

    cycle_check(model);
}


uint8_t model_is_inverter_alarm_active(model_t *model) {
    assert(model != NULL);
    return is_input_active(model, INPUT_INVERTER_ALARM, model->run.parmac.inverter_alarm_nc_na);
}


uint8_t model_is_emergency_alarm_active(model_t *model) {
    assert(model != NULL);
    return is_input_active(model, INPUT_EMERGENCY, model->run.parmac.emergency_alarm_nc_na);
}


uint8_t model_is_filter_alarm_active(model_t *model) {
    assert(model != NULL);
    return is_input_active(model, INPUT_FILTER_FEEDBACK, model->run.parmac.filter_alarm_nc_na);
}


uint8_t model_is_porthole_open(model_t *model) {
    assert(model != NULL);
    return is_input_active(model, INPUT_PORTHOLE, model->run.parmac.porthole_nc_na);
}


uint16_t model_get_pressure(model_t *model) {
    assert(model != NULL);

    if (model->run.sensors.pressure_adc > model->run.pressure_offset) {
        return ((model->run.sensors.pressure_adc - model->run.pressure_offset) * 500) / 3686;
    } else {
        return 0;
    }
}


uint16_t model_get_active_alarms(model_t *model) {
    assert(model != NULL);

    if (model->run.parmac.disable_alarms || !model->run.initialized_by_master) {
        return 0;
    } else {
        uint8_t air_flow_alarm =
            model_is_fan_on(model) &&
            timestamp_is_expired(model->run.air_flow_stopped_ts, model->run.parmac.air_flow_alarm_time * 1000UL);
        uint8_t safety_pressure_alarm =
            model->run.parmac.pressostat && (model_get_pressure(model) >= model->run.parmac.air_flow_safety_pressure);

        return (uint16_t)(((model_is_porthole_open(model) > 0) << ALARM_CODE_OBLO_APERTO) |
                          ((model_is_emergency_alarm_active(model) > 0) << ALARM_CODE_EMERGENZA) |
                          ((model_is_filter_alarm_active(model) > 0) << ALARM_CODE_FILTRO) |
                          ((air_flow_alarm > 0) << ALARM_CODE_AIR_FLOW) |
                          ((model->run.burner_alarm > 0) << ALARM_CODE_BURNER) |
                          ((model_over_safety_temperature(model) > 0) << ALARM_CODE_SAFETY_TEMPERATURE) |
                          ((model->run.temperature_not_reached_alarm > 0) << ALARM_CODE_TEMPERATURE_NOT_REACHED) |
                          ((model_is_inverter_alarm_active(model) > 0) << ALARM_CODE_INVERTER) |
                          ((safety_pressure_alarm > 0) << ALARM_CODE_SAFETY_PRESSURE) |
                          ((model->run.sensors.temperature_humidity_probe_error > 0)
                           << ALARM_CODE_TEMPERATURE_HUMIDITY_PROBE));
    }
}


void model_cycle_resume(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_START);
}


void model_cycle_pause(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_PAUSE);
}


void model_cycle_standby(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_STEP_DONE);
}


void model_cycle_stop(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_STOP);
}


uint32_t model_get_step_duration_seconds(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration * 1000UL;
}


uint8_t model_is_step_endless(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration == 0xFFFF;
}


unsigned long model_get_cycle_remaining_time(mut_model_t *model) {
    if (model->run.cycle.state_machine.node_index == CYCLE_STATE_STANDBY ||
        model->run.cycle.state_machine.node_index == CYCLE_STATE_STOPPED) {
        return 0;
    } else {
        return stopwatch_get_remaining(&model->run.cycle.timer_cycle.stopwatch, timestamp_get()) / 1000UL;
    }
}


uint8_t model_is_cycle_on(model_t *model) {
    assert(model != NULL);
    return model->run.cycle.state_machine.node_index != CYCLE_STATE_STOPPED;
}


uint16_t model_get_relay_map(model_t *model) {
    assert(model != NULL);
    if (communication_is_missing(model)) {
        return 0;
    } else if (model->run.test.on) {
        return model->run.test.outputs;
    } else {
        uint16_t map = 0;

        {     // Busy signal management
            uint8_t busy_signal_active = 0;
            switch (model->run.parmac.busy_signal_type) {
                case BUSY_SIGNAL_TYPE_ALARMS_ACTIVITY:
                    busy_signal_active = ((model->run.alarms & (~(1 << ALARM_CODE_OBLO_APERTO))) > 0) ||
                                         model_is_cycle_on(model) || is_payment_present(model);
                    break;

                case BUSY_SIGNAL_TYPE_ALARMS:
                    busy_signal_active = (model->run.alarms & (~(1 << ALARM_CODE_OBLO_APERTO))) > 0;
                    break;

                case BUSY_SIGNAL_TYPE_ACTIVITY:
                    if (model_is_porthole_open(model)) {
                        busy_signal_active = 0;
                    } else {
                        busy_signal_active = ((model->run.alarms & (~(1 << ALARM_CODE_OBLO_APERTO))) > 0) ||
                                             model_is_cycle_on(model) || is_payment_present(model);
                    }
                    break;
            }
            if (!model->run.parmac.busy_signal_nc_na) {
                busy_signal_active = !busy_signal_active;
            }
            if (busy_signal_active) {
                map |= 1 << OUTPUT_BUSY;
            }
        }

        if (model_is_fan_on(model)) {
            map |= 1 << OUTPUT_FAN;
        }

        switch (model->run.drum.state) {
            case DRUM_STATE_STOPPED:
                break;

            case DRUM_STATE_FORWARD:
                map |= 1 << OUTPUT_FORWARD;
                break;

            case DRUM_STATE_BACKWARD:
                map |= 1 << OUTPUT_BACKWARD;
                break;
        }

        switch (model->run.heating.state_machine.node_index) {
            case HEATING_STATE_ON:
                map |= 1 << OUTPUT_HEATING;
                break;

            default:
                break;
        }

        switch (model->run.burner_state) {
            case BURNER_STATE_RESETTING:
                map |= 1 << OUTPUT_RESET_GAS;
                break;
            default:
                break;
        }

        return map;
    }
}


uint8_t model_get_pwm_drum_percentage(model_t *model) {
    assert(model != NULL);
    if (communication_is_missing(model)) {
        return 0;
    } else {
        return get_pwm_drum_percentage(model);
    }
}


uint8_t model_get_pwm_fan_percentage(model_t *model) {
    assert(model != NULL);
    if (communication_is_missing(model)) {
        return 0;
    } else {
        return get_pwm_fan_percentage(model);
    }
}


void model_update_sensors(mut_model_t *model, uint16_t inputs, uint16_t temperature_input_adc,
                          uint16_t temperature_output_adc, uint16_t pressure_adc, int16_t temperature_probe,
                          uint16_t humidity_probe) {
    assert(model != NULL);
    uint8_t update = 0;

    if (model->run.sensors.temperature_input_adc != temperature_input_adc) {
        model->run.sensors.temperature_input_adc = temperature_input_adc;
        model->run.sensors.temperature_input     = ptc_temperature_from_adc_value(temperature_input_adc);
        update                                   = 1;
    }

    if (model->run.sensors.temperature_output_adc != temperature_output_adc) {
        model->run.sensors.temperature_output_adc = temperature_output_adc;
        model->run.sensors.temperature_output     = ptc_temperature_from_adc_value(temperature_output_adc);
        update                                    = 1;
    }

    if (model->run.sensors.pressure_adc != pressure_adc) {
        model->run.sensors.pressure_adc = pressure_adc;
        update                          = 1;
    }

    if (model->run.sensors.inputs != inputs) {
        uint8_t porthole_was_open = model_is_porthole_open(model);

        model->run.sensors.inputs = inputs;
        update                    = 1;

        // The porthole is newly opened
        if (model_is_porthole_open(model) && !porthole_was_open) {
            model->run.porthole_opened_ts = timestamp_get();
        }
    }

    if (model->run.sensors.temperature_probe != temperature_probe) {
        model->run.sensors.temperature_probe = temperature_probe;
        update                               = 1;
    }

    if (model->run.sensors.humidity_probe != humidity_probe) {
        model->run.sensors.humidity_probe = humidity_probe;
        update                            = 1;
    }

    if (update) {
        cycle_check(model);
    }
}


size_t model_pwoff_serialize(model_t *model, uint8_t *buff) {
    assert(model != NULL);
    size_t j = 0;
    size_t i = 0;
    for (j = 0; j < COIN_LINES; j++) {
        i += serialize_uint16_be(&buff[i], model->run.sensors.coins[j]);
    }

    uint16_t remaining_time = (uint16_t)model_get_cycle_remaining_time(model);

    i += serialize_uint16_be(&buff[i], model->statistics.complete_cycles);
    i += serialize_uint16_be(&buff[i], model->statistics.partial_cycles);
    i += serialize_uint32_be(&buff[i], model->statistics.active_time);
    i += serialize_uint32_be(&buff[i], model->statistics.work_time);
    i += serialize_uint32_be(&buff[i], model->statistics.rotation_time);
    i += serialize_uint32_be(&buff[i], model->statistics.ventilation_time);
    i += serialize_uint32_be(&buff[i], model->statistics.heating_time);
    i += serialize_uint16_be(&buff[i], remaining_time);
    i += serialize_uint16_be(&buff[i], model->run.program_number);
    i += serialize_uint16_be(&buff[i], model->run.step_number);
    i += serialize_uint8(&buff[i], (uint8_t)model->run.cycle.state_machine.node_index);

    assert(i == PWOFF_SERIALIZED_SIZE);
    return i;
}


int model_pwoff_deserialize(model_t *model, uint8_t *buff) {
    assert(model != NULL);
    size_t   j              = 0;
    size_t   i              = 0;
    uint16_t remaining_time = 0;
    uint8_t  cycle_state    = 0;

    for (j = 0; j < COIN_LINES; j++) {
        i += deserialize_uint16_be(&model->run.sensors.coins[j], &buff[i]);
    }
    i += deserialize_uint16_be(&model->statistics.complete_cycles, &buff[i]);
    i += deserialize_uint16_be(&model->statistics.partial_cycles, &buff[i]);
    i += deserialize_uint32_be(&model->statistics.active_time, &buff[i]);
    i += deserialize_uint32_be(&model->statistics.work_time, &buff[i]);
    i += deserialize_uint32_be(&model->statistics.rotation_time, &buff[i]);
    i += deserialize_uint32_be(&model->statistics.ventilation_time, &buff[i]);
    i += deserialize_uint32_be(&model->statistics.heating_time, &buff[i]);
    i += deserialize_uint16_be(&remaining_time, &buff[i]);
    i += deserialize_uint16_be(&model->run.program_number, &buff[i]);
    i += deserialize_uint16_be(&model->run.step_number, &buff[i]);
    i += deserialize_uint8(&cycle_state, &buff[i]);

    switch (cycle_state) {
        case CYCLE_STATE_STANDBY:
        case CYCLE_STATE_RUNNING:
        case CYCLE_EVENT_CODE_PAUSE:
            cycle_cold_start(model, remaining_time);
            break;

        default:
            break;
    }

    assert(i == PWOFF_SERIALIZED_SIZE);
    return (int)i;
}


void model_clear_coins(model_t *model) {
    assert(model != NULL);
    memset(model->run.sensors.coins, 0, sizeof(model->run.sensors.coins));
}


int model_heating_enabled(model_t *model) {
    assert(model != NULL);
    return model->run.step_type == DRYER_PROGRAM_STEP_TYPE_DRYING;
}


int model_is_step_antifold(model_t *model) {
    assert(model != NULL);
    return model->run.step_type == DRYER_PROGRAM_STEP_TYPE_ANTIFOLD;
}


int model_ciclo_continuo(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration > 0 && model->run.parmac.enable_reverse == 0;
}


int model_ciclo_fermo(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration == 0;
}


uint16_t model_get_elapsed_seconds(model_t *model) {
    assert(model != NULL);

    if (model_is_step_endless(model)) {
        return 0xFFFF;
    } else {
        switch (model->run.cycle.state_machine.node_index) {
            case CYCLE_STATE_STOPPED:
            case CYCLE_STATE_STANDBY:
                return 0;
            default:
            case CYCLE_STATE_RUNNING:
            case CYCLE_STATE_WAIT_START:
            case CYCLE_STATE_PAUSED:
                return (uint16_t)(stopwatch_get_elapsed(&model->run.cycle.timer_cycle.stopwatch, timestamp_get()) /
                                  1000UL);
        }
        return 0;
    }
}


uint16_t model_get_remaining_seconds(model_t *model) {
    assert(model != NULL);

    if (model_is_step_endless(model)) {
        return 0xFFFF;
    } else {
        switch (model->run.cycle.state_machine.node_index) {
            case CYCLE_STATE_STOPPED:
            case CYCLE_STATE_STANDBY:
                return 0;
            default:
            case CYCLE_STATE_RUNNING:
            case CYCLE_STATE_WAIT_START:
            case CYCLE_STATE_PAUSED:
                return (uint16_t)(stopwatch_get_remaining(&model->run.cycle.timer_cycle.stopwatch, timestamp_get()) /
                                  1000UL);
        }
        return 0;
    }
}


int model_get_setpoint(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.setpoint_temperature;
}


int16_t model_get_default_temperature(model_t *model) {
    assert(model != NULL);

    switch (model->run.parmac.temperature_probe) {
        case TEMPERATURE_PROBE_INPUT:
            return (int16_t)model->run.sensors.temperature_input;

        case TEMPERATURE_PROBE_OUTPUT:
            return (int16_t)model->run.sensors.temperature_output;

        case TEMPERATURE_PROBE_SHT: {
            int16_t rounding = (model->run.sensors.temperature_probe % 100) >= 50;
            return (int16_t)(model->run.sensors.temperature_probe / 100) + rounding;
        }

        default:
            return 0;
    }
}


void model_add_work_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statistics.work_time += ms / 1000UL;
}


void model_add_rotation_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statistics.rotation_time += ms / 1000UL;
}


void model_add_ventilation_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statistics.ventilation_time += ms / 1000UL;
}


void model_add_heating_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statistics.heating_time += ms / 1000UL;
}


int model_over_safety_temperature(model_t *model) {
    assert(model != NULL);
    return model_get_default_temperature(model) > model->run.parmac.safety_temperature;
}


static int16_t ptc_temperature_from_adc_value(uint16_t adc) {
    const int16_t minimum_ad_value   = 833;
    const int16_t maximum_ad_value   = 1740;
    const int16_t minimum_temp_value = -10;
    const int16_t maximum_temp_value = 140;
    const int16_t coeff_q =
        (-minimum_ad_value * (maximum_temp_value - minimum_temp_value) / (maximum_ad_value - minimum_ad_value) +
         minimum_temp_value);

#define COEFF_M_TIMES(x) ((x * (maximum_temp_value - minimum_temp_value)) / (maximum_ad_value - minimum_ad_value))

    if (adc <= minimum_ad_value) {
        return minimum_temp_value;
    } else if (adc >= maximum_ad_value) {
        return maximum_temp_value;
    } else {
        return (int16_t)(COEFF_M_TIMES(adc) + coeff_q);
    }

#undef COEFF_M_TIMES
}


void model_drum_forward(mut_model_t *model) {
    assert(model != NULL);
    set_drum_timestamp(model);
    model->run.drum.state = DRUM_STATE_FORWARD;
}


void model_drum_backward(mut_model_t *model) {
    assert(model != NULL);
    set_drum_timestamp(model);
    model->run.drum.state = DRUM_STATE_BACKWARD;
}


void model_drum_stop(mut_model_t *model) {
    assert(model != NULL);
    unsigned long res = 0;

    if (model->run.drum.state != DRUM_STATE_STOPPED) {
        res                   = timestamp_elapsed(model->run.drum.timestamp);
        model->run.drum.state = DRUM_STATE_STOPPED;
    }

    model_add_rotation_time_ms(model, res);
}


uint8_t model_is_drum_running_forward(model_t *model) {
    assert(model != NULL);
    return model->run.drum.state == DRUM_STATE_FORWARD;
}


static void set_drum_timestamp(mut_model_t *model) {
    if (model->run.drum.state != DRUM_STATE_STOPPED) {
        model->run.drum.timestamp = timestamp_get();
    }
}


uint8_t model_is_fan_on(model_t *model) {
    if (model_is_cycle_active(model)) {
        switch (model->run.step_type) {
            // When drying or cooling always blow air
            case DRYER_PROGRAM_STEP_TYPE_DRYING:
            case DRYER_PROGRAM_STEP_TYPE_COOLING:
                return 1;

            // During antifold only blow air when the drum is moving
            case DRYER_PROGRAM_STEP_TYPE_ANTIFOLD: {
                switch (model->run.drum.state) {
                    case DRUM_STATE_FORWARD:
                    case DRUM_STATE_BACKWARD:
                        return 1;

                    default:
                        return 0;
                }
                break;
            }

            default:
                return 0;
        }
    }
    // If the cycle is stopped and the porthole has been open for a certain time keep blowing air
    else if (model_is_porthole_open(model) &&
             !timestamp_is_expired(model->run.porthole_opened_ts,
                                   model->run.parmac.fan_with_open_porthole_time * 1000UL)) {
        __NOP();
        __NOP();
        __NOP();
        return 1;
    } else {
        __NOP();
        __NOP();
        __NOP();
        return 0;
    }
}


uint8_t model_get_heating_alarm(model_t *model) {
    assert(model != NULL);
    return (model->run.heating.state_machine.node_index == HEATING_STATE_ON) &&
           timestamp_is_expired(model->run.heating.timestamp,
                                model->run.parmac.temperature_alarm_delay_seconds * 1000UL);
}


uint8_t model_is_cycle_active(model_t *model) {
    assert(model != NULL);
    return model->run.cycle.state_machine.node_index == CYCLE_STATE_RUNNING ||
           model->run.cycle.state_machine.node_index == CYCLE_STATE_STANDBY;
}


void model_reset_burner(model_t *model) {
    assert(model != NULL);
    if (model->run.parmac.heating_type == HEATING_TYPE_GAS) {
        model->run.burner_ts    = timestamp_get();
        model->run.burner_state = BURNER_STATE_RESETTING;
    }
}


void model_cycle_over(mut_model_t *model) {
    assert(model != NULL);
    if (model->run.cycle.completed) {
        model->statistics.complete_cycles++;
    } else {
        model->statistics.partial_cycles++;
    }

    model->statistics.work_time += timestamp_elapsed(model->run.cycle.start_ts) / 1000UL;
}


uint8_t model_is_time_held_by_humidity(model_t *model) {
    if (model->run.parmac.temperature_probe == TEMPERATURE_PROBE_SHT && model->run.parmac.wait_for_humidity) {
        // Only if we already reached the humidity setpoint at least once
        return !model->run.humidity_was_reached;
    } else {
        return 0;
    }
}


uint8_t model_is_time_held_by_temperature(model_t *model) {
    if (model->run.parmac.wait_for_temperature) {
        // Only if we already reached the setpoint at least once
        return !model->run.heating.temperature_was_reached;
    } else {
        return 0;
    }
}


static uint8_t get_pwm_drum_percentage(model_t *model) {
    assert(model != NULL);
    if (model->run.test.on) {
        return model->run.test.pwm2;
    } else if (model->run.drum.state == DRUM_STATE_STOPPED) {
        return 0;
    } else {
        return (uint8_t)model->run.parmac.speed;
    }
}


static uint8_t get_pwm_fan_percentage(model_t *model) {
    assert(model != NULL);
    if (model->run.test.on) {
        return model->run.test.pwm1;
    } else if (model_is_fan_on(model)) {
        return 100;
    } else {
        return 0;
    }
}


static uint8_t is_input_active(model_t *model, input_t input, direction_t direction) {
    uint8_t level = (uint8_t)((model->run.sensors.inputs & (1 << input)) > 0);
    return direction == DIRECTION_NA ? level == 0 : level > 0;
}


static uint8_t communication_is_missing(model_t *model) {
    return timestamp_is_expired(model->run.communication_ts, 2000);
}


static uint8_t is_payment_present(model_t *model) {
    for (uint16_t i = 0; i < COIN_LINES; i++) {
        if (model->run.sensors.coins[i] > 0) {
            return 1;
        }
    }
    return 0;
}
