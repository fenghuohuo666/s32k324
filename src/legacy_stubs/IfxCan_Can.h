#ifndef IFXCAN_CAN_H
#define IFXCAN_CAN_H

#include <stdint.h>
#include <stdbool.h>
#include "Ifx_Types.h"
#include "IfxCan.h"

/* Stub types for Infineon CAN iLLD */
typedef struct { uint32 dummy; } IfxCan_Can;
typedef struct { uint32 dummy; } IfxCan_Can_Node;
typedef struct { uint32 dummy; } IfxCan_Can_Config;

static inline void IfxCan_Can_initModuleConfig(IfxCan_Can_Config *config, void *can)
{
    (void)config;
    (void)can;
}

static inline IfxCan_Status IfxCan_Can_initModule(IfxCan_Can *driver, const IfxCan_Can_Config *config)
{
    (void)driver;
    (void)config;
    return IfxCan_Status_ok;
}

static inline void IfxCan_Can_initNodeConfig(IfxCan_Can_NodeConfig *config, const IfxCan_Can *driver)
{
    (void)config;
    (void)driver;
}

static inline IfxCan_Status IfxCan_Can_initNode(IfxCan_Can_Node *node, const IfxCan_Can_NodeConfig *config)
{
    (void)node;
    (void)config;
    return IfxCan_Status_ok;
}

static inline IfxCan_Status IfxCan_Can_sendMessage(IfxCan_Can_Node *node, const void *message)
{
    (void)node;
    (void)message;
    return IfxCan_Status_ok;
}

static inline IfxCan_Status IfxCan_Can_readMessage(IfxCan_Can_Node *node, void *message)
{
    (void)node;
    (void)message;
    return IfxCan_Status_ok;
}

static inline bool IfxCan_Can_isMessageAvailable(IfxCan_Can_Node *node)
{
    (void)node;
    return false;
}

#endif /* IFXCAN_CAN_H */
