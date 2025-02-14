#include "hal_data.h"
#include "power_off.h"
#include "services/crc16-ccitt.h"
#include "services/serializer.h"


#define FLASH_DF_BLOCK_0_ADDRESS      0x40100000U
#define DATA_FLASH_SIZE               8192
#define BLOCK_SIZE                    1024
#define SUB_BLOCK_SIZE                64
#define NUMBER_OF_SUB_BLOCKS_IN_BLOCK (BLOCK_SIZE / SUB_BLOCK_SIZE)
#define NUMBER_OF_SUB_BLOCKS_TOTAL    (DATA_FLASH_SIZE / SUB_BLOCK_SIZE)
#define NUMBER_OF_BLOCKS_TOTAL        (DATA_FLASH_SIZE / BLOCK_SIZE)


static int16_t find_last_sub_block_index(uint32_t *counter);
static void    erase_block(uint16_t block_index);


static void (*user_callback)(void *) = NULL;
static void    *user_data            = NULL;
static int16_t  last_sub_block_index = 0;
static int16_t  next_sub_block_index = 0;
static uint32_t current_counter      = 0;



void power_off_callback(external_irq_callback_args_t *p_args) {
    (void)p_args;
    if (user_callback != NULL) {
        user_callback(user_data);
    }
}


void bsp_power_off_init(void (*callback)(void *), void *data) {
    user_data     = data;
    user_callback = callback;

    /* Open the flash lp instance. */
    fsp_err_t err = R_FLASH_LP_Open(&g_flash_ctrl, &g_flash_cfg);
    assert(FSP_SUCCESS == err);

    last_sub_block_index = find_last_sub_block_index(&current_counter);
    // Reset everything
    if (last_sub_block_index == -2) {
        err = R_FLASH_LP_Erase(&g_flash_ctrl, FLASH_DF_BLOCK_0_ADDRESS, NUMBER_OF_BLOCKS_TOTAL);
        assert(FSP_SUCCESS == err);
        next_sub_block_index = 0;
    }
    // Not found, start at 0
    else if (last_sub_block_index == -1) {
        erase_block(0);
        next_sub_block_index = 0;
    }
    // Block found
    else {
        next_sub_block_index = (last_sub_block_index + 1) % NUMBER_OF_SUB_BLOCKS_TOTAL;
        // First write in  new block, should clear it
        if ((next_sub_block_index % NUMBER_OF_SUB_BLOCKS_IN_BLOCK) == 0) {
            erase_block((uint16_t)(next_sub_block_index / NUMBER_OF_SUB_BLOCKS_IN_BLOCK));
        }
    }

    R_ICU_ExternalIrqOpen(&g_external_irq0_ctrl, &g_external_irq0_cfg);
    R_ICU_ExternalIrqEnable(&g_external_irq0_ctrl);
}


void bsp_power_off_save(uint8_t *data, uint16_t len) {
    assert(len < SUB_BLOCK_SIZE - 4);

    uint8_t buffer[SUB_BLOCK_SIZE] = {0};
    serialize_uint32_be(&buffer[0], current_counter);
    memcpy(&buffer[4], data, len);

    uint16_t crc = crc16_ccitt(buffer, SUB_BLOCK_SIZE - 2, 0);
    serialize_uint32_be(&buffer[SUB_BLOCK_SIZE - 2], crc);

    uint32_t address = FLASH_DF_BLOCK_0_ADDRESS + ((uint16_t)next_sub_block_index * SUB_BLOCK_SIZE);
    R_FLASH_LP_Write(&g_flash_ctrl, (uint32_t)(uintptr_t)buffer, address, SUB_BLOCK_SIZE);
}


uint8_t bsp_power_off_load(uint8_t *data, uint16_t len) {
    if (last_sub_block_index < 0) {
        return 0;
    } else {
        uint8_t *address =
            (uint8_t *)(uintptr_t)(FLASH_DF_BLOCK_0_ADDRESS + ((uint16_t)last_sub_block_index * SUB_BLOCK_SIZE));
        assert(len < SUB_BLOCK_SIZE - 4);
        memcpy(data, address, len);
        return 1;
    }
}


static int16_t find_last_sub_block_index(uint32_t *counter) {
    uint32_t max_counter = 0;
    int16_t  found_index = -1;

    for (uint16_t i = 0; i < NUMBER_OF_SUB_BLOCKS_TOTAL; i++) {
        uint8_t *pointer = (uint8_t *)(uintptr_t)(FLASH_DF_BLOCK_0_ADDRESS + i * SUB_BLOCK_SIZE);

        uint32_t read_counter = 0;
        deserialize_uint32_be(&read_counter, &pointer[0]);

        uint16_t crc_read = 0;
        deserialize_uint16_be(&crc_read, &pointer[SUB_BLOCK_SIZE - 2]);

        uint16_t crc_calc = crc16_ccitt(&pointer[0], SUB_BLOCK_SIZE - 2, 0);

        // Ignore wrong CRCs
        if (crc_calc == crc_read) {
            // Reached the limit, clear everything and restart (happens after an impossibly long time)
            if (read_counter == 0xFFFFFFFF) {
                *counter = 0;
                return -2;
            }
            // This is the last block (until now)
            else if (read_counter >= max_counter) {
                max_counter = read_counter;
                found_index = (int16_t)i;
            }
        }
    }
    *counter = max_counter + 1;

    return found_index;
}


static void erase_block(uint16_t block_index) {
    fsp_err_t err = R_FLASH_LP_Erase(&g_flash_ctrl, FLASH_DF_BLOCK_0_ADDRESS + BLOCK_SIZE * block_index, 1);
    assert(FSP_SUCCESS == err);
}
