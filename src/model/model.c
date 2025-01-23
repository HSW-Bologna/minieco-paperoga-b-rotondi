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


#define FLAG_ASCIUGATURA_TIPO_BIT               0
#define FLAG_ASCIUGATURA_ATTESA_TEMPERATURA_BIT 1
#define FLAG_ASCIUGATURA_INVERSIONE_BIT         2


static int16_t ptc_temperature_from_adc_value(uint16_t adc);
static void    set_drum_timestamp(mut_model_t *model);
static uint8_t get_pwm_drum_percentage(model_t *model);
static uint8_t get_pwm_fan_percentage(model_t *model);


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

    // Reset burner state while not running
    if (!model_is_cycle_active(model)) {
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
                    } else {
                        model->run.burner_reset_attempts++;
                    }

                    model->run.burner_ts    = timestamp_get();
                    model->run.burner_state = BURNER_STATE_RESETTING;
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
    }

    // Air not active or active and flowing
    if (!model_is_fan_on(model) || (model->run.sensors.inputs & (1 << INPUT_AIR_FLOW)) > 0) {
        model->run.air_flow_stopped_ts = timestamp_get();
    }
}


void model_clear_alarms(model_t *model) {
    assert(model != NULL);
    model->run.alarms       = 0;
    model->run.burner_alarm = 0;
    model_fix_alarms(model);
}


void model_fix_alarms(model_t *model) {
    assert(model != NULL);
    uint8_t porthole_was_open = model_is_porthole_open(model);

    model->run.alarms |= model_get_active_alarms(model);
    // The porthole is the only self-clearing alarm
    if (!model_is_porthole_open(model)) {
        model->run.alarms &= ~((uint16_t)0x01);
    }

    // The porthole is newly opened
    if (model_is_porthole_open(model) && !porthole_was_open) {
        model->run.porthole_opened_ts = timestamp_get();
    }
}


uint8_t model_is_emergency_alarm_active(model_t *model) {
    assert(model != NULL);
    return (model->run.sensors.inputs & (1 << INPUT_EMERGENCY)) == 0;
}


uint8_t model_is_filter_alarm_active(model_t *model) {
    assert(model != NULL);
    return (model->run.sensors.inputs & (1 << INPUT_FILTER_FEEDBACK)) == 0;
}


uint8_t model_is_porthole_open(model_t *model) {
    assert(model != NULL);
    uint8_t porthole_level = (model->run.sensors.inputs & (1 << INPUT_PORTHOLE));
    return model->run.parmac.porthole_nc_na ? porthole_level == 0 : porthole_level == 1;
}


uint16_t model_get_active_alarms(model_t *model) {
    assert(model != NULL);

    if (model->run.parmac.disable_alarms) {
        return 0;
    } else {
        uint8_t air_flow_alarm =
            model_is_fan_on(model) &&
            timestamp_is_expired(model->run.air_flow_stopped_ts, model->run.parmac.air_flow_alarm_time * 1000UL);

        return (uint16_t)(((model_is_porthole_open(model) > 0) << 0) |
                          ((model_is_emergency_alarm_active(model) > 0) << 1) |
                          ((model_is_filter_alarm_active(model) > 0) << 2) | ((air_flow_alarm > 0) << 3) |
                          ((model->run.burner_alarm > 0) << 4) | ((model_over_safety_temperature(model) > 0) << 5));
    }
}


void model_cycle_resume(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_START);
}


void model_cycle_pause(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_PAUSE);
}


void model_cycle_stop(mut_model_t *model) {
    cycle_send_event(&model->run.cycle.state_machine, model, CYCLE_EVENT_CODE_STOP);
}


uint32_t model_get_step_duration_seconds(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration * 60UL * 1000UL;
}


uint8_t model_is_step_endless(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration == 0xFFFF;
}


