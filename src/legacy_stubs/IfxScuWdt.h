#ifndef IFXSCUWDT_H
#define IFXSCUWDT_H

#include <stdint.h>
#include <stdbool.h>

static inline uint16 IfxScuWdt_getCpuWatchdogPassword(void)
{
    return 0;
}

static inline void IfxScuWdt_disableCpuWatchdog(uint16 password)
{
    (void)password;
}

static inline uint16 IfxScuWdt_getSafetyWatchdogPassword(void)
{
    return 0;
}

static inline uint16 IfxScuWdt_getSafetyWatchdogPasswordInline(void)
{
    return 0;
}

static inline void IfxScuWdt_disableSafetyWatchdog(uint16 password)
{
    (void)password;
}

static inline void IfxScuWdt_clearSafetyEndinit(uint16 password)
{
    (void)password;
}

static inline void IfxScuWdt_clearSafetyEndinitInline(uint16 password)
{
    (void)password;
}

static inline void IfxScuWdt_setSafetyEndinit(uint16 password)
{
    (void)password;
}

static inline void IfxScuWdt_setSafetyEndinitInline(uint16 password)
{
    (void)password;
}

static inline void IfxScuWdt_clearCpuEndinit(uint16 password)
{
    (void)password;
}

static inline void IfxScuWdt_setCpuEndinit(uint16 password)
{
    (void)password;
}

#endif /* IFXSCUWDT_H */
