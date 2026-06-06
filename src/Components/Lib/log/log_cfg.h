/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Alex_min	   first version
 */

#ifndef __LOG_CFG_H__
#define __LOG_CFG_H__

// Log module IDs
typedef enum {
    LOG_MODULE_UART = 0,
    LOG_MODULE_CAN,
    LOG_MODULE_LIN,
    LOG_MODULE_SCHEDULE,
    LOG_MODULE_TX_TASK,
    LOG_MODULE_RX_TASK,
    LOG_MODULE_LAN7801,
    LOG_MODULE_TIMESTAMP,
    LOG_MODULE_FLEXRAY,
    LOG_MODULE_OTA,
    LOG_MODULE_COUNT
} log_module_t;

// Log levels
typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} log_level_t;

#endif /* __LOG_CFG_H__ */
