/**
 * @file gpio_demo.h
 * @brief GPIO demo public definitions and prototypes.
 *
 * Contains GPIO level/direction enums, platform pin maps and the public
 * function declarations used by the GPIO demo implementation.
 *
 * @note One of SIM7070 / SIM7080 / SIM7090 must be defined to select the
 *       appropriate pin mapping for the target module.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __GPIO_DEMO_H__
#define __GPIO_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"


//Customer need to define one module for GPIO control

#define SIM7070
//#define SIM7080
//#define SIM7090

typedef enum
{
  GPIO_LOW_LEVEL,     /**< Drive the output LOW. */
  GPIO_HIGH_LEVEL,    /**< Drive the output HIGH. */

}GPIO_Level_t;

typedef enum
{
  GPIO_INPUT_DIR,     /**< Input Direction. */
  GPIO_OUTPUT_DIR,    /**< Output Direction. */

}GPIO_Dir_t;

#ifdef SIM7070
typedef enum
{
    PIN3_UART_DTR = 49,
    PIN4_UART_RI = 26,
    PIN5_UART_DCD = 28,
    PIN7_UART_CTS = 14,
    PIN8_UART_RTS = 15,
    PIN11_PCM_CLK = 24,
    PIN12_PCM_SYNC = 21,
    PIN13_PCM_DIN = 22,
    PIN14_PCM_DOUT = 23,
    PIN19_GPIO1 = 4,
    PIN20_GPIO2 = 5,
    PIN21_GPIO3 = 7,
    PIN22_UART2_RXD = 1,
    PIN23_UART2_TXD = 0,
    PIN34_GPIO4 = 19,
    PIN37_I2C_SDA = 2,
    PIN38_I2C_SCL = 3,
    PIN48_GPIO5 = 6,
    PIN49_UART3_RXD = 9,
    PIN50_UART3_TXD = 8,
    PIN52_NETLIGHT = 11,
    PIN66_STATUS = 10,
    PIN67_GPIO6 = 30,
    PIN68_GPIO7 = 52,

}DAM_GPIO_PINMAP;

#elif defined SIM7080

typedef enum
{
    PIN3_UART_RTS = 15,
    PIN4_UART_CTS = 14,
    PIN5_UART_DCD = 28,
    PIN6_UART_DTR = 49,
    PIN7_UART_RI = 26,
    PIN9_PCM_DI = 22,
    PIN10_PCM_DOUT = 23,
    PIN11_PCM_CLK = 24,
    PIN12_PCM_SYNC = 21,
    PIN14_GPIO5 = 19,
    PIN22_UART2_TXD = 0,
    PIN23_UART2_RXD = 1,
    PIN41_NETLIGHT = 11,
    PIN42_STATUS = 10,
    PIN48_SPI_CS = 6,
    PIN49_SPI_MOSI = 4,
    PIN50_SPI_CLK = 7,
    PIN51_SPI_MISO = 5,
    PIN57_GPIO1 = 50,
    PIN58_GPIO2 = 30,
    PIN59_GPIO3 = 25,
    PIN60_GPIO4 = 52,
    PIN61_UART3_TXD = 8,
    PIN62_UART3_RXD = 9,
    PIN64_I2C_SDA = 2,
    PIN65_I2C_SCL = 3,

}DAM_GPIO_PINMAP;

#elif defined SIM7090

typedef enum
{
    PIN1_GPIO1 = 50,
    PIN2_PCM_DI = 22,
    PIN3_PCM_CLK = 24,
    PIN4_GNSS_UART_RXD = 9,
    PIN5_I2C_SDA = 2,
    PIN8_SPI_MISO = 5,
    PIN9_SPI_CLK = 7,
    PIN33_GPIO33 = 30,
    PIN34_PCM_DOUT = 23,
    PIN35_PCM_SYNC = 21,
    PIN36_GNSS_UART_TXD = 8,
    PIN37_I2C_SCL = 3,
    PIN38_UART_RTS = 15,
    PIN39_UART_CTS = 14,
    PIN40_SPI_MOSI = 4,
    PIN60_DBG_TXD = 0,
    PIN61_DBG_RXD = 1,
    PIN62_UART_DTR = 49,
    PIN63_SPI_CS = 6,
    PIN76_UART_RI = 26,
    PIN77_AP_READY = 52,
    PIN78_STATUS = 10,
    PIN79_NETLIGHT = 11,
    PIN90_UART_DCD = 28,


}DAM_GPIO_PINMAP;

#else

    #error "No module defined,please define SIM7070 or SIM7080 or SIM7090"

#endif

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Register an interrupt handler for a GPIO pin.
 * @param[in] gpio_num GPIO pin number to register the interrupt for.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gpio_interrupt_register(uint32_t gpio_num);

/**
 * @brief Initialize a GPIO pin with direction and initial level.
 * @param[in] gpio_num GPIO pin number to configure.
 * @param[in] dir Direction of the GPIO (input/output).
 * @param[in] level Initial logical level when configured as output.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gpio_init(uint32_t gpio_num, GPIO_Dir_t dir , GPIO_Level_t level);

/**
 * @brief Write a logical level to a GPIO pin.
 * @param[in] gpio_num GPIO pin number to write.
 * @param[in] level Logical level to set (GPIO_LOW_LEVEL or GPIO_HIGH_LEVEL).
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gpio_write_pin(uint32_t gpio_num, GPIO_Level_t level);

/**
 * @brief Read the logical level from a GPIO pin.
 * @param[in] gpio_num GPIO pin number to read.
 * @param[out] level Pointer to receive the pin level.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gpio_read_pin(uint32_t gpio_num , GPIO_Level_t *level);

#endif

