#ifndef IFXI2C_I2C_H
#define IFXI2C_I2C_H

#include "Ifx_Types.h"

typedef struct { void *bus; uint8 deviceAddress; } IfxI2c_I2c_Device;
typedef struct { void *i2c; } IfxI2c_I2c;
typedef struct { void *config; } IfxI2c_I2c_DeviceConfig;

static inline void IfxI2c_I2c_initConfig(IfxI2c_I2c *i2c) { (void)i2c; }
static inline void IfxI2c_I2c_initDevice(IfxI2c_I2c_Device *device, const IfxI2c_I2c *i2c) { (void)device; (void)i2c; }
static inline void IfxI2c_I2c_write(IfxI2c_I2c_Device *device, uint8 *data, uint32 size) { (void)device; (void)data; (void)size; }
static inline void IfxI2c_I2c_read(IfxI2c_I2c_Device *device, uint8 *data, uint32 size) { (void)device; (void)data; (void)size; }
static inline void IfxI2c_I2c_initDeviceConfig(IfxI2c_I2c_DeviceConfig *config, IfxI2c_I2c_Device *device) { (void)config; (void)device; }
static inline void IfxI2c_I2c_initModule(IfxI2c_I2c *i2c) { (void)i2c; }

#endif
