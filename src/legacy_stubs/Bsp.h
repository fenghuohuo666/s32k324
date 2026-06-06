#ifndef BSP_H
#define BSP_H

#include "Ifx_Types.h"

/* Minimal Bsp stub - only waitTime is actually used in schedule.c */
static inline void waitTime(Ifx_TickTime timeout)
{
    (void)timeout;
    volatile uint32_t i;
    for (i = 0; i < 1000; i++) {}
}

static inline void waitPoll(void)
{
}

#endif /* BSP_H */
