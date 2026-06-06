#ifndef IFXCPU_H
#define IFXCPU_H

#include "Ifx_Types.h"
#include "tc387_def.h"

/* Spinlock type from Infineon - alias to spinLock for S32K324 */
typedef spinLock IfxCpu_spinLock;

/* Spinlock is now just a volatile uint32_t */
static inline boolean IfxCpu_setSpinLock(spinLock *lock, uint32 timeout)
{
    (void)timeout;
    *lock = 1;
    return TRUE;
}

static inline void IfxCpu_resetSpinLock(spinLock *lock)
{
    *lock = 0;
}

static inline void IfxCpu_enableInterrupts(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

static inline boolean IfxCpu_disableInterrupts(void)
{
    __asm volatile ("cpsid i" ::: "memory");
    return TRUE;
}

static inline void IfxCpu_restoreInterrupts(boolean enabled)
{
    if (enabled) __asm volatile ("cpsie i" ::: "memory");
    else __asm volatile ("cpsid i" ::: "memory");
}

static inline boolean IfxCpu_areInterruptsEnabled(void)
{
    return TRUE;
}

static inline void IfxCpu_forceDisableInterrupts(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}

typedef unsigned int IfxCpu_syncEvent;

static inline void IfxCpu_emitEvent(volatile IfxCpu_syncEvent *event)
{
    *event = 1;
}

static inline boolean IfxCpu_waitEvent(volatile IfxCpu_syncEvent *event, uint32 timeoutMilliSec)
{
    (void)timeoutMilliSec;
    while (*event == 0) {}
    return TRUE;
}

/* Atomic bitfield insert - Infineon intrinsic stub */
void Ifx__imaskldmst(volatile IfxCpu_syncEvent *address, uint32 value, uint32 offset, uint32 count);

#endif /* IFXCPU_H */
