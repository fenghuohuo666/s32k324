/*
 * Copyright (c) 2025, Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-08     Alex_min     first version
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "device.h"
#include "list.h"
#include "bitmap.h"

LIST_HEAD(device_list);

/**
 * @brief This function finds a device driver by specified name.
 *
 * @param name is the device driver's name.
 *
 * @return the registered device driver on successful, or NULL on failure.
 */
device_t device_find(const char *name)
{
	device_t ptr;

	list_for_each_entry(ptr, &device_list, node, struct device)
	{
		if (strncmp(name, ptr->name, NAME_MAX) == 0)
			return ptr;
	}

	return NULL;
}

/**
 * @brief This function finds a device driver by type.
 *
 * @param type is the device's type.
 *
 * @return the registered device driver on successful, or NULL on failure.
 */
device_t match_device(device_class_type type, uint8_t id)
{
    device_t ptr;

    if (type < Device_Class_Char || type >= Device_Class_Unknown) {
        return NULL;
    }

	list_for_each_entry(ptr, &device_list, node, struct device) {
		if ((type == ptr->type) && (id == ptr->device_id)) {
            return ptr;
        }
	}

	return NULL;
}

/**
 * @brief This function registers a device driver with a specified name.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param name is the device driver's name.
 *
 * @param flags is the capabilities flag of device.
 *
 * @return the error code, EOK on initialization successfully.
 */
f_err_t device_register(device_t dev,
                      const char *name,
                      uint16_t flags)
{
    if (dev == NULL)
        return ERROR;

    if (device_find(name) != NULL)
        return ERROR;

    strncpy(dev->name, name, NAME_MAX);
    dev->flag = flags;
    dev->ref_count = 0;
    dev->open_flag = 0;

    list_add(&dev->node, &device_list);

    return EOK;
}

/**
 * @brief This function removes a previously registered device driver.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @return the error code, EOK on successfully.
 */
f_err_t device_unregister(device_t dev)
{
    /* parameter check */
    assert(dev != NULL);

    list_del(&dev->node);

    return EOK;
}

/**
 * @brief This function will initialize the specified device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @return the result, EOK on successfully.
 */
f_err_t device_init(device_t dev)
{
	f_err_t result = EOK;

    assert(dev != NULL);

    lock(dev->spinlock);
    /* get device_init handler */
    if (dev->ops->init != NULL) {
        if (!(dev->flag & DEVICE_FLAG_ACTIVATED)) {
            result = dev->ops->init(dev);
            if (result != EOK) {
                return ERROR;
            }
            else {
                dev->flag |= DEVICE_FLAG_ACTIVATED;
            }
        }
    }
    unlock(dev->spinlock);

    return result;
}

/**
 * @brief This function will open a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param oflag is the flags for device open.
 *
 * @return the result, EOK on successfully.
 */
f_err_t device_open(device_t dev, uint16_t oflag)
{
	f_err_t result = EOK;

    /* parameter check */
    assert(dev != NULL);

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & DEVICE_FLAG_ACTIVATED))
    {
        if (dev->ops->init != NULL)
        {
            result = dev->ops->init(dev);
            if (result !=  EOK)
            {
                printf("To initialize device:%.*s failed. The error code is %ld",
                      NAME_MAX, dev->name, result);

                return result;
            }
        }

        dev->flag |= DEVICE_FLAG_ACTIVATED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & DEVICE_FLAG_STANDALONE) &&
        (dev->open_flag & DEVICE_OFLAG_OPEN))
    {
        return -EBUSY;
    }

    /* device is not opened or opened by other oflag, call device_open interface */
    if (!(dev->open_flag & DEVICE_OFLAG_OPEN) ||
         ((dev->open_flag & DEVICE_OFLAG_MASK) != ((oflag & DEVICE_OFLAG_MASK) | DEVICE_OFLAG_OPEN)))
    {
        if (dev->ops->open != NULL)
        {
            result = dev->ops->open(dev, oflag);
        }
        else
        {
            /* set open flag */
            dev->open_flag = (oflag & DEVICE_OFLAG_MASK);
        }
    }

    /* set open flag */
    if (result == EOK || result == ENOSYS)
    {
        dev->open_flag |= DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        /* don't let bad things happen silently. If you are bitten by this assert,
        * please set the ref_count to a bigger type. */
        assert(dev->ref_count != 0);
    }

    return result;
}

