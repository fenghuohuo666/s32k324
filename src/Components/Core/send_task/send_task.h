/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-29     Alex_min     first version
 */

#ifndef __SEND_TASH_H_
#define __SEND_TASH_H_

#include "ringbuffer.h"
#include "IfxCpu.h"

#define SEND_BUFF_DEPTH             20
#define ITEM_PER_WORD               32

extern IfxCpu_spinLock udp_spinLock;

struct ringbuffer *get_tx_ring(void);
struct ringbuffer *creat_tx_ring(void);
int send_task(void);
int core2_send_task(void);

#endif /* __SEND_TASH_H_ */
