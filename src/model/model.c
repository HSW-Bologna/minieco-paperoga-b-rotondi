#include <string.h>
#include <assert.h>
#include "model.h"
#include "services/serializer.h"
#include "services/timestamp.h"
#include "services/crc16-ccitt.h"
#include "cycle.h"


#define FLAG_ASCIUGATURA_TIPO_BIT               0
#define FLAG_ASCIUGATURA_ATTESA_TEMPERATURA_BIT 1
#define FLAG_ASCIUGATURA_INVERSIONE_BIT         2


static int16_t ptc_temperature_from_adc_value(uint16_t adc);
static void    set_drum_timestamp(mut_model_t *model);


void model_init(model_t *model) {
    assert(model != NULL);
    memset(model, 0, sizeof(model_t));

    cycle_init(model);
}


void model_manage(mut_model_t *model) {
    assert(model != NULL);

    cycle_manage(model);
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
    return (model->run.sensors.inputs & (1 << INPUT_PORTHOLE)) == 0;
}


uint16_t model_get_alarms(model_t *model) {
    assert(model != NULL);
    return (uint16_t)(((model_is_emergency_alarm_active(model) > 0) << 0) |
                      ((model_is_filter_alarm_active(model) > 0) << 1) | ((model_is_porthole_open(model) > 0) << 2));
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

    switch (model->run.step_type) {
        case DRYER_PROGRAM_STEP_TYPE_DRYING:
            return model->run.parmac.drying_duration * 1000UL;

        case DRYER_PROGRAM_STEP_TYPE_COOLING:
            return 0;

        case DRYER_PROGRAM_STEP_TYPE_UNFOLDING:
            return 0;
    }

    return 0;
}


unsigned long model_get_cycle_remaining_time(mut_model_t *model) {
    if (model->run.cycle.state_machine.node_index == CYCLE_STATE_ACTIVE ||
        model->run.cycle.state_machine.node_index == CYCLE_STATE_STOPPED) {
        return 0;
    } else {
        return stopwatch_get_remaining(&model->run.cycle.timer_cycle.stopwatch, timestamp_get()) / 1000UL;
    }
}


uint16_t model_get_relay_map(model_t *model) {
    assert(model != NULL);
    if (model->run.test.on) {
        return model->run.test.outputs;
    } else {
        uint16_t map = 0;

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

        return map;
    }
}


uint8_t model_get_pwm1_percentage(model_t *model) {
    assert(model != NULL);
    if (model->run.test.on) {
        return model->run.test.pwm1;
    } else if (model->run.drum.state == DRUM_STATE_STOPPED) {
        return 0;
    } else {
        return model->run.drum.speed;
    }
}


uint8_t model_get_pwm2_percentage(model_t *model) {
    assert(model != NULL);
    if (model->run.test.on) {
        return model->run.test.pwm2;
    } else {
        return 0;
    }
}


void model_update_temperature(mut_model_t *model, temperature_probe_t temperature_probe, uint16_t adc) {
    assert(model != NULL);

    switch (temperature_probe) {
        case TEMPERATURE_PROBE_PTC_1:
            model->run.sensors.temperature_1_adc = adc;
            model->run.sensors.temperature_1     = ptc_temperature_from_adc_value(adc);
            break;

        case TEMPERATURE_PROBE_PTC_2:
            model->run.sensors.temperature_2_adc = adc;
            model->run.sensors.temperature_2     = ptc_temperature_from_adc_value(adc);
            break;

        case TEMPERATURE_PROBE_SHT:
            break;
    }
}


