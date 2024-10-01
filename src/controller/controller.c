#include "adapters/modbus_server.h"
#include "model/model.h"
#include "observer.h"
#include "services/timestamp.h"
#include "bsp/adc.h"
#include "bsp/digin.h"


void controller_init(mut_model_t *model) {
    model_init(model);
    observer_init(model);

    modbus_server_init();
}


void controller_manage(mut_model_t *model) {
    model_manage(model);
    modbus_server_manage(model);
    observer_manage(model);

    {
        static timestamp_t ts = 0;
        if (timestamp_is_expired(ts, 500)) {
            model_update_temperature(model, TEMPERATURE_PROBE_PTC_1, bsp_adc_get(BSP_ADC_TEMP));
            model_update_temperature(model, TEMPERATURE_PROBE_PTC_2, bsp_adc_get(BSP_ADC_TEMP1));

            model->run.sensors.inputs = bsp_digin_get_bitmap();

            ts = timestamp_get();
        }
    }
}
