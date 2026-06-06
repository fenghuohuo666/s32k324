#ifndef IFXASCLIN_LIN_H
#define IFXASCLIN_LIN_H

#include <stdint.h>
#include "Ifx_Types.h"

/* Stub types for Infineon ASCLIN LIN driver */
typedef struct {
    uint32 dummy;
} IfxAsclin_Lin;

typedef struct {
    uint32 dummy;
} IfxAsclin_Lin_Config;

static inline void IfxAsclin_Lin_initModuleConfig(IfxAsclin_Lin_Config *config, void *asclin)
{
    (void)config;
    (void)asclin;
}

static inline void IfxAsclin_Lin_initModule(IfxAsclin_Lin *driver, IfxAsclin_Lin_Config *config)
{
    (void)driver;
    (void)config;
}

static inline void IfxAsclin_Lin_initChannelConfig(IfxAsclin_Lin_Config *config, IfxAsclin_Lin *driver)
{
    (void)config;
    (void)driver;
}

static inline void IfxAsclin_Lin_initChannel(IfxAsclin_Lin *driver, IfxAsclin_Lin_Config *config)
{
    (void)driver;
    (void)config;
}

static inline void IfxAsclin_Lin_sendResponse(IfxAsclin_Lin *driver, uint8 *data, uint32 length)
{
    (void)driver;
    (void)data;
    (void)length;
}

static inline void IfxAsclin_Lin_receiveResponse(IfxAsclin_Lin *driver, uint8 *data, uint32 length)
{
    (void)driver;
    (void)data;
    (void)length;
}

static inline void IfxAsclin_Lin_sendHeader(IfxAsclin_Lin *driver, uint8 pid)
{
    (void)driver;
    (void)pid;
}

static inline boolean IfxAsclin_Lin_isHeaderTransmitted(IfxAsclin_Lin *driver)
{
    (void)driver;
    return TRUE;
}

static inline boolean IfxAsclin_Lin_isResponseTransmitted(IfxAsclin_Lin *driver)
{
    (void)driver;
    return TRUE;
}

static inline boolean IfxAsclin_Lin_isResponseReceived(IfxAsclin_Lin *driver)
{
    (void)driver;
    return TRUE;
}

static inline boolean IfxAsclin_Lin_hasHeaderReceived(IfxAsclin_Lin *driver)
{
    (void)driver;
    return FALSE;
}

static inline uint8 IfxAsclin_Lin_getHeaderReceivedPid(IfxAsclin_Lin *driver)
{
    (void)driver;
    return 0;
}

static inline void IfxAsclin_Lin_clearFlags(IfxAsclin_Lin *driver)
{
    (void)driver;
}

static inline void IfxAsclin_Lin_enableErrorFlags(IfxAsclin_Lin *driver)
{
    (void)driver;
}

#endif /* IFXASCLIN_LIN_H */
