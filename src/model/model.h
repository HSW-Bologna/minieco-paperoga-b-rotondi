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

#define HEATING_TYPE_ELECTRIC 0
#define HEATING_TYPE_GAS      1

#define TIPO_MACCHINA_OCCUPATA_ALLARMI_CICLO_PAGATO 0
#define TIPO_MACCHINA_OCCUPATA_ALLARMI              1
#define TIPO_MACCHINA_OCCUPATA_CICLO                2


typedef enum {
    DIRECTION_NC = 0,
    DIRECTION_NA = 1,
} direction_t;


typedef enum {
    CYCLE_EVENT_CODE_START,
    CYCLE_EVENT_CODE_COLD_START,
    CYCLE_EVENT_CODE_STOP,
    CYCLE_EVENT_CODE_STEP_DONE,
    CYCLE_EVENT_CODE_PAUSE,
    CYCLE_EVENT_CODE_RESUME,
    CYCLE_EVENT_CODE_MOTION_PAUSE,
    CYCLE_EVENT_CODE_FORWARD,
    CYCLE_EVENT_CODE_BACKWARD,
    CYCLE_EVENT_CODE_CHECK,
    CYCLE_EVENT_CODE_CHECK_TEMPERATURE,
} cycle_event_code_t;


typedef enum {
    RISCALDAMENTO_EVENT_CODE_ON,
    RISCALDAMENTO_EVENT_CODE_OFF,
    RISCALDAMENTO_EVENT_CODE_CHECK,
} heating_event_code_t;


typedef enum {
    BUSY_SIGNAL_TYPE_ALARMS_ACTIVITY = 0,
    BUSY_SIGNAL_TYPE_ALARMS,
    BUSY_SIGNAL_TYPE_ACTIVITY,
} busy_signal_type_t;


typedef enum {
    BURNER_STATE_OK = 0,
    BURNER_STATE_RESETTING,
    BURNER_STATE_DEBOUNCE,
} burner_state_t;


STATE_MACHINE_DECLARE(cycle, cycle_event_code_t);
STATE_MACHINE_DECLARE(heating, heating_event_code_t);


typedef enum {
    DRUM_STATE_STOPPED = 0,
    DRUM_STATE_FORWARD,
    DRUM_STATE_BACKWARD,
} drum_state_t;


typedef enum {
    HEATING_STATE_OFF = 0,
    HEATING_STATE_ON,
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
    TEMPERATURE_PROBE_INPUT = 0,
    TEMPERATURE_PROBE_OUTPUT,
    TEMPERATURE_PROBE_SHT,
} temperature_probe_t;


typedef enum {
    DRYER_PROGRAM_STEP_TYPE_DRYING = 0,
    DRYER_PROGRAM_STEP_TYPE_COOLING,
    DRYER_PROGRAM_STEP_TYPE_ANTIFOLD,
#define DRYER_PROGRAM_STEP_TYPE_NUM 3,
} dryer_program_step_type_t;


typedef enum {
    ALARM_CODE_OBLO_APERTO = 0,
    ALARM_CODE_EMERGENZA,
    ALARM_CODE_FILTRO,
    ALARM_CODE_AIR_FLOW,
    ALARM_CODE_BURNER,
    ALARM_CODE_SAFETY_TEMPERATURE,
    ALARM_CODE_TEMPERATURE_NOT_REACHED,
} alarm_code_t;


typedef enum {
    INPUT_SAFETY_THERMOSTAT = 0,
    INPUT_PORTHOLE,
    INPUT_EMERGENCY,
    INPUT_AIR_FLOW,
    INPUT_INVERTER_ALARM,
    INPUT_PAYMENT,
    INPUT_FILTER_FEEDBACK,
    INPUT_BURNER_ALARM,
} input_t;


typedef enum {
    OUTPUT_FORWARD = 0,
    OUTPUT_BACKWARD,
    OUTPUT_FAN,
    OUTPUT_HEATING,
    OUTPUT_RESET_GAS,
    OUTPUT_BUSY,
    OUTPUT_AUX,
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
} statistics_t;


