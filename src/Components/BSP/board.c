/*
 * Copyright (c) 2025, Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-16     Alex_min	   first version
 */

#include "board.h"
uint8 core0_status = 0;

void lock(spinLock *lock)
{
#ifdef TC3XX
    while (!IfxCpu_setSpinLock(lock, 0xffff)) {};
#else

#endif
}

void unlock(spinLock *lock)
{
#ifdef TC3XX
    IfxCpu_resetSpinLock(lock);
#else

#endif  
}

void wait_for_core0_done(void)
{
    while (!core0_status) {}
}

void set_core0_status(void)
{
    core0_status = 1;
}