size_t model_pwoff_serialize(model_t *pmodel, uint8_t *buff) {
    assert(pmodel != NULL);
    size_t j = 0;
    size_t i = 2;
    for (j = 0; j < COIN_LINES; j++) {
        i += serialize_uint16_be(&buff[i], pmodel->pwoff.coins[j]);
    }
    i += serialize_uint16_be(&buff[i], pmodel->pwoff.complete_cycles);
    i += serialize_uint16_be(&buff[i], pmodel->pwoff.partial_cycles);
    i += serialize_uint32_be(&buff[i], pmodel->pwoff.active_time);
    i += serialize_uint32_be(&buff[i], pmodel->pwoff.work_time);
    i += serialize_uint32_be(&buff[i], pmodel->pwoff.rotation_time);
    i += serialize_uint32_be(&buff[i], pmodel->pwoff.ventilation_time);
    i += serialize_uint32_be(&buff[i], pmodel->pwoff.heating_time);
    i += serialize_uint16_be(&buff[i], pmodel->pwoff.remaining_time);
    i += serialize_uint16_be(&buff[i], pmodel->pwoff.program_number);
    i += serialize_uint16_be(&buff[i], pmodel->pwoff.step_number);
    i += serialize_uint8(&buff[i], pmodel->pwoff.active);

    unsigned short crc = crc16_ccitt(&buff[2], (unsigned int)i - 2, 0);
    serialize_uint16_be(&buff[0], crc);
    assert(i == PWOFF_SERIALIZED_SIZE);
    return i;
}


int model_pwoff_deserialize(model_t *pmodel, uint8_t *buff) {
    assert(pmodel != NULL);
    size_t   j = 0;
    size_t   i = 0;
    uint16_t crc;
    i += deserialize_uint16_be(&crc, &buff[i]);
    if (crc != crc16_ccitt(&buff[2], PWOFF_SERIALIZED_SIZE - 2, 0)) {
        return -1;
    } else {
        for (j = 0; j < COIN_LINES; j++) {
            i += deserialize_uint16_be(&pmodel->pwoff.coins[j], &buff[i]);
        }
        i += deserialize_uint16_be(&pmodel->pwoff.complete_cycles, &buff[i]);
        i += deserialize_uint16_be(&pmodel->pwoff.partial_cycles, &buff[i]);
        i += deserialize_uint32_be(&pmodel->pwoff.active_time, &buff[i]);
        i += deserialize_uint32_be(&pmodel->pwoff.work_time, &buff[i]);
        i += deserialize_uint32_be(&pmodel->pwoff.rotation_time, &buff[i]);
        i += deserialize_uint32_be(&pmodel->pwoff.ventilation_time, &buff[i]);
        i += deserialize_uint32_be(&pmodel->pwoff.heating_time, &buff[i]);
        i += deserialize_uint16_be(&pmodel->pwoff.remaining_time, &buff[i]);
        i += deserialize_uint16_be(&pmodel->pwoff.program_number, &buff[i]);
        i += deserialize_uint16_be(&pmodel->pwoff.step_number, &buff[i]);
        i += deserialize_uint8(&pmodel->pwoff.active, &buff[i]);
    }

    assert(i == PWOFF_SERIALIZED_SIZE);
    return (int)i;
}


void model_clear_coins(model_t *pmodel) {
    assert(pmodel != NULL);
    memset(pmodel->pwoff.coins, 0, sizeof(pmodel->pwoff.coins));
}


int model_tipo_asciugatura(model_t *pmodel) {
    assert(pmodel != NULL);
    return (pmodel->flag_asciugatura & (1 << FLAG_ASCIUGATURA_TIPO_BIT)) > 0;
}


int model_attesa_temperatura_asciugatura(model_t *pmodel) {
    assert(pmodel != NULL);
    return (pmodel->flag_asciugatura & (1 << FLAG_ASCIUGATURA_ATTESA_TEMPERATURA_BIT)) > 0;
}


int model_inversione_asciugatura(model_t *pmodel) {
    assert(pmodel != NULL);
    return (pmodel->flag_asciugatura & (1 << FLAG_ASCIUGATURA_INVERSIONE_BIT)) > 0;
}


int model_heating_enabled(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.step_type == DRYER_PROGRAM_STEP_TYPE_DRYING;
}


int model_is_step_unfolding(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.step_type == DRYER_PROGRAM_STEP_TYPE_UNFOLDING;
}


int model_ciclo_continuo(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.parmac.drying_duration > 0 && pmodel->run.parmac.rotation_pause_time == 0;
}


int model_ciclo_fermo(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.parmac.drying_duration == 0;
}


