/*
 * Copyright (c) 2025, Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-16     Alex_min	   first version
 */
#ifndef __BOARD_H_
#define __BOARD_H_

#include "tc387_def.h"
#include "IfxCpu.h"

void lock(spinLock *spinlock);
void unlock(spinLock *spinlock);
void wait_for_core0_done(void);
void set_core0_status(void);

#endif /* __BOARD_H_ */
