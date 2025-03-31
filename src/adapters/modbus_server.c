#define LIGHTMODBUS_IMPL
#define LIGHTMODBUS_SLAVE
#define LIGHTMODBUS_F03S
#define LIGHTMODBUS_F04S
#define LIGHTMODBUS_F16S
#include <lightmodbus/lightmodbus.h>
#include <string.h>
#include "services/timestamp.h"
#include "config/app_config.h"
#include "model/cycle.h"
#include "modbus_server.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "bsp/rs232.h"


#define COMPUTE_BUILD_YEAR                                                                                             \
    ((__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0'))


#define COMPUTE_BUILD_DAY (((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0'))


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define COMPUTE_BUILD_MONTH                                                                                            \
    ((BUILD_MONTH_IS_JAN)   ? 1                                                                                        \
     : (BUILD_MONTH_IS_FEB) ? 2                                                                                        \
     : (BUILD_MONTH_IS_MAR) ? 3                                                                                        \
     : (BUILD_MONTH_IS_APR) ? 4                                                                                        \
     : (BUILD_MONTH_IS_MAY) ? 5                                                                                        \
     : (BUILD_MONTH_IS_JUN) ? 6                                                                                        \
     : (BUILD_MONTH_IS_JUL) ? 7                                                                                        \
     : (BUILD_MONTH_IS_AUG) ? 8                                                                                        \
     : (BUILD_MONTH_IS_SEP) ? 9                                                                                        \
     : (BUILD_MONTH_IS_OCT) ? 10                                                                                       \
     : (BUILD_MONTH_IS_NOV) ? 11                                                                                       \
     : (BUILD_MONTH_IS_DEC) ? 12                                                                                       \
                            : /* error default */ 99)


#define COMMAND_REGISTER_RESUME                 1
#define COMMAND_REGISTER_PAUSE                  2
#define COMMAND_REGISTER_STANDBY                3
#define COMMAND_REGISTER_DONE                   4
#define COMMAND_REGISTER_CLEAR_ALARMS           5
#define COMMAND_REGISTER_CLEAR_COINS            6
#define COMMAND_REGISTER_CLEAR_CYCLE_STATISTICS 7


enum {
    MODBUS_IR_DEVICE_MODEL = 0,
    MODBUS_IR_FIRMWARE_VERSION,
    MODBUS_IR_BUILD_DAY_MONTH,
    MODBUS_IR_BUILD_YEAR,
    MODBUS_IR_STATUS,
    MODBUS_IR_INPUTS,
    MODBUS_IR_OUTPUTS,
    MODBUS_IR_DRUM_SPEED,
    MODBUS_IR_FAN_SPEED,
    MODBUS_IR_TEMPERATURE_1_ADC,
    MODBUS_IR_TEMPERATURE_1,
    MODBUS_IR_TEMPERATURE_2_ADC,
    MODBUS_IR_TEMPERATURE_2,
    MODBUS_IR_TEMPERATURE_PROBE,
    MODBUS_IR_HUMIDITY_PROBE,
    MODBUS_IR_PRESSURE_ADC,
    MODBUS_IR_PRESSURE,
    MODBUS_IR_PAYMENT,
    MODBUS_IR_COIN_LINES_1,
    MODBUS_IR_COIN_LINES_2,
    MODBUS_IR_COIN_LINES_3,
    MODBUS_IR_COIN_LINES_4,
    MODBUS_IR_COIN_LINES_5,
    MODBUS_IR_CYCLE_STATE,
    MODBUS_IR_DEFAULT_TEMPERATURE,
    MODBUS_IR_ELAPSED_TIME,
    MODBUS_IR_REMAINING_TIME,
    MODBUS_IR_ALARMS,
    MODBUS_IR_STATS_COMPLETE_CYCLES_HIGH,
    MODBUS_IR_STATS_COMPLETE_CYCLES_LOW,
    MODBUS_IR_STATS_PARTIAL_CYCLES_HIGH,
    MODBUS_IR_STATS_PARTIAL_CYCLES_LOW,
    MODBUS_IR_STATS_ACTIVE_TIME_SECONDS_HIGH,
    MODBUS_IR_STATS_ACTIVE_TIME_SECONDS_LOW,
    MODBUS_IR_STATS_WORK_TIME_SECONDS_HIGH,
    MODBUS_IR_STATS_WORK_TIME_SECONDS_LOW,
    MODBUS_IR_STATS_ROTATION_TIME_SECONDS_HIGH,
    MODBUS_IR_STATS_ROTATION_TIME_SECONDS_LOW,
    MODBUS_IR_STATS_VENTILATION_TIME_SECONDS_HIGH,
    MODBUS_IR_STATS_VENTILATION_TIME_SECONDS_LOW,
    MODBUS_IR_STATS_HEATING_TIME_SECONDS_HIGH,
    MODBUS_IR_STATS_HEATING_TIME_SECONDS_LOW,
};


enum {
    MODBUS_HR_TEST_MODE = 0,
    MODBUS_HR_TEST_OUTPUTS,
    MODBUS_HR_TEST_PWM,
    MODBUS_HR_COIN_READER_INHIBITION,
    MODBUS_HR_BUSY_SIGNAL_TYPE,
    MODBUS_HR_SAFETY_TEMPERATURE,
    MODBUS_HR_TEMPERATURE_ALARM_DELAY_SECONDS,
    MODBUS_HR_AIR_FLOW_ALARM_TIME,
    MODBUS_HR_TEMPERATURE_PROBE,
    MODBUS_HR_HEATING_TYPE,
    MODBUS_HR_GAS_IGNITION_ATTEMPTS,
    MODBUS_HR_FAN_WITH_OPEN_PORTHOLE_TIME,
    MODBUS_HR_CYCLE_DELAY_TIME,
    MODBUS_HR_CYCLE_RESET_TIME,
    MODBUS_HR_FLAGS,
    MODBUS_HR_DURATION,
    MODBUS_HR_ROTATION_RUNNING_TIME,
    MODBUS_HR_ROTATION_PAUSE_TIME,
    MODBUS_HR_ROTATION_SPEED,
    MODBUS_HR_SETPOINT_TEMPERATURE,
    MODBUS_HR_SETPOINT_HUMIDITY,
    MODBUS_HR_TEMPERATURE_COOLING_HYSTERESIS,
    MODBUS_HR_TEMPERATURE_HEATING_HYSTERESIS,
    MODBUS_HR_DRYING_TYPE,
    MODBUS_HR_PROGRAM_NUMBER,
    MODBUS_HR_STEP_NUMBER,
    MODBUS_HR_STEP_TYPE,
    MODBUS_HR_COMMAND,
    MODBUS_HR_INCREASE_DURATION,
    MODBUS_HR_SET_DURATION,
};


static ModbusError register_callback(const ModbusSlave *minion, const ModbusRegisterCallbackArgs *args,
                                     ModbusRegisterCallbackResult *result);
static ModbusError static_allocator(ModbusBuffer *buffer, uint16_t size, void *context);
static ModbusError exception_callback(const ModbusSlave *minion, uint8_t function, ModbusExceptionCode code);

static ModbusSlave modbus_minion;

void modbus_server_init(void) {
    // Read the data from stdin
    ModbusErrorInfo err = modbusSlaveInit(&modbus_minion, register_callback, exception_callback, static_allocator,
                                          modbusSlaveDefaultFunctions, modbusSlaveDefaultFunctionCount);

    // Check for errors
    assert(modbusIsOk(err) && "modbusSlaveInit() failed!");
}

void modbus_server_manage(mut_model_t *model) {
    uint8_t request[BSP_RS232_MAX_PACKET_LEN] = {0};
    size_t  request_length                    = bsp_rs232_read(request, sizeof(request));

    if (request_length > 0 && bsp_rs232_timed_out(10)) {
        modbusSlaveSetUserPointer(&modbus_minion, model);
        ModbusErrorInfo err = modbusParseRequestRTU(&modbus_minion, 1, request, (uint8_t)request_length);

        if (modbusIsOk(err)) {
            const uint8_t *response        = modbusSlaveGetResponse(&modbus_minion);
            uint16_t       response_length = modbusSlaveGetResponseLength(&modbus_minion);
            bsp_rs232_write((uint8_t *)response, response_length);
            bsp_rs232_flush();
            model->run.communication_ts = timestamp_get();
        } else {
            bsp_rs232_flush();
        }
    } else {
        // No data
    }
}

static ModbusError static_allocator(ModbusBuffer *buffer, uint16_t size, void *context) {
    (void)context;
#define MAX_RESPONSE 256

    static uint8_t response[MAX_RESPONSE];

    if (size != 0)     // Allocation reqest
    {
        if (size <= MAX_RESPONSE)     // Allocation request is within bounds
        {
            buffer->data = response;
            return MODBUS_OK;
        } else     // Allocation error
        {
            buffer->data = NULL;
            return MODBUS_ERROR_ALLOC;
        }
    } else     // Free request
    {
        buffer->data = NULL;
        return MODBUS_OK;
    }
}

static ModbusError register_callback(const ModbusSlave *minion, const ModbusRegisterCallbackArgs *args,
                                     ModbusRegisterCallbackResult *result) {
    mut_model_t *model = modbusSlaveGetUserPointer(minion);

    switch (args->query) {
        // Pretend to allow all access
        case MODBUS_REGQ_R_CHECK:
            switch (args->type) {
                case MODBUS_INPUT_REGISTER:
                case MODBUS_HOLDING_REGISTER:
                    result->exceptionCode = MODBUS_EXCEP_NONE;
                    break;

                default:
                    result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION;
                    break;
            }
            break;

        case MODBUS_REGQ_W_CHECK:
            switch (args->type) {
                case MODBUS_HOLDING_REGISTER:
                    switch (args->index) {
                        case MODBUS_HR_COMMAND:
                        case MODBUS_HR_INCREASE_DURATION:
                        case MODBUS_HR_SET_DURATION:
                        case MODBUS_HR_TEST_MODE:
                        case MODBUS_HR_TEST_OUTPUTS:
                        case MODBUS_HR_TEST_PWM:
                        case MODBUS_HR_COIN_READER_INHIBITION:
                        case MODBUS_HR_BUSY_SIGNAL_TYPE:
                        case MODBUS_HR_SAFETY_TEMPERATURE:
                        case MODBUS_HR_TEMPERATURE_ALARM_DELAY_SECONDS:
                        case MODBUS_HR_AIR_FLOW_ALARM_TIME:
                        case MODBUS_HR_TEMPERATURE_PROBE:
                        case MODBUS_HR_HEATING_TYPE:
                        case MODBUS_HR_GAS_IGNITION_ATTEMPTS:
                        case MODBUS_HR_FAN_WITH_OPEN_PORTHOLE_TIME:
                        case MODBUS_HR_CYCLE_DELAY_TIME:
                        case MODBUS_HR_CYCLE_RESET_TIME:
                        case MODBUS_HR_FLAGS:
                        case MODBUS_HR_ROTATION_RUNNING_TIME:
                        case MODBUS_HR_ROTATION_PAUSE_TIME:
                        case MODBUS_HR_ROTATION_SPEED:
                        case MODBUS_HR_DURATION:
                        case MODBUS_HR_DRYING_TYPE:
                        case MODBUS_HR_SETPOINT_TEMPERATURE:
                        case MODBUS_HR_SETPOINT_HUMIDITY:
                        case MODBUS_HR_TEMPERATURE_COOLING_HYSTERESIS:
                        case MODBUS_HR_TEMPERATURE_HEATING_HYSTERESIS:
                        case MODBUS_HR_PROGRAM_NUMBER:
                        case MODBUS_HR_STEP_NUMBER:
                        case MODBUS_HR_STEP_TYPE:
                            result->exceptionCode = MODBUS_EXCEP_NONE;
                            break;
                        default:
                            result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
                            break;
                    }
                    break;

                default:
                    result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION;
                    break;
            }
            break;

            // Return 7 when reading
        case MODBUS_REGQ_R: {
            switch (args->type) {
                case MODBUS_INPUT_REGISTER: {
                    switch (args->index) {
                        case MODBUS_IR_DEVICE_MODEL:
                            result->value = APP_CONFIG_DEVICE_MODEL;
                            break;

                        case MODBUS_IR_FIRMWARE_VERSION:
                            result->value = ((APP_CONFIG_FIRMWARE_VERSION_MAJOR & 0x1F) << 11) |
                                            ((APP_CONFIG_FIRMWARE_VERSION_MINOR & 0x1F) << 6) |
                                            (APP_CONFIG_FIRMWARE_VERSION_PATCH & 0x3F);
                            break;

                        case MODBUS_IR_BUILD_DAY_MONTH:
                            result->value = (COMPUTE_BUILD_DAY << 8) | COMPUTE_BUILD_MONTH;
                            break;

                        case MODBUS_IR_BUILD_YEAR:
                            result->value = COMPUTE_BUILD_YEAR;
                            break;

                        case MODBUS_IR_STATUS:
                            if (model_is_cycle_active(model)) {
                                result->value =
                                    (uint16_t)((((model_get_relay_map(model) & (1 << OUTPUT_HEATING)) > 0) << 0) |
                                               ((model_is_time_held_by_temperature(model) > 0) << 1) |
                                               ((model_is_time_held_by_humidity(model) > 0) << 2));
                            } else {
                                result->value =
                                    (uint16_t)(((model_get_relay_map(model) & (1 << OUTPUT_HEATING)) > 0) << 0);
                            }
                            break;

                        case MODBUS_IR_INPUTS:
                            result->value = model->run.sensors.inputs;
                            break;

                        case MODBUS_IR_OUTPUTS:
                            result->value = model_get_relay_map(model);
                            break;

                        case MODBUS_IR_FAN_SPEED:
                            result->value = model_get_pwm_fan_percentage(model);
                            break;

                        case MODBUS_IR_DRUM_SPEED:
                            result->value = model_get_pwm_drum_percentage(model);
                            break;

                        case MODBUS_IR_TEMPERATURE_1_ADC:
                            result->value = model->run.sensors.temperature_input_adc;
                            break;

                        case MODBUS_IR_TEMPERATURE_1:
                            result->value = (uint16_t)model->run.sensors.temperature_input;
                            break;

                        case MODBUS_IR_TEMPERATURE_2_ADC:
                            result->value = model->run.sensors.temperature_output_adc;
                            break;

                        case MODBUS_IR_TEMPERATURE_2:
                            result->value = (uint16_t)model->run.sensors.temperature_output;
                            break;

                        case MODBUS_IR_TEMPERATURE_PROBE:
                            result->value = (uint16_t)model->run.sensors.temperature_probe;
                            break;

                        case MODBUS_IR_HUMIDITY_PROBE:
                            result->value = (uint16_t)model->run.sensors.humidity_probe;
                            break;

                        case MODBUS_IR_PRESSURE_ADC:
                            result->value = 0;
                            break;

                        case MODBUS_IR_PRESSURE:
                            result->value = 0;
                            break;

                        case MODBUS_IR_PAYMENT:
                        case MODBUS_IR_COIN_LINES_1:
                        case MODBUS_IR_COIN_LINES_2:
                        case MODBUS_IR_COIN_LINES_3:
                        case MODBUS_IR_COIN_LINES_4:
                        case MODBUS_IR_COIN_LINES_5:
                            result->value = model->run.sensors.coins[args->index - MODBUS_IR_PAYMENT];
                            break;

                        case MODBUS_IR_CYCLE_STATE:
                            result->value = (uint16_t)model->run.cycle.state_machine.node_index;
                            break;

                        case MODBUS_IR_DEFAULT_TEMPERATURE:
                            result->value = (uint16_t)model_get_default_temperature(model);
                            break;

                        case MODBUS_IR_ELAPSED_TIME:
                            result->value = model_get_elapsed_seconds(model);
                            break;

                        case MODBUS_IR_REMAINING_TIME:
                            result->value = model_get_remaining_seconds(model);
                            break;

                        case MODBUS_IR_ALARMS:
                            result->value = model->run.alarms;
                            break;

                        case MODBUS_IR_STATS_COMPLETE_CYCLES_HIGH:
                            result->value = (uint16_t)((model->statistics.complete_cycles >> 16) & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_COMPLETE_CYCLES_LOW:
                            result->value = (uint16_t)(model->statistics.complete_cycles & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_PARTIAL_CYCLES_HIGH:
                            result->value = (uint16_t)((model->statistics.partial_cycles >> 16) & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_PARTIAL_CYCLES_LOW:
                            result->value = (uint16_t)(model->statistics.partial_cycles & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_ACTIVE_TIME_SECONDS_HIGH: {
                            uint32_t current_active_time = model->statistics.active_time + timestamp_get() / 1000UL;
                            result->value                = (uint16_t)((current_active_time >> 16) & 0xFFFF);
                            break;
                        }

                        case MODBUS_IR_STATS_ACTIVE_TIME_SECONDS_LOW: {
                            uint32_t current_active_time = model->statistics.active_time + timestamp_get() / 1000UL;
                            result->value                = (uint16_t)(current_active_time & 0xFFFF);
                            break;
                        }

                        case MODBUS_IR_STATS_WORK_TIME_SECONDS_HIGH:
                            result->value = (uint16_t)((model->statistics.work_time >> 16) & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_WORK_TIME_SECONDS_LOW:
                            result->value = (uint16_t)(model->statistics.work_time & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_ROTATION_TIME_SECONDS_HIGH:
                            result->value = (uint16_t)((model->statistics.rotation_time >> 16) & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_ROTATION_TIME_SECONDS_LOW:
                            result->value = (uint16_t)(model->statistics.rotation_time & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_VENTILATION_TIME_SECONDS_HIGH:
                            result->value = (uint16_t)((model->statistics.ventilation_time >> 16) & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_VENTILATION_TIME_SECONDS_LOW:
                            result->value = (uint16_t)(model->statistics.ventilation_time & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_HEATING_TIME_SECONDS_HIGH:
                            result->value = (uint16_t)((model->statistics.heating_time >> 16) & 0xFFFF);
                            break;

                        case MODBUS_IR_STATS_HEATING_TIME_SECONDS_LOW:
                            result->value = (uint16_t)(model->statistics.heating_time & 0xFFFF);
                            break;

                        default:
                            result->value = 0;
                            break;
                    }
                    break;
                }

                case MODBUS_HOLDING_REGISTER: {
                    switch (args->index) {
                        case MODBUS_HR_TEST_MODE:
                            result->value = model->run.test.on;
                            break;

                        case MODBUS_HR_TEST_OUTPUTS:
                            result->value = model->run.test.outputs;
                            break;

                        case MODBUS_HR_TEST_PWM:
                            result->value = (uint16_t)model->run.test.pwm1 | (uint16_t)(model->run.test.pwm2 << 8);
                            break;

                        case MODBUS_HR_COIN_READER_INHIBITION:
                            result->value = model->run.coin_reader_enabled;
                            break;

                        case MODBUS_HR_BUSY_SIGNAL_TYPE:
                            result->value = (uint16_t)model->run.parmac.busy_signal_type;
                            break;

                        case MODBUS_HR_SAFETY_TEMPERATURE:
                            result->value = (uint16_t)model->run.parmac.safety_temperature;
                            break;

                        case MODBUS_HR_TEMPERATURE_ALARM_DELAY_SECONDS:
                            result->value = (uint16_t)model->run.parmac.temperature_alarm_delay_seconds;
                            break;

                        case MODBUS_HR_AIR_FLOW_ALARM_TIME:
                            result->value = (uint16_t)model->run.parmac.air_flow_alarm_time;
                            break;

                        case MODBUS_HR_TEMPERATURE_PROBE:
                            result->value = (uint16_t)model->run.parmac.temperature_probe;
                            break;

                        case MODBUS_HR_HEATING_TYPE:
                            result->value = (uint16_t)model->run.parmac.heating_type;
                            break;

                        case MODBUS_HR_GAS_IGNITION_ATTEMPTS:
                            result->value = (uint16_t)model->run.parmac.gas_ignition_attempts;
                            break;

                        case MODBUS_HR_FAN_WITH_OPEN_PORTHOLE_TIME:
                            result->value = (uint16_t)model->run.parmac.fan_with_open_porthole_time;
                            break;

                        case MODBUS_HR_CYCLE_DELAY_TIME:
                            result->value = model->run.parmac.cycle_delay_time;
                            break;

                        case MODBUS_HR_CYCLE_RESET_TIME:
                            result->value = model->run.parmac.cycle_reset_time;
                            break;

                        case MODBUS_HR_FLAGS:
                            result->value = (uint16_t)((model->run.parmac.stop_time_in_pause > 0) << 0 |
                                                       (model->run.parmac.disable_alarms > 0) << 1 |
                                                       (model->run.parmac.emergency_alarm_nc_na > 0) << 2 |
                                                       (model->run.parmac.inverter_alarm_nc_na > 0) << 3 |
                                                       (model->run.parmac.filter_alarm_nc_na > 0) << 4 |
                                                       (model->run.parmac.porthole_nc_na > 0) << 5 |
                                                       (model->run.parmac.busy_signal_nc_na > 0) << 6 |
                                                       (model->run.parmac.enable_reverse > 0) << 7 |
                                                       (model->run.parmac.wait_for_temperature > 0) << 8     //|
                            );
                            break;

                        case MODBUS_HR_DURATION:
                            result->value = model->run.parmac.duration;
                            break;

                        case MODBUS_HR_DRYING_TYPE:
                            result->value = model->run.parmac.wait_for_humidity;
                            break;

                        case MODBUS_HR_SETPOINT_TEMPERATURE:
                            result->value = model->run.parmac.setpoint_temperature;
                            break;

                        case MODBUS_HR_SETPOINT_HUMIDITY:
                            result->value = model->run.parmac.setpoint_humidity;
                            break;

                        case MODBUS_HR_TEMPERATURE_COOLING_HYSTERESIS:
                            result->value = model->run.parmac.temperature_cooling_hysteresis;
                            break;

                        case MODBUS_HR_TEMPERATURE_HEATING_HYSTERESIS:
                            result->value = model->run.parmac.temperature_heating_hysteresis;
                            break;

                        case MODBUS_HR_PROGRAM_NUMBER:
                            result->value = model->run.program_number;
                            break;

                        case MODBUS_HR_STEP_NUMBER:
                            result->value = model->run.step_number;
                            break;

                        case MODBUS_HR_ROTATION_RUNNING_TIME:
                            result->value = model->run.parmac.rotation_running_time;
                            break;

                        case MODBUS_HR_ROTATION_PAUSE_TIME:
                            result->value = model->run.parmac.rotation_pause_time;
                            break;

                        case MODBUS_HR_ROTATION_SPEED:
                            result->value = model->run.parmac.speed;
                            break;

                        case MODBUS_HR_STEP_TYPE:
                            result->value = (uint16_t)model->run.step_type;
                            break;

                        default:
                            result->value = 0;
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case MODBUS_REGQ_W: {
            switch (args->type) {
                case MODBUS_HOLDING_REGISTER: {
                    // The device is considered initialized on first HR write
                    model->run.initialized_by_master = 1;

                    switch (args->index) {
                        case MODBUS_HR_COMMAND: {
                            switch (args->value) {
                                case COMMAND_REGISTER_RESUME:
                                    model_cycle_resume(model);
                                    break;

                                case COMMAND_REGISTER_PAUSE:
                                    model_cycle_pause(model);
                                    break;

                                case COMMAND_REGISTER_STANDBY:
                                    model_cycle_standby(model);
                                    break;

                                case COMMAND_REGISTER_DONE:
                                    model_cycle_stop(model);
                                    break;

                                case COMMAND_REGISTER_CLEAR_ALARMS:
                                    model_clear_alarms(model);
                                    break;

                                case COMMAND_REGISTER_CLEAR_COINS:
                                    model_clear_coins(model);
                                    break;

                                case COMMAND_REGISTER_CLEAR_CYCLE_STATISTICS:
                                    model->statistics.complete_cycles = 0;
                                    model->statistics.partial_cycles  = 0;
                                    break;

                                default:
                                    break;
                            }
                            break;
                        }

                        case MODBUS_HR_INCREASE_DURATION: {
                            cycle_increase_duration(model, args->value);
                            break;
                        }

                        case MODBUS_HR_SET_DURATION: {
                            cycle_set_duration(model, args->value);
                            break;
                        }

                        case MODBUS_HR_TEST_MODE:
                            model->run.test.on = args->value > 0;
                            break;

                        case MODBUS_HR_TEST_OUTPUTS:
                            model->run.test.outputs = args->value;
                            break;

                        case MODBUS_HR_TEST_PWM:
                            model->run.test.pwm1 = (uint8_t)(args->value & 0xFF);
                            model->run.test.pwm2 = (uint8_t)((args->value >> 8) & 0xFF);
                            break;

                        case MODBUS_HR_COIN_READER_INHIBITION:
                            model->run.coin_reader_enabled = (uint8_t)(args->value > 0);
                            break;

                        case MODBUS_HR_BUSY_SIGNAL_TYPE:
                            model->run.parmac.busy_signal_type = (busy_signal_type_t)args->value;
                            break;

                        case MODBUS_HR_SAFETY_TEMPERATURE:
                            model->run.parmac.safety_temperature = (int16_t)args->value;
                            break;

                        case MODBUS_HR_TEMPERATURE_ALARM_DELAY_SECONDS:
                            model->run.parmac.temperature_alarm_delay_seconds = args->value;
                            break;

                        case MODBUS_HR_AIR_FLOW_ALARM_TIME:
                            model->run.parmac.air_flow_alarm_time = args->value;
                            break;

                        case MODBUS_HR_TEMPERATURE_PROBE:
                            model->run.parmac.temperature_probe = args->value;
                            break;

                        case MODBUS_HR_HEATING_TYPE:
                            model->run.parmac.heating_type = args->value;
                            break;

                        case MODBUS_HR_GAS_IGNITION_ATTEMPTS:
                            model->run.parmac.gas_ignition_attempts = args->value;
                            break;

                        case MODBUS_HR_FAN_WITH_OPEN_PORTHOLE_TIME:
                            model->run.parmac.fan_with_open_porthole_time = args->value;
                            break;

                        case MODBUS_HR_CYCLE_DELAY_TIME:
                            model->run.parmac.cycle_delay_time = args->value;
                            break;

                        case MODBUS_HR_CYCLE_RESET_TIME:
                            model->run.parmac.cycle_reset_time = args->value;
                            break;

                        case MODBUS_HR_FLAGS:
                            model->run.parmac.stop_time_in_pause    = (args->value & (1 << 0)) > 0;
                            model->run.parmac.disable_alarms        = (args->value & (1 << 1)) > 0;
                            model->run.parmac.emergency_alarm_nc_na = (args->value & (1 << 2)) > 0;
                            model->run.parmac.inverter_alarm_nc_na  = (args->value & (1 << 3)) > 0;
                            model->run.parmac.filter_alarm_nc_na    = (args->value & (1 << 4)) > 0;
                            model->run.parmac.porthole_nc_na        = (args->value & (1 << 5)) > 0;
                            model->run.parmac.busy_signal_nc_na     = (args->value & (1 << 6)) > 0;
                            model->run.parmac.enable_reverse        = (args->value & (1 << 7)) > 0;
                            model->run.parmac.wait_for_temperature  = (args->value & (1 << 8)) > 0;
                            break;

                        case MODBUS_HR_DURATION:
                            model->run.parmac.duration = args->value;
                            break;

                        case MODBUS_HR_DRYING_TYPE:
                            model->run.parmac.wait_for_humidity = args->value;
                            break;

                        case MODBUS_HR_SETPOINT_TEMPERATURE: {
                            if (model->run.parmac.setpoint_temperature != args->value) {
                                model->run.parmac.setpoint_temperature     = args->value;
                                model->run.heating.temperature_was_reached = 0;
                            }
                            break;
                        }

                        case MODBUS_HR_SETPOINT_HUMIDITY:
                            model->run.parmac.setpoint_humidity = args->value;
                            break;

                        case MODBUS_HR_TEMPERATURE_COOLING_HYSTERESIS:
                            model->run.parmac.temperature_cooling_hysteresis = args->value;
                            break;

                        case MODBUS_HR_TEMPERATURE_HEATING_HYSTERESIS:
                            model->run.parmac.temperature_heating_hysteresis = args->value;
                            break;

                        case MODBUS_HR_PROGRAM_NUMBER:
                            model->run.program_number = args->value;
                            break;

                        case MODBUS_HR_STEP_NUMBER:
                            model->run.step_number = args->value;
                            break;

                        case MODBUS_HR_ROTATION_RUNNING_TIME:
                            model->run.parmac.rotation_running_time = args->value;
                            break;

                        case MODBUS_HR_ROTATION_PAUSE_TIME:
                            model->run.parmac.rotation_pause_time = args->value;
                            break;

                        case MODBUS_HR_ROTATION_SPEED:
                            model->run.parmac.speed = args->value;
                            break;

                        case MODBUS_HR_STEP_TYPE:
                            model->run.step_type = args->value;
                            break;

                        default:
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return MODBUS_OK;
}


static ModbusError exception_callback(const ModbusSlave *minion, uint8_t function, ModbusExceptionCode code) {
    (void)minion;
    (void)function;
    (void)code;
    return MODBUS_OK;
}