uint16_t model_get_remaining_seconds(model_t *model) {
    assert(model != NULL);
    switch (model->run.cycle.state_machine.node_index) {
        case CYCLE_STATE_STOPPED:
            return 0;
        default:
            return (uint16_t)(stopwatch_get_remaining(&model->run.cycle.timer_cycle.stopwatch, timestamp_get()) /
                              1000UL);
    }
    return 0;
}


int model_get_setpoint(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.parmac.drying_temperature;
}


int16_t model_get_default_temperature(model_t *model) {
    assert(model != NULL);

    switch (model->tipo_sonda_temperatura) {
        case TEMPERATURE_PROBE_PTC_1:
            return (int16_t)model->run.sensors.temperature_1;

        case TEMPERATURE_PROBE_PTC_2:
            return (int16_t)model->run.sensors.temperature_2;

        case TEMPERATURE_PROBE_SHT:
            return 0;

        default:
            return 0;
    }
}


void model_cycle_active(model_t *pmodel, uint8_t active) {
    assert(pmodel != NULL);
    pmodel->pwoff.active = active;
}


uint16_t model_get_function_flags(model_t *pmodel, uint8_t test) {
    assert(pmodel != NULL);
    return (uint16_t)(((pmodel->run.inizializzato > 0) << 0) | ((test > 0) << 1));
}


void model_add_second(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->pwoff.active_time++;
}


void model_add_work_time_ms(model_t *pmodel, unsigned long ms) {
    assert(pmodel != NULL);
    pmodel->pwoff.work_time += ms / 1000UL;
}


void model_add_rotation_time_ms(model_t *pmodel, unsigned long ms) {
    assert(pmodel != NULL);
    pmodel->pwoff.rotation_time += ms / 1000UL;
}


void model_add_ventilation_time_ms(model_t *pmodel, unsigned long ms) {
    assert(pmodel != NULL);
    pmodel->pwoff.ventilation_time += ms / 1000UL;
}


void model_add_heating_time_ms(model_t *pmodel, unsigned long ms) {
    assert(pmodel != NULL);
    pmodel->pwoff.heating_time += ms / 1000UL;
}


int model_over_safety_temperature(model_t *pmodel) {
    assert(pmodel != NULL);
    if (!pmodel->run.inizializzato) {
        return 0;
    } else {
        return model_get_default_temperature(pmodel) > pmodel->run.parmac.safety_temperature;
    }
}


void model_add_complete_cycle(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->pwoff.complete_cycles++;
}


void model_add_partial_cycle(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->pwoff.partial_cycles++;
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


void model_fan_on(mut_model_t *model) {
    assert(model != NULL);
    if (!model->run.fan.on) {
        model->run.fan.timestamp = timestamp_get();
    }
    model->run.fan.on = 1;
}


void model_fan_off(mut_model_t *model) {
    assert(model != NULL);
    unsigned long res = 0;

    if (model->run.fan.on) {
        res = timestamp_elapsed(model->run.fan.timestamp);
    }

    model_add_ventilation_time_ms(model, res);
}


uint8_t model_is_fan_ok(model_t *model) {
    assert(model != NULL);
    return (model->run.sensors.inputs & 0x01) > 0;
}


uint8_t model_is_heating_ok(model_t *model) {
    assert(model != NULL);
    return model->run.heating.state != HEATING_STATE_OFF || ((model->run.sensors.inputs & 0x02) == 0);
}


void model_set_heating_state(model_t *model, heating_state_t state) {
    assert(model != NULL);

    if (model->run.heating.state != HEATING_STATE_ON && state == HEATING_STATE_ON) {
        model->run.heating.timestamp = timestamp_get();
    }
    model->run.heating.state = state;
}


uint8_t model_get_heating_alarm(model_t *model) {
    return (model->run.heating.state == HEATING_STATE_ON || model->run.heating.state == HEATING_STATE_MIDWAY) &&
           timestamp_is_expired(model->run.heating.timestamp, model->run.parmac.temperature_alarm_delay_seconds * 1000UL);
}
