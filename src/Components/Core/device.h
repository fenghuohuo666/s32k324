/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-08     Alex_min     first version
 */

#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "def.h"

/*
 * device (I/O) system interface
 */
device_t device_find(const char *name);

device_t match_device(device_class_type type, uint8_t id);

f_err_t device_register(device_t dev,
                      const char *name,
                      uint16_t flags);
f_err_t device_unregister(device_t dev);

f_err_t
device_set_rx_indicate(device_t dev,
                       f_err_t (*rx_ind)(device_t dev, void *buffer));
f_err_t
device_set_tx_complete(device_t dev,
                       f_err_t (*tx_done)(device_t dev, void *buffer));

f_err_t  device_init   (device_t dev);
f_err_t  device_open   (device_t dev, uint16_t oflag);
f_err_t  device_close  (device_t dev);
f_err_t device_read    (device_t dev,
                      off_t    pos,
                      void     *buffer,
                      size_t   size);
f_err_t device_write   (device_t dev,
                      off_t    pos,
                      const void *buffer,
                      size_t   size);
f_err_t  device_control(device_t dev, int cmd, void *arg);
#endif /* __DEVICE_H_ */
