/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     Alex_min	   first version
 */


#ifndef __DEF_H_
#define __DEF_H_

#include <stddef.h>
#include <stdbool.h>
#include "type.h"
#include "board.h"

#define EOK                          0               /**< There is no error */
#define ERROR                        1               /**< A generic/unknown error happens */
#define ETIMEOUT                     2               /**< Timed out */
#define EFULL                        3               /**< The resource is full */
#define EEMPTY                       4               /**< The resource is empty */
#define ENOMEM                       5               /**< No memory */
#define ENOSYS                       6               /**< Function not implemented */
#define EBUSY                        7               /**< Busy */
#define EIO                          8               /**< IO error */
#define EINTR                        9               /**< Interrupted system call */
#define EINVAL                       10              /**< Invalid argument */
#define ENOENT                       11              /**< No entry */
#define ENOSPC                       12              /**< No space left */
#define EPERM                        13              /**< Operation not permitted */
#define ETRAP                        14              /**< Trap event */
#define EFAULT                       15              /**< Bad address */
#define ENOBUFS                      16              /**< No buffer space is available */
#define ESCHEDISR                    17              /**< scheduler failure in isr context */
#define ESCHEDLOCKED                 18              /**< scheduler failure in critical region */

#define ALIGN_SIZE					 8
#define NAME_MAX                     20
#define DEV_NUM_MAX                  64              /**< The maximum number of devices allowed for registration*/
/**
 * @ingroup group_basic_definition
 *
 * @def ALIGN(size, align)
 * Return the most contiguous size aligned at specified width. RT_ALIGN(13, 4)
 * would return 16.
 * @note align Must be an integer power of 2 or the result will be incorrect
 */
#define ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))

/**
 * @ingroup group_basic_definition
 *
 * @def ALIGN_DOWN(size, align)
 * Return the down number of aligned at specified width. RT_ALIGN_DOWN(13, 4)
 * would return 12.
 * @note align Must be an integer power of 2 or the result will be incorrect
 */
#define ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

/**
 * device flags definitions
 */
#define DEVICE_FLAG_DEACTIVATE       0x000           /**< device is not not initialized */

#define DEVICE_FLAG_RDONLY           0x001           /**< read only */
#define DEVICE_FLAG_WRONLY           0x002           /**< write only */
#define DEVICE_FLAG_RDWR             0x003           /**< read and write */

#define DEVICE_FLAG_REMOVABLE        0x004           /**< removable device */
#define DEVICE_FLAG_STANDALONE       0x008           /**< standalone device */
#define DEVICE_FLAG_ACTIVATED        0x010           /**< device is activated */
#define DEVICE_FLAG_SUSPENDED        0x020           /**< device is suspended */
#define DEVICE_FLAG_STREAM           0x040           /**< stream mode */
#define DEVICE_FLAG_DYNAMIC          0x080           /**< device is determined when open() */

#define DEVICE_FLAG_INT_RX           0x100           /**< INT mode on Rx */
#define DEVICE_FLAG_DMA_RX           0x200           /**< DMA mode on Rx */
#define DEVICE_FLAG_INT_TX           0x400           /**< INT mode on Tx */
#define DEVICE_FLAG_DMA_TX           0x800           /**< DMA mode on Tx */

#define DEVICE_OFLAG_CLOSE           0x000           /**< device is closed */
#define DEVICE_OFLAG_RDONLY          0x001           /**< read only access */
#define DEVICE_OFLAG_WRONLY          0x002           /**< write only access */
#define DEVICE_OFLAG_RDWR            0x003           /**< read and write */
#define DEVICE_OFLAG_OPEN            0x008           /**< device is opened */
#define DEVICE_OFLAG_MASK            0xf0f           /**< mask of open flag */

/**
 * general device commands
 * 0x01 - 0x1F general device control commands
 * 0x20 - 0x3F udevice control commands
 * 0x40 -      special device control commands
 */
#define DEVICE_CTRL_RESUME           0x01            /**< resume device */
#define DEVICE_CTRL_SUSPEND          0x02            /**< suspend device */
#define DEVICE_CTRL_CONFIG           0x03            /**< configure device */
#define DEVICE_CTRL_CLOSE            0x04            /**< close device */
#define DEVICE_CTRL_NOTIFY_SET       0x05            /**< set notify func */
#define DEVICE_CTRL_SET_INT          0x06            /**< set interrupt */
#define DEVICE_CTRL_CLR_INT          0x07            /**< clear interrupt */
#define DEVICE_CTRL_GET_INT          0x08            /**< get interrupt status */
#define DEVICE_CTRL_CONSOLE_OFLAG    0x09            /**< get console open flag */
#define DEVICE_CTRL_MASK             0x1f            /**< mask for contrl commands */

