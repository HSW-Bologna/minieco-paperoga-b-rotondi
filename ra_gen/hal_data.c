/* generated HAL source file - do not edit */
#include "hal_data.h"

flash_lp_instance_ctrl_t g_flash_ctrl;
const flash_cfg_t g_flash_cfg = { .data_flash_bgo = false, .p_callback = NULL,
        .p_context = NULL, .ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
        .irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const flash_instance_t g_flash = { .p_ctrl = &g_flash_ctrl, .p_cfg =
        &g_flash_cfg, .p_api = &g_flash_on_flash_lp };
gpt_instance_ctrl_t g_timer_pwm_ctrl;
#if 0
const gpt_extended_pwm_cfg_t g_timer_pwm_pwm_extend =
{
    .trough_ipl          = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT2_COUNTER_UNDERFLOW)
    .trough_irq          = VECTOR_NUMBER_GPT2_COUNTER_UNDERFLOW,
#else
    .trough_irq          = FSP_INVALID_VECTOR,
#endif
    .poeg_link           = GPT_POEG_LINK_POEG0,
    .output_disable      = (gpt_output_disable_t) ( GPT_OUTPUT_DISABLE_NONE),
    .adc_trigger         = (gpt_adc_trigger_t) ( GPT_ADC_TRIGGER_NONE),
    .dead_time_count_up  = 0,
    .dead_time_count_down = 0,
    .adc_a_compare_match = 0,
    .adc_b_compare_match = 0,
    .interrupt_skip_source = GPT_INTERRUPT_SKIP_SOURCE_NONE,
    .interrupt_skip_count  = GPT_INTERRUPT_SKIP_COUNT_0,
    .interrupt_skip_adc    = GPT_INTERRUPT_SKIP_ADC_NONE,
    .gtioca_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
    .gtiocb_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
};
#endif
const gpt_extended_cfg_t g_timer_pwm_extend =
        {
                .gtioca = { .output_enabled = true, .stop_level =
                        GPT_PIN_LEVEL_HIGH }, .gtiocb =
                        { .output_enabled = true, .stop_level =
                                GPT_PIN_LEVEL_HIGH }, .start_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .stop_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .clear_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .count_up_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .count_down_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .capture_a_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .capture_b_source =
                        (gpt_source_t)(GPT_SOURCE_NONE), .capture_a_ipl =
                        (BSP_IRQ_DISABLED), .capture_b_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT2_CAPTURE_COMPARE_A)
    .capture_a_irq       = VECTOR_NUMBER_GPT2_CAPTURE_COMPARE_A,
#else
                .capture_a_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_GPT2_CAPTURE_COMPARE_B)
    .capture_b_irq       = VECTOR_NUMBER_GPT2_CAPTURE_COMPARE_B,
#else
                .capture_b_irq = FSP_INVALID_VECTOR,
#endif
                .compare_match_value = { /* CMP_A */0x0, /* CMP_B */0x0 },
                .compare_match_status = (0U << 1U) | 0U,
                .capture_filter_gtioca = GPT_CAPTURE_FILTER_NONE,
                .capture_filter_gtiocb = GPT_CAPTURE_FILTER_NONE,
#if 0
    .p_pwm_cfg                   = &g_timer_pwm_pwm_extend,
#else
                .p_pwm_cfg = NULL,
