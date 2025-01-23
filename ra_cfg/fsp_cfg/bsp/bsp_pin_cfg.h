/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define BSP_PIN_P_G5 (BSP_IO_PORT_00_PIN_00) /* Coin 5 line */
#define BSP_PIN_P_G4 (BSP_IO_PORT_00_PIN_01) /* Coin 4 line */
#define BSP_PIN_P_G3 (BSP_IO_PORT_00_PIN_02) /* Coin 3 line */
#define BSP_PIN_P_G2 (BSP_IO_PORT_00_PIN_03) /* Coin 2 line */
#define BSP_PIN_P_G1 (BSP_IO_PORT_00_PIN_04) /* Coin 1 line */
#define BSP_PIN_INIBIT (BSP_IO_PORT_00_PIN_10) /* Coin reader inhibition */
#define BSP_PIN_PRESS1 (BSP_IO_PORT_00_PIN_11) /* Pressure sensor ADC */
#define BSP_PIN_TEMP1 (BSP_IO_PORT_00_PIN_12) /* Temperature sensor 2 ADC */
#define BSP_PIN_TEMP (BSP_IO_PORT_00_PIN_13) /* Temperature sensor 1 ADC */
#define BSP_PIN_IN7 (BSP_IO_PORT_00_PIN_14) /* Input 7 */
#define BSP_PIN_IN6 (BSP_IO_PORT_00_PIN_15) /* Input 6 */
#define BSP_PIN_IN4 (BSP_IO_PORT_01_PIN_00) /* Digital input 4 */
#define BSP_PIN_IN3 (BSP_IO_PORT_01_PIN_01) /* Digital input 3 */
#define BSP_PIN_IN2 (BSP_IO_PORT_01_PIN_02) /* Digital input 2 */
#define BSP_PIN_IN1 (BSP_IO_PORT_01_PIN_03) /* Digital input 1 */
#define BSP_PIN_RL6 (BSP_IO_PORT_01_PIN_04) /* Relay 6 */
#define BSP_PIN_RL5 (BSP_IO_PORT_01_PIN_05) /* Relay 5 */
#define BSP_PIN_RL4 (BSP_IO_PORT_01_PIN_06) /* Relay 4 */
#define BSP_PIN_RL3 (BSP_IO_PORT_01_PIN_07) /* Relay 3 */
#define BSP_PIN_MOSI (BSP_IO_PORT_01_PIN_09) /* SPI MOSI */
#define BSP_PIN_MISO (BSP_IO_PORT_01_PIN_10) /* SPI MISO */
#define BSP_PIN_CLK (BSP_IO_PORT_01_PIN_11) /* SPI clock */
#define BSP_PIN_RL1 (BSP_IO_PORT_01_PIN_12) /* Relay 1 */
#define BSP_PIN_RL2 (BSP_IO_PORT_01_PIN_13) /* Relay 2 */
#define BSP_PIN_SW2 (BSP_IO_PORT_02_PIN_04) /* Dipswitch line 2 */
#define BSP_PIN_INT0 (BSP_IO_PORT_02_PIN_06) /* Power off interrupt */
#define BSP_PIN_SW1 (BSP_IO_PORT_02_PIN_07) /* Dipswitch line 1 */
#define BSP_PIN_RX (BSP_IO_PORT_03_PIN_01) /* Machine communication RS232 RX Pin */
#define BSP_PIN_TX (BSP_IO_PORT_03_PIN_02) /* Machine communication RS232 TX Pin */
#define BSP_PIN_CS1 (BSP_IO_PORT_03_PIN_03) /* Temperature/Humidity sensor CS */
#define BSP_PIN_RUN1 (BSP_IO_PORT_04_PIN_00) /* Heartbit LED */
#define BSP_PIN_RL7 (BSP_IO_PORT_04_PIN_01) /* Rele' 7 */
#define BSP_PIN_IN8 (BSP_IO_PORT_04_PIN_02) /* Input 8 */
#define BSP_PIN_DIR (BSP_IO_PORT_04_PIN_09)
#define BSP_PIN_RX1 (BSP_IO_PORT_04_PIN_10)
#define BSP_PIN_TX1 (BSP_IO_PORT_04_PIN_11)
#define BSP_PIN_PWM2 (BSP_IO_PORT_05_PIN_00) /* Second PWM channel */
#define BSP_PIN_PWM1 (BSP_IO_PORT_05_PIN_01) /* First PWM channel */
#define BSP_PIN_IN5 (BSP_IO_PORT_05_PIN_02) /* Digital input 5 */
extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA2L1AB2DFM.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
