/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxI2c_I2c.h"
#include "i2c.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* MCP79411 I2C PINS */
#define MCP_SCL_PIN                 IfxI2c1_SCL_P11_14_INOUT     /* SCL PIN                                          */
#define MCP_SDA_PIN                 IfxI2c1_SDA_P11_13_INOUT     /* SDA PIN                                          */
#define I2C_BAUDRATE                400000                      /* 400 kHz baud rate                                */

#define SLAVE_DEVICE_ADDRESS        0x20                        /* 7 bit slave device address for reading from EEPROM
                                                                   of MCP79411 is 0b1010111 which is 0x57.          */
#define ADDRESS_OF_MAC_ADDRESS      0xF2                        /* Location of EUI-48 node address (MAC address)    */
#define LENGTH_OF_ADDRESS           1                           /* Length of address of the register, in which the
                                                                   requested MAC address is stored in bytes         */
#define LENGTH_OF_MAC_ADDRESS       6                           /* Length of the MAC address in bytes               */

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
IfxI2c_I2c g_i2cHandle;                                         /* I2C handle                                       */
IfxI2c_I2c_Device g_i2cDevEeprom;                               /* I2C Slave device handle to EEPROM of MC79411     */

uint8 g_macAddr[LENGTH_OF_MAC_ADDRESS] = {0, 0, 0, 0, 0, 0};    /* Global parameter for 6-byte EUI-48 MAC address   */

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* This function initializes the I2C in master mode and configures the MCP79411 (real time clock) as an I2C slave */
void init_I2C_module(void)
{
    /* Initialize module */
    IfxI2c_I2c_Config i2cConfig;                                    /* Create configuration structure               */
    IfxI2c_I2c_initConfig(&i2cConfig, &MODULE_I2C1);                /* Fill structure with default values and Module
                                                                       address                                      */
    /* I2c pin configuration */
    const IfxI2c_Pins MCP_PINS =
    {
            &MCP_SCL_PIN,                                           /* SCL port pin                                 */
            &MCP_SDA_PIN,                                           /* SDA port pin                                 */
            IfxPort_PadDriver_ttlSpeed1                             /* Pad driver mode                              */
    };
    i2cConfig.pins = &MCP_PINS;                                     /* Configure port pins                          */
    i2cConfig.baudrate = I2C_BAUDRATE;                              /* Configure baud rate with 400kHz              */

    IfxI2c_I2c_initModule(&g_i2cHandle, &i2cConfig);                /* Initialize module                            */

    /* Initialize device */
    IfxI2c_I2c_deviceConfig i2cDeviceConfig;                        /* Create device configuration                  */
    IfxI2c_I2c_initDeviceConfig(&i2cDeviceConfig, &g_i2cHandle);    /* Fill structure with default values and I2C
                                                                       Handler                                      */
    /* Because it is 7 bit long and bit 0 is R/W bit, the device address has to be shifted by 1 */
    i2cDeviceConfig.deviceAddress = SLAVE_DEVICE_ADDRESS << 1;
    IfxI2c_I2c_initDevice(&g_i2cDevEeprom, &i2cDeviceConfig);       /* Initialize the I2C device handle             */
}


void i2c_write_register(uint8_t reg_addr, uint8_t value)
{
    /* Address of 6-byte EUI-48 MAC address location */
    uint8_t i2cTxBuffer[2];

    i2cTxBuffer[0] = reg_addr;
    i2cTxBuffer[1] = value;
    while(IfxI2c_I2c_write(&g_i2cDevEeprom, (uint8_t *)i2cTxBuffer, 2) != IfxI2c_I2c_Status_ok);
}
