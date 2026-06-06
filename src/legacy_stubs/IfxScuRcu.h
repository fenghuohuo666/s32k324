#ifndef IFXSCURCU_H
#define IFXSCURCU_H

#include <stdint.h>

typedef enum {
    IfxScuRcu_Trigger_sw = 0
} IfxScuRcu_Trigger;

typedef enum {
    IfxScuRcu_ResetType_system = 0
} IfxScuRcu_ResetType;

static inline void IfxScuRcu_configureResetRequestTrigger(IfxScuRcu_Trigger trigger, IfxScuRcu_ResetType type)
{
    (void)trigger;
    (void)type;
}

static inline void IfxCpu_triggerSwReset(void)
{
    /* S32K324: implement software reset if needed */
}

#endif /* IFXSCURCU_H */