#endif
#if 0
    .gtior_setting.gtior_b.gtioa  = (0U << 4U) | (0U << 2U) | (0U << 0U),
    .gtior_setting.gtior_b.oadflt = (uint32_t) GPT_PIN_LEVEL_HIGH,
    .gtior_setting.gtior_b.oahld  = 0U,
    .gtior_setting.gtior_b.oae    = (uint32_t) true,
    .gtior_setting.gtior_b.oadf   = (uint32_t) GPT_GTIOC_DISABLE_PROHIBITED,
    .gtior_setting.gtior_b.nfaen  = ((uint32_t) GPT_CAPTURE_FILTER_NONE & 1U),
    .gtior_setting.gtior_b.nfcsa  = ((uint32_t) GPT_CAPTURE_FILTER_NONE >> 1U),
    .gtior_setting.gtior_b.gtiob  = (0U << 4U) | (0U << 2U) | (0U << 0U),
    .gtior_setting.gtior_b.obdflt = (uint32_t) GPT_PIN_LEVEL_HIGH,
    .gtior_setting.gtior_b.obhld  = 0U,
    .gtior_setting.gtior_b.obe    = (uint32_t) true,
    .gtior_setting.gtior_b.obdf   = (uint32_t) GPT_GTIOC_DISABLE_PROHIBITED,
    .gtior_setting.gtior_b.nfben  = ((uint32_t) GPT_CAPTURE_FILTER_NONE & 1U),
    .gtior_setting.gtior_b.nfcsb  = ((uint32_t) GPT_CAPTURE_FILTER_NONE >> 1U),
#else
                .gtior_setting.gtior = 0U,
#endif
        };

const timer_cfg_t g_timer_pwm_cfg = { .mode = TIMER_MODE_PWM,
/* Actual period: 0.0005 seconds. Actual duty: 50%. */.period_counts =
        (uint32_t) 0x7d00, .duty_cycle_counts = 0x3e80, .source_div =
        (timer_source_div_t) 0, .channel = 2, .p_callback = NULL,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
        .p_context = &NULL,
#endif
        .p_extend = &g_timer_pwm_extend, .cycle_end_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT2_COUNTER_OVERFLOW)
    .cycle_end_irq       = VECTOR_NUMBER_GPT2_COUNTER_OVERFLOW,
#else
        .cycle_end_irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const timer_instance_t g_timer_pwm = { .p_ctrl = &g_timer_pwm_ctrl, .p_cfg =
        &g_timer_pwm_cfg, .p_api = &g_timer_on_gpt };
adc_instance_ctrl_t g_adc0_ctrl;
const adc_extended_cfg_t g_adc0_cfg_extend = { .add_average_count = ADC_ADD_OFF,
        .clearing = ADC_CLEAR_AFTER_READ_ON, .trigger =
                ADC_START_SOURCE_DISABLED, .trigger_group_b =
                ADC_START_SOURCE_DISABLED, .double_trigger_mode =
                ADC_DOUBLE_TRIGGER_DISABLED, .adc_vref_control =
                ADC_VREF_CONTROL_AVCC0_AVSS0, .enable_adbuf = 0,
#if defined(VECTOR_NUMBER_ADC0_WINDOW_A)
    .window_a_irq        = VECTOR_NUMBER_ADC0_WINDOW_A,
#else
        .window_a_irq = FSP_INVALID_VECTOR,
#endif
        .window_a_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_ADC0_WINDOW_B)
    .window_b_irq      = VECTOR_NUMBER_ADC0_WINDOW_B,
#else
        .window_b_irq = FSP_INVALID_VECTOR,
#endif
        .window_b_ipl = (BSP_IRQ_DISABLED), };
