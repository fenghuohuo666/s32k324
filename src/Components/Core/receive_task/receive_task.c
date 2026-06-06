/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-29     Alex_min     first version
 */
#include <string.h>

#include "def.h"
#include "receive_task.h"
#include "ringbuffer.h"
#include "device.h"
#include "dispose_rcp.h"
#include "tc387_def.h"

struct ringbuffer *rx_buf;
#if (USE_CPU2_LOAD)
extern volatile uint64 core2_idle_cnt;
#endif

/* receive_task 入口验证计数器（纯软件，不涉及硬件操作） */
volatile uint32_t receive_task_entry_cnt = 0;

struct ringbuffer *get_rx_ring(void)
{
    return rx_buf;
}

struct ringbuffer *creat_rx_ring(void)
{
    rx_buf = ringbuffer_create(RECEIVE_BUFF_DEPTH);

    return rx_buf;
}

int receive_task(void)
{
    int loop;

    receive_task_entry_cnt++;

    for (loop = 0; loop < 10; loop++)
    {
#if (USE_CPU2_LOAD)
        core2_idle_cnt++;
#endif
    }

	return 0;
}
