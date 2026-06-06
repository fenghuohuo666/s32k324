#ifndef IFXSTM_H
#define IFXSTM_H

#include "Ifx_Types.h"

/* Stub for STM timer functions */
typedef void* Ifx_STM;

#define MODULE_STM0 ((Ifx_STM*)0)
#define MODULE_STM1 ((Ifx_STM*)0)
#define MODULE_STM2 ((Ifx_STM*)0)
#define MODULE_STM3 ((Ifx_STM*)0)

static inline Ifx_TickTime IfxStm_getTicksFromMilliseconds(Ifx_STM *stm, uint32 ms)
{
    (void)stm;
    return (Ifx_TickTime)(ms * 100000);
}

static inline Ifx_TickTime IfxStm_getTicksFromMicroseconds(Ifx_STM *stm, uint32 us)
{
    (void)stm;
    return (Ifx_TickTime)(us * 100);
}

static inline uint64 IfxStm_get(Ifx_STM *stm)
{
    (void)stm;
    return 0;
}

static inline void IfxStm_clearCompareFlag(Ifx_STM *stm, IfxStm_CompareConfig *config)
{
    (void)stm;
    (void)config;
}

static inline void IfxStm_increaseCompare(Ifx_STM *stm, IfxStm_CompareConfig *config, uint32 ticks)
{
    (void)stm;
    (void)config;
    (void)ticks;
}

static inline void IfxStm_initCompareConfig(IfxStm_CompareConfig *config)
{
    (void)config;
}

static inline void IfxStm_initCompare(Ifx_STM *stm, IfxStm_CompareConfig *config)
{
    (void)stm;
    (void)config;
}

#endif /* IFXSTM_H */