unsigned long model_get_cycle_remaining_time(mut_model_t *model) {
    if (model->run.cycle.state_machine.node_index == CYCLE_STATE_ACTIVE ||
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
    if (model->run.test.on) {
        return model->run.test.outputs;
    } else {
        uint16_t map = 0;

        {     // Busy signal management
            uint8_t busy_signal_active = 0;
            switch (model->run.parmac.busy_signal_type) {
                case BUSY_SIGNAL_TYPE_ALARMS_ACTIVITY:
                    busy_signal_active = (model->run.alarms > 0) || model_is_cycle_on(model);
                    break;

                case BUSY_SIGNAL_TYPE_ALARMS:
                    busy_signal_active = model->run.alarms > 0;
                    break;

                case BUSY_SIGNAL_TYPE_ACTIVITY:
                    busy_signal_active = model_is_cycle_on(model);
                    break;
            }
            if (model->run.parmac.busy_signal_nc_na) {
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
    if (model->run.parmac.invert_fan_drum) {
        return get_pwm_fan_percentage(model);
    } else {
        return get_pwm_drum_percentage(model);
    }
}


uint8_t model_get_pwm_fan_percentage(model_t *model) {
    assert(model != NULL);
    if (model->run.parmac.invert_fan_drum) {
        return get_pwm_drum_percentage(model);
    } else {
        return get_pwm_fan_percentage(model);
    }
}


void model_update_sensors(mut_model_t *model, uint16_t inputs, uint16_t temperature_input_adc,
                          uint16_t temperature_output_adc, int16_t temperature_probe, uint16_t humidity_probe) {
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

    if (model->run.sensors.inputs != inputs) {
        model->run.sensors.inputs = inputs;
        update                    = 1;
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
    size_t i = 2;
    for (j = 0; j < COIN_LINES; j++) {
        i += serialize_uint16_be(&buff[i], model->run.sensors.coins[j]);
    }

    uint16_t remaining_time = (uint16_t)model_get_cycle_remaining_time(model);
    uint8_t  active         = model_is_cycle_active(model);

    i += serialize_uint16_be(&buff[i], model->statisics.complete_cycles);
    i += serialize_uint16_be(&buff[i], model->statisics.partial_cycles);
    i += serialize_uint32_be(&buff[i], model->statisics.active_time);
    i += serialize_uint32_be(&buff[i], model->statisics.work_time);
    i += serialize_uint32_be(&buff[i], model->statisics.rotation_time);
    i += serialize_uint32_be(&buff[i], model->statisics.ventilation_time);
    i += serialize_uint32_be(&buff[i], model->statisics.heating_time);
    i += serialize_uint16_be(&buff[i], remaining_time);
    i += serialize_uint16_be(&buff[i], model->run.program_number);
    i += serialize_uint16_be(&buff[i], model->run.step_number);
    i += serialize_uint8(&buff[i], active);

    unsigned short crc = crc16_ccitt(&buff[2], (unsigned int)i - 2, 0);
    serialize_uint16_be(&buff[0], crc);
    assert(i == PWOFF_SERIALIZED_SIZE);
    return i;
}


int model_pwoff_deserialize(model_t *model, uint8_t *buff) {
    assert(model != NULL);
    size_t   j = 0;
    size_t   i = 0;
    uint16_t crc;
    i += deserialize_uint16_be(&crc, &buff[i]);
    if (crc != crc16_ccitt(&buff[2], PWOFF_SERIALIZED_SIZE - 2, 0)) {
        return -1;
    } else {
        uint16_t remaining_time = 0;
        uint8_t  active         = 0;

        for (j = 0; j < COIN_LINES; j++) {
            i += deserialize_uint16_be(&model->run.sensors.coins[j], &buff[i]);
        }
        i += deserialize_uint16_be(&model->statisics.complete_cycles, &buff[i]);
        i += deserialize_uint16_be(&model->statisics.partial_cycles, &buff[i]);
        i += deserialize_uint32_be(&model->statisics.active_time, &buff[i]);
        i += deserialize_uint32_be(&model->statisics.work_time, &buff[i]);
        i += deserialize_uint32_be(&model->statisics.rotation_time, &buff[i]);
        i += deserialize_uint32_be(&model->statisics.ventilation_time, &buff[i]);
        i += deserialize_uint32_be(&model->statisics.heating_time, &buff[i]);
        i += deserialize_uint16_be(&remaining_time, &buff[i]);
        i += deserialize_uint16_be(&model->run.program_number, &buff[i]);
        i += deserialize_uint16_be(&model->run.step_number, &buff[i]);
        i += deserialize_uint8(&active, &buff[i]);

        if (active) {
            cycle_cold_start(model, remaining_time);
        }
    }

    assert(i == PWOFF_SERIALIZED_SIZE);
    return (int)i;
}


void model_clear_coins(model_t *model) {
    assert(model != NULL);
    memset(model->run.sensors.coins, 0, sizeof(model->run.sensors.coins));
}


int model_tipo_asciugatura(model_t *model) {
    assert(model != NULL);
    return (model->flag_asciugatura & (1 << FLAG_ASCIUGATURA_TIPO_BIT)) > 0;
}


int model_attesa_temperatura_asciugatura(model_t *model) {
    assert(model != NULL);
    return (model->flag_asciugatura & (1 << FLAG_ASCIUGATURA_ATTESA_TEMPERATURA_BIT)) > 0;
}


int model_inversione_asciugatura(model_t *model) {
    assert(model != NULL);
    return (model->flag_asciugatura & (1 << FLAG_ASCIUGATURA_INVERSIONE_BIT)) > 0;
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
    return model->run.parmac.duration > 0 && model->run.parmac.rotation_pause_time == 0;
}


int model_ciclo_fermo(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.duration == 0;
}


uint16_t model_get_remaining_seconds(model_t *model) {
    assert(model != NULL);

    if (model_is_step_endless(model)) {
        return 0xFFFF;
    } else {
        switch (model->run.cycle.state_machine.node_index) {
            case CYCLE_STATE_STOPPED:
            case CYCLE_STATE_ACTIVE:
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
            int16_t rounding = (model->run.sensors.temperature_probe % 10) >= 5;
            return (int16_t)(model->run.sensors.temperature_probe / 10) + rounding;
        }

        default:
            return 0;
    }
}



void model_add_second(model_t *model) {
    assert(model != NULL);
    model->statisics.active_time++;
}


void model_add_work_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statisics.work_time += ms / 1000UL;
}


void model_add_rotation_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statisics.rotation_time += ms / 1000UL;
}


void model_add_ventilation_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statisics.ventilation_time += ms / 1000UL;
}


void model_add_heating_time_ms(model_t *model, unsigned long ms) {
    assert(model != NULL);
    model->statisics.heating_time += ms / 1000UL;
}


int model_over_safety_temperature(model_t *model) {
    assert(model != NULL);
    return model_get_default_temperature(model) > model->run.parmac.safety_temperature;
}


void model_add_complete_cycle(model_t *model) {
    assert(model != NULL);
    model->statisics.complete_cycles++;
}


void model_add_partial_cycle(model_t *model) {
    assert(model != NULL);
    model->statisics.partial_cycles++;
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
    model->run.drum.speed = (uint8_t)model->run.parmac.speed;
}


void model_drum_backward(mut_model_t *model) {
    assert(model != NULL);
    set_drum_timestamp(model);
    model->run.drum.state = DRUM_STATE_BACKWARD;
    model->run.drum.speed = (uint8_t)model->run.parmac.speed;
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
        return 1;
    } else {
        return 0;
    }
}


uint8_t model_is_fan_ok(model_t *model) {
    assert(model != NULL);
    return (model->run.sensors.inputs & 0x01) > 0;
}


uint8_t model_is_heating_ok(model_t *model) {
    assert(model != NULL);
    return model->run.heating.state_machine.node_index != HEATING_STATE_OFF ||
           ((model->run.sensors.inputs & 0x02) == 0);
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
           model->run.cycle.state_machine.node_index == CYCLE_STATE_ACTIVE;
}


void model_reset_burner(model_t *model) {
    assert(model != NULL);
    if (model->run.parmac.heating_type == HEATING_TYPE_GAS) {
        model->run.burner_ts    = timestamp_get();
        model->run.burner_state = BURNER_STATE_RESETTING;
    }
}


uint8_t model_cycles_exceeded(model_t *model) {
    assert(model != NULL);
    return model->run.parmac.max_cycles > 0 && model->run.cycle.num_cycles >= model->run.parmac.max_cycles;
}


static uint8_t get_pwm_drum_percentage(model_t *model) {
    assert(model != NULL);
    if (model->run.test.on) {
        return model->run.test.pwm2;
    } else if (model->run.drum.state == DRUM_STATE_STOPPED) {
        return 0;
    } else {
        return model->run.drum.speed;
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
