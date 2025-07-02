#include "adapters/modbus_server.h"
#include "model/model.h"
#include "observer.h"
#include "services/timestamp.h"
#include "bsp/adc.h"
#include "bsp/digin.h"
#include "bsp/temperature_humidity_probe.h"
#include "bsp/coin_reader.h"
#include "bsp/power_off.h"


void controller_init(mut_model_t *model) {
    model_init(model);

    {
        uint8_t buffer[PWOFF_SERIALIZED_SIZE] = {0};
        if (bsp_power_off_load(buffer, PWOFF_SERIALIZED_SIZE)) {
            model_pwoff_deserialize(model, buffer);
        }
    }

    observer_init(model);

    modbus_server_init();
}


void controller_manage(mut_model_t *model) {
    model_manage(model);
    modbus_server_manage(model);
    observer_manage(model);

    if (bsp_digin_is_ready()) {
        model_fix_alarms(model);
    }

    {
        static timestamp_t ts = 0;
        if (timestamp_is_expired(ts, 500)) {
            int16_t  temperature_probe = 0;
            uint16_t humidity_probe    = 0;

            if (model->run.parmac.temperature_probe == TEMPERATURE_PROBE_SHT) {
                model->run.sensors.temperature_humidity_probe_error =
                    (uint8_t)(temperature_humidity_probe_read(&temperature_probe, &humidity_probe) != 0);
            } else {
                model->run.sensors.temperature_humidity_probe_error = 0;
            }

            model_update_sensors(model, bsp_digin_get_bitmap(), bsp_adc_get(BSP_ADC_TEMPERATURE_INPUT),
                                 bsp_adc_get(BSP_ADC_TEMPERATURE_OUTPUT), bsp_adc_get(BSP_ADC_PRESS1),
                                 temperature_probe, humidity_probe);

            for (uint16_t i = BSP_COIN_READER_PAYMENT; i <= BSP_COIN_READER_LINE_5; i++) {
                model->run.sensors.coins[i] += bsp_coin_reader_read(i);
            }
            bsp_coin_reader_clear();

            ts = timestamp_get();
        }
    }
}


void controller_power_off(void *arg) {
    model_t *model = arg;

    model->statistics.active_time += timestamp_get() / 1000UL;

    {
        uint8_t buffer[PWOFF_SERIALIZED_SIZE] = {0};
        model_pwoff_serialize(model, buffer);
        bsp_power_off_save(buffer, PWOFF_SERIALIZED_SIZE);
    }
}
