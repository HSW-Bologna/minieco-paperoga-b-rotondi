#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>
#include "stopwatch.h"
#include "stopwatch_timer.h"
#include "services/timestamp.h"
#include "state_machine.h"


#define PWOFF_SERIALIZED_SIZE 45
#define COIN_LINES            6

#define TIPO_RISCALDAMENTO_ELETTRICO 0
#define TIPO_RISCALDAMENTO_GAS       1

#define TIPO_MACCHINA_OCCUPATA_ALLARMI_CICLO_PAGATO 0
#define TIPO_MACCHINA_OCCUPATA_ALLARMI              1
#define TIPO_MACCHINA_OCCUPATA_CICLO                2


typedef enum {
    CYCLE_EVENT_CODE_START,
    CYCLE_EVENT_CODE_STOP,
    CYCLE_EVENT_CODE_STEP_DONE,
    CYCLE_EVENT_CODE_PAUSE,
    CYCLE_EVENT_CODE_RESUME,
    CYCLE_EVENT_CODE_MOTION_PAUSE,
    CYCLE_EVENT_CODE_FORWARD,
    CYCLE_EVENT_CODE_BACKWARD,
    CYCLE_EVENT_CODE_ALARM,
} cycle_event_code_t;


typedef enum {
    BUSY_SIGNAL_TYPE_ALARMS_ACTIVITY_PAYMENT = 0,
    BUSY_SIGNAL_TYPE_ALARMS,
    BUSY_SIGNAL_TYPE_ACTIVITY,
    BUSY_SIGNAL_TYPE_ALARMS_ACTIVITY_PAYMENT_INVERTED,
    BUSY_SIGNAL_TYPE_ALARMS_INVERTED,
    BUSY_SIGNAL_TYPE_ACTIVITY_INVERTED,
} busy_signal_type_t;


STATE_MACHINE_DECLARE(cycle, cycle_event_code_t);


typedef enum {
    DRUM_STATE_STOPPED = 0,
    DRUM_STATE_FORWARD,
    DRUM_STATE_BACKWARD,
} drum_state_t;


typedef enum {
    HEATING_STATE_OFF = 0,
    HEATING_STATE_ON,
    HEATING_STATE_MIDWAY,
    HEATING_STATE_SETPOINT_REACHED,
} heating_state_t;


typedef enum {
    CYCLE_STATE_STOPPED = 0,
    CYCLE_STATE_ACTIVE,
    CYCLE_STATE_WAIT_START,
    CYCLE_STATE_RUNNING,
    CYCLE_STATE_PAUSED,
} cycle_state_t;


typedef enum {
    TEMPERATURE_PROBE_PTC_1 = 0,
    TEMPERATURE_PROBE_PTC_2,
    TEMPERATURE_PROBE_SHT,
} temperature_probe_t;


typedef enum {
    DRYER_PROGRAM_STEP_TYPE_DRYING = 0,
    DRYER_PROGRAM_STEP_TYPE_COOLING,
    DRYER_PROGRAM_STEP_TYPE_UNFOLDING,
#define DRYER_PROGRAM_STEP_TYPE_NUM 3,
} dryer_program_step_type_t;


typedef enum {
    ALARM_CODE_EMERGENZA = 0,
    ALARM_CODE_TEMPERATURA,
    ALARM_CODE_RISCALDAMENTO,
    ALARM_CODE_OBLO_APERTO,
} alarm_code_t;


typedef enum {
    INPUT_1 = 0,
    INPUT_FILTER_FEEDBACK,
    INPUT_3,
    INPUT_4,
    INPUT_5,
    INPUT_EMERGENCY,
    INPUT_PORTHOLE,
} input_t;


typedef enum {
    OUTPUT_1 = 0,
    OUTPUT_2,
    OUTPUT_3,
    OUTPUT_4,
    OUTPUT_BACKWARD,
    OUTPUT_FORWARD,
} output_t;


typedef enum {
    WARNING_CODE_VENTILAZIONE = 0,
    WARNING_CODE_RISCALDAMENTO,
} warning_code_t;


typedef struct {
    uint16_t complete_cycles;
    uint16_t partial_cycles;
    uint32_t active_time;
    uint32_t work_time;
    uint32_t rotation_time;
    uint32_t ventilation_time;
    uint32_t heating_time;

    uint16_t coins[COIN_LINES];
    uint16_t remaining_time;
    uint16_t program_number;
    uint16_t step_number;
    uint8_t  active;
} pwoff_data_t;


