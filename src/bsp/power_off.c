#include "hal_data.h"
#include "power_off.h"


#define DATA_ID 0


static void (*user_callback)(void *) = NULL;
static void *user_data               = NULL;


void power_off_callback(external_irq_callback_args_t *p_args) {
    (void)p_args;
    if (user_callback != NULL) {
        user_callback(user_data);
    }
}


void bsp_power_off_init(void (*callback)(void *), void *data) {
    user_data     = data;
    user_callback = callback;

    /* Open the Virtual EEPROM Module. */
    fsp_err_t err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
    assert(FSP_SUCCESS == err);

    R_ICU_ExternalIrqEnable(&g_external_irq0_ctrl);
}


void bsp_power_off_save(uint8_t *data, uint16_t len) {
    /* Write the pressure data to a Virtual EEPROM Record. */
    fsp_err_t err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, DATA_ID, data, len);
    assert(FSP_SUCCESS == err);
}


uint8_t bsp_power_off_load(uint8_t **data, uint16_t *len) {
    fsp_err_t err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, DATA_ID, data, len);
    if (FSP_SUCCESS == err) {
        return 1;
    } else {
        return 0;
    }
}


void vee_callback(rm_vee_callback_args_t *p_args) {
    FSP_PARAMETER_NOT_USED(p_args);
}
