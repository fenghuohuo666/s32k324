/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-29     Alex_min     first version
 */

#ifndef __RECEIVE_TASH_H_
#define __RECEIVE_TASH_H_

#define RECEIVE_BUFF_DEPTH             20

struct ringbuffer *get_rx_ring(void);
struct ringbuffer *creat_rx_ring(void);
int receive_task(void);

#endif /* __RECEIVE_TASH_H_ */
