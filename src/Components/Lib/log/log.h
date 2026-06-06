/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Alex_min	   first version
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include "log_cfg.h"

#define LOG_COLOR_RED     "\x1b[31m"
#define LOG_COLOR_YELLOW  "\x1b[33m"
#define LOG_COLOR_GREEN   "\x1b[32m"
#define LOG_COLOR_CYAN    "\x1b[36m"
#define LOG_COLOR_RESET   "\x1b[0m"


#define LOG_ERROR(mod, ...) log_output(mod, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WARN(mod, ...)  log_output(mod, LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_INFO(mod, ...)  log_output(mod, LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_DEBUG(mod, ...) log_output(mod, LOG_LEVEL_DEBUG, __VA_ARGS__)

void log_output(log_module_t module, log_level_t level, const char *fmt, ...);

#endif /* __LOG_H__ */