/**
 * device (I/O) class type
 */
typedef enum {
    Device_Class_Char = 0,                           /**< character device */
    Device_Class_Block,                              /**< block device */
    Device_Class_NetIf,                              /**< net interface */
    Device_Class_MTD,                                /**< memory device */
    Device_Class_CAN,                                /**< CAN device */
    Device_Class_LIN,                                /**< CAN device */
    Device_Class_RTC,                                /**< RTC device */
    Device_Class_Sound,                              /**< Sound device */
    Device_Class_Graphic,                            /**< Graphic device */
    Device_Class_I2CBUS,                             /**< I2C bus device */
    Device_Class_USBDevice,                          /**< USB slave device */
    Device_Class_USBHost,                            /**< USB host bus */
    Device_Class_USBOTG,                             /**< USB OTG bus */
    Device_Class_SPIBUS,                             /**< SPI bus device */
    Device_Class_SPIDevice,                          /**< SPI device */
    Device_Class_SDIO,                               /**< SDIO bus device */
    Device_Class_PM,                                 /**< PM pseudo device */
    Device_Class_Pipe,                               /**< Pipe device */
    Device_Class_Portal,                             /**< Portal device */
    Device_Class_Timer,                              /**< Timer device */
    Device_Class_Miscellaneous,                      /**< Miscellaneous device */
    Device_Class_Sensor,                             /**< Sensor device */
    Device_Class_Touch,                              /**< Touch device */
    Device_Class_PHY,                                /**< PHY device */
    Device_Class_Security,                           /**< Security device */
    Device_Class_WLAN,                               /**< WLAN device */
    Device_Class_Pin,                                /**< Pin device */
    Device_Class_ADC,                                /**< ADC device */
    Device_Class_DAC,                                /**< DAC device */
    Device_Class_WDT,                                /**< WDT device */
    Device_Class_PWM,                                /**< PWM device */
    Device_Class_Bus,                                /**< Bus device */
    Device_Class_Remote,                             /**< Remote IPCF device */
    Device_Class_OTA,                                /**< OTA device */
    Device_Class_Unknown                             /**< unknown device */
} device_class_type;

/**
 * device (I/O) class type
 */
enum device_priority {
    Device_PRIORITY_MIN = 0,
    Device_PRIORITY_DEFAULT = 5,
    Device_PRIORITY_MAX = 8
};

/**
 * operations set for device object
 */
typedef struct device *device_t;

struct device_ops
{
    /* common device interface */
    f_err_t  (*init)   (device_t dev);
    f_err_t  (*open)   (device_t dev, uint16_t oflag);
    f_err_t  (*close)  (device_t dev);
    f_err_t  (*read)   (device_t dev, off_t pos, void *buffer, size_t size);
    f_err_t  (*write)  (device_t dev, off_t pos, const void *buffer, size_t size);
    f_err_t  (*control)(device_t dev, int cmd, void *args);
};

typedef struct
{
    void *data;
    uint32_t len;
} Item;

/**
 * Device structure
 */
struct device
{
	char                      name[NAME_MAX];		    /**< device name */
    device_class_type         type;                     /**< device type */
    uint16_t                  flag;                     /**< device flag */
    uint16_t                  open_flag;                /**< device open flag */
    uint8_t                   ref_count;                /**< reference count */
    uint8_t                   device_id;                /**< 0 - 255 */
    uint16_t                  frames;                   /**< count of frames */
    bool                      immediate_trans;          /**< Bypass the schedule and transmit directly. */
    bool                      loopback;                 /**< Loopback mode for test. */
    spinLock                  *spinlock;                 /**< device lock. */

    /* device call back */
    f_err_t (*rx_indicate)(device_t dev, void *buffer);
    f_err_t (*tx_complete)(device_t dev, void *buffer);
    /* encode/decode function */
    f_err_t (*encode)(Item *item);
    f_err_t (*decode)(Item *item);

    struct device_ops *ops;
    struct list_head node;
    void                     *user_data;                /**< device private data */
    void                     *ring_buf;                 /**< device receive buf */
};

typedef struct 
{
	uint8_t *data;
	uint8_t *head;
	uint8_t *tail;
} protocol;

#endif /* __DEF_H_ */