const adc_cfg_t g_adc0_cfg = { .unit = 0, .mode = ADC_MODE_SINGLE_SCAN,
        .resolution = ADC_RESOLUTION_12_BIT, .alignment =
                (adc_alignment_t) ADC_ALIGNMENT_RIGHT, .trigger =
                (adc_trigger_t) 0xF, // Not used
        .p_callback = bsp_adc_callback,
        /** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
        .p_context = &NULL,
#endif
        .p_extend = &g_adc0_cfg_extend,
#if defined(VECTOR_NUMBER_ADC0_SCAN_END)
    .scan_end_irq        = VECTOR_NUMBER_ADC0_SCAN_END,
#else
        .scan_end_irq = FSP_INVALID_VECTOR,
#endif
        .scan_end_ipl = (3),
#if defined(VECTOR_NUMBER_ADC0_SCAN_END_B)
    .scan_end_b_irq      = VECTOR_NUMBER_ADC0_SCAN_END_B,
#else
        .scan_end_b_irq = FSP_INVALID_VECTOR,
#endif
        .scan_end_b_ipl = (BSP_IRQ_DISABLED), };
#if ((0) | (0))
const adc_window_cfg_t g_adc0_window_cfg =
{
    .compare_mask        =  0,
    .compare_mode_mask   =  0,
    .compare_cfg         = (0) | (0) | (0) | (ADC_COMPARE_CFG_EVENT_OUTPUT_OR),
    .compare_ref_low     = 0,
    .compare_ref_high    = 0,
    .compare_b_channel   = (ADC_WINDOW_B_CHANNEL_0),
    .compare_b_mode      = (ADC_WINDOW_B_MODE_LESS_THAN_OR_OUTSIDE),
    .compare_b_ref_low   = 0,
    .compare_b_ref_high  = 0,
};
#endif
const adc_channel_cfg_t g_adc0_channel_cfg = { .scan_mask = ADC_MASK_CHANNEL_6
        | ADC_MASK_CHANNEL_7 | ADC_MASK_CHANNEL_8 | 0, .scan_mask_group_b = 0,
        .priority_group_a = ADC_GROUP_A_PRIORITY_OFF, .add_mask = 0,
        .sample_hold_mask = 0, .sample_hold_states = 24,
#if ((0) | (0))
    .p_window_cfg        = (adc_window_cfg_t *) &g_adc0_window_cfg,
#else
        .p_window_cfg = NULL,
#endif
        };
/* Instance structure to use this module. */
const adc_instance_t g_adc0 = { .p_ctrl = &g_adc0_ctrl, .p_cfg = &g_adc0_cfg,
        .p_channel_cfg = &g_adc0_channel_cfg, .p_api = &g_adc_on_adc };
sci_uart_instance_ctrl_t g_uart0_ctrl;

baud_setting_t g_uart0_baud_setting = {
/* Baud rate calculated with 0.644% error. */.semr_baudrate_bits_b.abcse = 1,
        .semr_baudrate_bits_b.abcs = 0, .semr_baudrate_bits_b.bgdm = 0,
        .cks = 0, .brr = 22, .mddr = (uint8_t) 256, .semr_baudrate_bits_b.brme =
                false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_uart0_cfg_extend = {
        .clock = SCI_UART_CLOCK_INT, .rx_edge_start =
                SCI_UART_START_BIT_FALLING_EDGE, .noise_cancel =
                SCI_UART_NOISE_CANCELLATION_DISABLE, .rx_fifo_trigger =
                SCI_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting =
                &g_uart0_baud_setting,
        .flow_control = SCI_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
        .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
        .rs485_setting = { .enable = SCI_UART_RS485_DISABLE, .polarity =
                SCI_UART_RS485_DE_POLARITY_HIGH,
#if 0xFF != 0xFF
                    .de_control_pin = BSP_IO_PORT_FF_PIN_0xFF,
                #else
                .de_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
                }, };

/** UART interface configuration */
const uart_cfg_t g_uart0_cfg = { .channel = 2, .data_bits = UART_DATA_BITS_8,
        .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
                rs232_callback, .p_context = NULL, .p_extend =
                &g_uart0_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
        .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
        .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
        .rxi_ipl = (0), .txi_ipl = (2), .tei_ipl = (2), .eri_ipl = (2),
#if defined(VECTOR_NUMBER_SCI2_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI2_RXI,
#else
        .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI2_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI2_TXI,
#else
        .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI2_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI2_TEI,
#else
        .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI2_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI2_ERI,
#else
        .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart0 = { .p_ctrl = &g_uart0_ctrl,
        .p_cfg = &g_uart0_cfg, .p_api = &g_uart_on_sci };
void g_hal_init(void) {
    g_common_init();
}
