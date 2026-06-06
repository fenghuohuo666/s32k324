#ifndef IFXASCLIN_H
#define IFXASCLIN_H

#include <stdint.h>
#include "Ifx_Types.h"

/* Minimal stub for Infineon ASCLIN (UART) module */
typedef struct { uint32 dummy; } Ifx_ASCLIN;

typedef enum {
    IfxAsclin_DataLength_8 = 8
} IfxAsclin_DataLength;

typedef enum {
    IfxAsclin_ParityMode_no = 0,
    IfxAsclin_ParityMode_even
} IfxAsclin_ParityMode;

typedef enum {
    IfxAsclin_StopBit_1 = 1
} IfxAsclin_StopBit;

typedef enum {
    IfxAsclin_OversamplingFactor_4 = 4
} IfxAsclin_OversamplingFactor;

typedef enum {
    IfxAsclin_SamplePointPosition_3 = 3
} IfxAsclin_SamplePointPosition;

typedef enum {
    IfxAsclin_LoopBackMode_disable = 0
} IfxAsclin_LoopBackMode;

static inline void IfxAsclin_setClockSource(void *asclin, uint32 source)
{
    (void)asclin;
    (void)source;
}

static inline void IfxAsclin_setPrescaler(void *asclin, uint16 prescaler)
{
    (void)asclin;
    (void)prescaler;
}

static inline void IfxAsclin_setBaudrateBitFields(void *asclin, uint32 oversampling, uint32 samplePoint)
{
    (void)asclin;
    (void)oversampling;
    (void)samplePoint;
}

static inline void IfxAsclin_enableLoopBackMode(void *asclin, uint32 mode)
{
    (void)asclin;
    (void)mode;
}

static inline void IfxAsclin_initTxPin(void *pin, void *mode, void *driver)
{
    (void)pin;
    (void)mode;
    (void)driver;
}

static inline void IfxAsclin_initRxPin(void *pin, void *mode, void *driver)
{
    (void)pin;
    (void)mode;
    (void)driver;
}

static inline void IfxAsclin_setDataLength(void *asclin, uint32 length)
{
    (void)asclin;
    (void)length;
}

static inline void IfxAsclin_setParityMode(void *asclin, uint32 mode)
{
    (void)asclin;
    (void)mode;
}

static inline void IfxAsclin_setStopBit(void *asclin, uint32 stopBit)
{
    (void)asclin;
    (void)stopBit;
}

static inline void IfxAsclin_enableParityErrorFlag(void *asclin, uint32 enable)
{
    (void)asclin;
    (void)enable;
}

static inline void IfxAsclin_enableFrameErrorFlag(void *asclin, uint32 enable)
{
    (void)asclin;
    (void)enable;
}

#endif /* IFXASCLIN_H */