/**
 * @brief This function will close a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @return the result, EOK on successfully.
 */
f_err_t device_close(device_t dev)
{
	f_err_t result = EOK;

    /* parameter check */
    assert(dev != NULL);

    if (dev->ref_count == 0)
        return -ERROR;

    dev->ref_count--;

    if (dev->ref_count != 0)
        return EOK;

    /* call device_close interface */
    if (dev->ops->close != NULL)
    {
        result = dev->ops->close(dev);
    }

    /* set open flag */
    if (result == EOK || result == -ENOSYS)
        dev->open_flag = DEVICE_OFLAG_CLOSE;

    return result;
}

/**
 * @brief This function will read some data from a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param pos is the position when reading.
 *
 * @param buffer is a data buffer to save the read data.
 *
 * @param size is the size of buffer.
 *
 * @return the actually read size on successful, otherwise 0 will be returned.
 *
 * @note the unit of size/pos is a block for block device.
 */
f_err_t device_read(device_t dev,
                   off_t    pos,
                   void     *buffer,
                   size_t   size)
{
    /* parameter check */
    assert(dev != NULL);

    if (dev->ref_count == 0)
        return -ERROR;

    /* call device_read interface */
    if (dev->ops->read != NULL)
    {
        return dev->ops->read(dev, pos, buffer, size);
    }

    return -ENOSYS;
}

/**
 * @brief This function will write some data to a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param pos is the position when writing.
 *
 * @param buffer is the data buffer to be written to device.
 *
 * @param size is the size of buffer.
 *
 * @return the actually written size on successful, otherwise 0 will be returned.
 *
 * @note the unit of size/pos is a block for block device.
 */
f_err_t device_write(device_t   dev,
                   off_t      pos,
                   const void *buffer,
                   size_t     size)
{
    /* parameter check */
    assert(dev != NULL);

    if (dev->ref_count == 0)
    {
        return -ERROR;
    }

    /* call device_write interface */
    if (dev->ops->write != NULL)
    {
        return dev->ops->write(dev, pos, buffer, size);
    }

    return -ENOSYS;
}

/**
 * @brief This function will perform a variety of control functions on devices.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param cmd is the command sent to device.
 *
 * @param arg is the argument of command.
 *
 * @return the result, -ENOSYS for failed.
 */
f_err_t device_control(device_t dev, int cmd, void *arg)
{
    /* parameter check */
    assert(dev != NULL);

    /* call device_write interface */
    if (dev->ops->control != NULL)
    {
        return dev->ops->control(dev, cmd, arg);
    }

    return -ENOSYS;
}

/**
 * @brief This function will set the reception indication callback function. This callback function
 *        is invoked when this device receives data.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param rx_ind is the indication callback function.
 *
 * @return EOK
 */
f_err_t device_set_rx_indicate(device_t dev,
                             f_err_t (*rx_ind)(device_t dev,
                             void *buffer))
{
    /* parameter check */
    assert(dev != NULL);

    dev->rx_indicate = rx_ind;

    return EOK;
}

/**
 * @brief This function will set a callback function. The callback function
 *        will be called when device has written data to physical hardware.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param tx_done is the indication callback function.
 *
 * @return EOK
 */
f_err_t device_set_tx_complete(device_t dev,
                             f_err_t (*tx_done)(device_t dev,
                             void *buffer))
{
    /* parameter check */
    assert(dev != NULL);

    dev->tx_complete = tx_done;

    return EOK;
}
