#ifndef I2C_H_
#define I2C_H_

#include "stdint.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* I2C extension chip register addresses */
#define I2C_REG_CONFIG_PORT_0    0x0C    /* Configuration port 0 */
#define I2C_REG_CONFIG_PORT_1    0x0D    /* Configuration port 1 */
#define I2C_REG_CONFIG_PORT_2    0x0E    /* Configuration port 2 */
#define I2C_REG_OUTPUT_PORT_0    0x04    /* Output port 0 */
#define I2C_REG_OUTPUT_PORT_1    0x05    /* Output port 1 */
#define I2C_REG_OUTPUT_PORT_2    0x06    /* Output port 2 */

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void init_I2C_module(void);
void i2c_write_register(uint8_t reg_addr, uint8_t value);

#endif /* I2C_H_ */
