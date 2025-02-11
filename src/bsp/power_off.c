#include "hal_data.h"
#include "power_off.h"


#define DATA_ID 0


static void (*user_callback)(void *) = NULL;
static void     *user_data           = NULL;
volatile uint8_t callback_called     = 0;

/* Record ID to use for storing pressure data. */
#define ID_PRESSURE (0U)
/* Example data structure. */
typedef struct st_pressure {
    uint32_t timestamp;
    uint16_t low;
    uint16_t average;
    uint16_t high;
} pressure_t;


void error_handler() {
    while (1)
        ;
}


void rm_vee_example() {
    /* Open the Virtual EEPROM Module. */
    fsp_err_t err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
    if (err == FSP_ERR_PE_FAILURE) {
        err = RM_VEE_FLASH_Refresh(&g_vee0_ctrl);
        assert(FSP_SUCCESS == err);
        err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
    }
    if (FSP_SUCCESS != err) {
        error_handler();
    }
    /* Read pressure data from external sensor. */
    pressure_t pressure_data = {.timestamp = 1, .low = 2, .average = 3, .high = 4};
    /* Write the pressure data to a Virtual EEPROM Record. */
    err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, ID_PRESSURE, (uint8_t *)&pressure_data, sizeof(pressure_t));
    if (FSP_SUCCESS != err) {
        error_handler();
    }
    /* Wait for the Virtual EEPROM callback to indicate it finished writing data. */
    while (false == callback_called) {
        ;
    }
    /* Get a pointer to the record that is stored in data flash. */
    uint32_t    length;
    pressure_t *p_pressure_data;
    err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, ID_PRESSURE, (uint8_t **)&p_pressure_data, &length);
    if (FSP_SUCCESS != err) {
        error_handler();
    }
    /* Close the Virtual EEPROM Module. */
    err = RM_VEE_FLASH_Close(&g_vee0_ctrl);
    if (FSP_SUCCESS != err) {
        error_handler();
    }
}


void power_off_callback(external_irq_callback_args_t *p_args) {
    (void)p_args;
    __NOP();
    __NOP();
    __NOP();
    if (user_callback != NULL) {
        user_callback(user_data);
    }
}


void bsp_power_off_init(void (*callback)(void *), void *data) {
    user_data     = data;
    user_callback = callback;

    //rm_vee_example();
    //return;

    /* Open the Virtual EEPROM Module. */
    fsp_err_t err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
    if (err == FSP_ERR_PE_FAILURE) {
        err = RM_VEE_FLASH_Refresh(&g_vee0_ctrl);
        assert(FSP_SUCCESS == err);
        err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
    }
    assert(FSP_SUCCESS == err);

    R_ICU_ExternalIrqOpen(&g_external_irq0_ctrl, &g_external_irq0_cfg);
    R_ICU_ExternalIrqEnable(&g_external_irq0_ctrl);
}


void bsp_power_off_save(uint8_t *data, uint16_t len) {
    /* Write the pressure data to a Virtual EEPROM Record. */
    fsp_err_t err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, DATA_ID, data, len);
    assert(FSP_SUCCESS == err);
}


uint8_t bsp_power_off_load(uint8_t **const data, uint32_t *const len) {
    fsp_err_t err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, DATA_ID, data, len);
    if (FSP_SUCCESS == err) {
        return 1;
    } else {
        return 0;
    }
}


void vee_callback(rm_vee_callback_args_t *p_args) {
    FSP_PARAMETER_NOT_USED(p_args);
    callback_called = 1;
}
