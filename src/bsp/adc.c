#include <stdint.h>
#include "hal_data.h"
#include "adc.h"
#include "services/timestamp.h"


#define AVERAGE_SAMPLES 10


static uint16_t samples_index                   = 0;
static uint16_t press1_samples[AVERAGE_SAMPLES] = {0};
static uint16_t temp_samples[AVERAGE_SAMPLES]   = {0};
static uint16_t temp1_samples[AVERAGE_SAMPLES]  = {0};


void bsp_adc_init(void) {
    fsp_err_t ret = FSP_SUCCESS;
    ret           = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    assert(FSP_SUCCESS == ret);

    ret = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
    assert(FSP_SUCCESS == ret);

    R_BSP_IrqDisable(ADC0_SCAN_END_IRQn);
    (void)R_ADC_ScanStart(&g_adc0_ctrl);
}


void bsp_adc_manage(void) {
    static timestamp_t ts = 0;

    if (timestamp_is_expired(ts, 500)) {
        const adc_channel_t channels[BSP_ADC_NUM] = {ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_8};
        uint16_t *const     arrays[BSP_ADC_NUM]   = {press1_samples, temp1_samples, temp_samples};

        for (bsp_adc_t adc = 0; adc < BSP_ADC_NUM; adc++) {
            R_ADC_Read(&g_adc0_ctrl, channels[adc], &(arrays[adc][samples_index]));
        }

        samples_index = (samples_index + 1) % AVERAGE_SAMPLES;

        // Start a new scan
        (void)R_ADC_ScanStart(&g_adc0_ctrl);

        ts = timestamp_get();
    }
}


uint16_t bsp_adc_get(bsp_adc_t adc) {
    uint16_t *const arrays[BSP_ADC_NUM] = {press1_samples, temp1_samples, temp_samples};

    uint32_t total = 0;
    for (size_t i = 0; i < AVERAGE_SAMPLES; i++) {
        total += arrays[adc][i];
    }

    return (uint16_t)(total / AVERAGE_SAMPLES);
}

void bsp_adc_callback(adc_callback_args_t *p_arg) {
    switch (p_arg->event) {
        case ADC_EVENT_SCAN_COMPLETE: {
            break;
        }

        case ADC_EVENT_WINDOW_COMPARE_A: {
            break;
        }

        case ADC_EVENT_WINDOW_COMPARE_B: {
            break;
        }

        default: {
            break;
        }
    }

    return;
}