typedef struct {
    statistics_t statisics;

    // Parametri step
    uint16_t flag_asciugatura;

    struct {
        uint8_t                   coin_reader_enabled;
        uint16_t                  alarms;
        timestamp_t               porthole_opened_ts;
        timestamp_t               air_flow_stopped_ts;
        timestamp_t               burner_ts;
        uint16_t                  burner_reset_attempts;
        burner_state_t            burner_state;
        uint8_t                   burner_alarm;
        uint8_t                   temperature_not_reached_alarm;
        uint16_t                  program_number;
        uint16_t                  step_number;
        dryer_program_step_type_t step_type;

        struct {
            uint8_t  on;
            uint16_t outputs;
            uint8_t  pwm1;
            uint8_t  pwm2;
        } test;

        struct {
            uint16_t temperature_input_adc;
            int16_t  temperature_input;
            uint16_t temperature_output_adc;
            int16_t  temperature_output;

            int16_t  temperature_probe;
            uint16_t humidity_probe;

            uint16_t inputs;

            uint16_t coins[COIN_LINES];
        } sensors;

        struct {
            // Parametri macchina
            busy_signal_type_t busy_signal_type;
            int16_t            safety_temperature;
            uint16_t           temperature_alarm_delay_seconds;
            uint16_t           air_flow_alarm_time;
            uint16_t           temperature_probe;
            uint8_t            disable_alarms;
            uint16_t           enable_inverter_alarm;
            uint16_t           enable_filter_alarm;
            uint8_t            stop_time_in_pause;
            uint16_t           cycle_delay_time;
            uint16_t           duration;
            uint16_t           max_cycles;
            uint16_t           start_delay;
            uint16_t           rotation_running_time;
            uint16_t           rotation_pause_time;
            uint16_t           speed;
            uint16_t           setpoint_temperature;
            uint16_t           setpoint_humidity;
            uint16_t           temperature_heating_hysteresis;
            uint16_t           temperature_cooling_hysteresis;
            uint16_t           progressive_heating_time;
            uint16_t           drying_type;
            uint16_t           heating_type;
            uint16_t           gas_ignition_attempts;
            uint16_t           fan_with_open_porthole_time;
            uint8_t            porthole_nc_na;
            uint8_t            emergency_alarm_nc_na;
            uint8_t            inverter_alarm_nc_na;
            uint8_t            filter_alarm_nc_na;
            uint8_t            busy_signal_nc_na;
            uint8_t            wait_for_temperature;
            uint8_t            enable_reverse;
            uint8_t            invert_fan_drum;
        } parmac;

        struct {
            timestamp_t             timestamp;
            heating_state_machine_t state_machine;
        } heating;

        struct {
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
            stopwatch_timer_t     timer_reset;
            stopwatch_timer_t     timer_temperature;
            cycle_state_machine_t state_machine;
            uint16_t              num_cycles;
        } cycle;
    } run;
} model_t;


typedef model_t mut_model_t;


void     model_init(model_t *model);
size_t   model_pwoff_serialize(model_t *model, uint8_t *buff);
int      model_pwoff_deserialize(model_t *model, uint8_t *buff);
int      model_set_warning(model_t *model, warning_code_t warning);
int      model_heating_enabled(model_t *model);
int      model_ciclo_continuo(model_t *model);
int      model_ciclo_fermo(model_t *model);
int16_t  model_get_default_temperature(model_t *model);
int      model_is_step_antifold(model_t *model);
void     model_clear_coins(model_t *model);
uint16_t model_get_function_flags(model_t *model, uint8_t test);
uint8_t  model_is_porthole_open(model_t *model);
int      model_get_setpoint(model_t *model);

void          model_vaporizzazione_attivata(model_t *model);
int           model_vaporizzazione_da_attivare(model_t *model);
void          model_comincia_step(model_t *model);
void          model_add_second(model_t *model);
void          model_add_work_time_ms(model_t *model, unsigned long ms);
void          model_add_rotation_time_ms(model_t *model, unsigned long ms);
void          model_add_ventilation_time_ms(model_t *model, unsigned long ms);
void          model_add_heating_time_ms(model_t *model, unsigned long ms);
int           model_over_safety_temperature(model_t *model);
void          model_add_complete_cycle(model_t *model);
void          model_add_partial_cycle(model_t *model);
uint16_t      model_get_relay_map(model_t *model);
void          model_update_sensors(mut_model_t *model, uint16_t inputs, uint16_t temperature_input_adc,
                                   uint16_t temperature_output_adc, int16_t temperature_probe, uint16_t humidity_probe);
uint8_t       model_get_pwm_fan_percentage(model_t *model);
uint8_t       model_get_pwm_drum_percentage(model_t *model);
unsigned long model_get_cycle_remaining_time(mut_model_t *model);
uint8_t       model_is_drum_running_forward(model_t *model);
void          model_drum_stop(mut_model_t *model);
void          model_drum_forward(mut_model_t *model);
void          model_drum_backward(mut_model_t *model);
uint8_t       model_get_heating_alarm(model_t *model);
uint32_t      model_get_step_duration_seconds(model_t *model);
void          model_cycle_resume(mut_model_t *model);
void          model_cycle_pause(mut_model_t *model);
void          model_cycle_stop(mut_model_t *model);
void          model_manage(mut_model_t *model);
uint16_t      model_get_remaining_seconds(model_t *model);
uint8_t       model_is_emergency_alarm_active(model_t *model);
uint16_t      model_get_active_alarms(model_t *model);
void          model_fix_alarms(model_t *model);
void          model_clear_alarms(model_t *model);
uint8_t       model_is_cycle_active(model_t *model);
uint8_t       model_is_cycle_on(model_t *model);
void          model_reset_burner(model_t *model);
uint8_t       model_is_fan_on(model_t *model);
uint8_t       model_is_step_endless(model_t *model);
uint8_t       model_cycles_exceeded(model_t *model);
uint8_t       model_is_inverter_alarm_active(model_t *model);


#endif