typedef struct {
    pwoff_data_t pwoff;


    // Parametri step
    uint16_t flag_asciugatura;
    uint16_t tipo_sonda_temperatura;

    uint16_t isteresi_temperatura_on_res2;
    uint16_t isteresi_temperatura_off_res1;

    struct {
        uint8_t inizializzato;

        struct {
            uint8_t  on;
            uint16_t outputs;
            uint8_t  pwm1;
            uint8_t  pwm2;
        } test;

        struct {
            uint16_t temperature_1_adc;
            int16_t  temperature_1;
            uint16_t temperature_2_adc;
            int16_t  temperature_2;

            uint16_t inputs;
        } sensors;

        dryer_program_step_type_t step_type;

        struct {
            // Parametri macchina
            busy_signal_type_t busy_signal_type;
            int16_t            safety_temperature;
            uint16_t           temperature_alarm_delay_seconds;
            uint8_t            disable_alarms;
            uint16_t           enable_inverter_alarm;
            uint16_t           enable_filter_alarm;
            uint8_t            stop_time_in_pause;
            uint16_t           cycle_delay_time;
            uint16_t           rotation_running_time;
            uint16_t           rotation_pause_time;
            uint16_t           drying_duration;
            uint16_t           speed;
            uint16_t           drying_type;
            uint16_t           drying_temperature;
            uint16_t           drying_humidity;
        } parmac;

        struct {
            heating_state_t state;
            timestamp_t     timestamp;
        } heating;

        struct {
            uint8_t     on;
            timestamp_t timestamp;
        } fan;

        struct {
            drum_state_t state;
            timestamp_t  timestamp;
            uint8_t      speed;
        } drum;

        struct {
            stopwatch_timer_t     timer_cycle;
            stopwatch_timer_t     timer_rotation;
            cycle_state_machine_t state_machine;
        } cycle;
    } run;
} model_t;


typedef model_t mut_model_t;


void     model_init(model_t *pmodel);
size_t   model_pwoff_serialize(model_t *pmodel, uint8_t *buff);
int      model_pwoff_deserialize(model_t *pmodel, uint8_t *buff);
int      model_set_warning(model_t *pmodel, warning_code_t warning);
int      model_heating_enabled(model_t *pmodel);
int      model_ciclo_continuo(model_t *pmodel);
int      model_ciclo_fermo(model_t *pmodel);
int16_t  model_get_default_temperature(model_t *model);
int      model_is_step_unfolding(model_t *pmodel);
void     model_clear_coins(model_t *pmodel);
uint16_t model_get_function_flags(model_t *pmodel, uint8_t test);
void     model_cycle_active(model_t *pmodel, uint8_t active);
uint8_t  model_is_porthole_open(model_t *pmodel);
int      model_get_setpoint(model_t *pmodel);

void          model_vaporizzazione_attivata(model_t *pmodel);
int           model_vaporizzazione_da_attivare(model_t *pmodel);
void          model_comincia_step(model_t *pmodel);
void          model_add_second(model_t *pmodel);
void          model_add_work_time_ms(model_t *pmodel, unsigned long ms);
void          model_add_rotation_time_ms(model_t *pmodel, unsigned long ms);
void          model_add_ventilation_time_ms(model_t *pmodel, unsigned long ms);
void          model_add_heating_time_ms(model_t *pmodel, unsigned long ms);
int           model_over_safety_temperature(model_t *pmodel);
void          model_add_complete_cycle(model_t *pmodel);
void          model_add_partial_cycle(model_t *pmodel);
uint16_t      model_get_relay_map(model_t *model);
void          model_update_temperature(mut_model_t *model, temperature_probe_t temperature_probe, uint16_t adc);
uint8_t       model_get_pwm1_percentage(model_t *model);
uint8_t       model_get_pwm2_percentage(model_t *model);
unsigned long model_get_cycle_remaining_time(mut_model_t *model);
void          model_fan_on(mut_model_t *model);
uint8_t       model_is_drum_running_forward(model_t *model);
void          model_drum_stop(mut_model_t *model);
void          model_drum_forward(mut_model_t *model);
void          model_drum_backward(mut_model_t *model);
void          model_fan_off(mut_model_t *model);
uint8_t       model_get_heating_alarm(model_t *model);
void          model_set_heating_state(model_t *model, heating_state_t state);
uint32_t      model_get_step_duration_seconds(model_t *model);
void          model_cycle_resume(mut_model_t *model);
void          model_cycle_pause(mut_model_t *model);
void          model_cycle_stop(mut_model_t *model);
void          model_manage(mut_model_t *model);
uint16_t      model_get_remaining_seconds(model_t *model);
uint8_t       model_is_emergency_alarm_active(model_t *model);
uint16_t      model_get_alarms(model_t *model);


#endif
