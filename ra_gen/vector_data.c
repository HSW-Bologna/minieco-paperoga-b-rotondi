/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = adc_scan_end_isr, /* ADC0 SCAN END (End of A/D scanning operation) */
            [3] = agt_int_isr, /* AGT0 INT (AGT interrupt) */
            [4] = sci_uart_rxi_isr, /* SCI2 RXI (Receive data full) */
            [5] = sci_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [6] = sci_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [7] = sci_uart_eri_isr, /* SCI2 ERI (Receive error) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_ADC0_SCAN_END,GROUP0), /* ADC0 SCAN END (End of A/D scanning operation) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_AGT0_INT,GROUP3), /* AGT0 INT (AGT interrupt) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI2_RXI,GROUP4), /* SCI2 RXI (Receive data full) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TXI,GROUP5), /* SCI2 TXI (Transmit data empty) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TEI,GROUP6), /* SCI2 TEI (Transmit end) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SCI2_ERI,GROUP7), /* SCI2 ERI (Receive error) */
        };
        #endif
        #endif
