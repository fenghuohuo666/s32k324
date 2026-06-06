/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Alex_min	   first version
 */
#include <stdio.h>
#include <string.h>
#include "log.h"

static const char *log_level_str[] = {
	" NONE",
	"ERROR",
	" WARN",
	" INFO",
	"DEBUG"
};

static const char *log_level_color[] = {
    "",
    LOG_COLOR_RED,
    LOG_COLOR_YELLOW,
    LOG_COLOR_GREEN,
    LOG_COLOR_CYAN
};

static const char *log_module_str[] = {
    "UART", "CAN", "LIN", "SCHEDULE", "TX_TASK",
    "RX_TASK","LAN7801","TIMESTAMP", "FLEXRAY", "OTA"
};

static log_level_t g_log_level_table[LOG_MODULE_COUNT] = {
    [LOG_MODULE_UART]       = LOG_LEVEL_NONE,
    [LOG_MODULE_CAN]        = LOG_LEVEL_NONE,
    [LOG_MODULE_LIN]        = LOG_LEVEL_NONE,
    [LOG_MODULE_SCHEDULE]   = LOG_LEVEL_DEBUG,  /* 验证日志通路 */
    [LOG_MODULE_TX_TASK]    = LOG_LEVEL_NONE,
    [LOG_MODULE_RX_TASK]    = LOG_LEVEL_NONE,
    [LOG_MODULE_LAN7801]    = LOG_LEVEL_NONE,
    [LOG_MODULE_TIMESTAMP]  = LOG_LEVEL_NONE,
    [LOG_MODULE_FLEXRAY]    = LOG_LEVEL_NONE,
    [LOG_MODULE_OTA]        = LOG_LEVEL_NONE,
};

/* 日志输出验证计数器（纯软件，确认 log_output 实际走到了 printf） */
volatile uint32_t log_output_cnt = 0;

void log_output(log_module_t module, log_level_t level, const char *fmt, ...)
{
    if (module >= LOG_MODULE_COUNT || level > g_log_level_table[module])
        return;

    log_output_cnt++;

    const char *color = log_level_color[level];
    const char *reset = LOG_COLOR_RESET;

    printf("%s[%5s][%s]%s ", color, log_level_str[level], log_module_str[module], reset);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
