/*
 * Copyright (c) 2026, Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2026-02-4      Alex.min      TC387 mac dirver, bind LAN7801
 */

#ifndef __NET_H_
#define __NET_H_
#include "tc387_def.h"
#include "def.h"

#ifdef TC3XX
    /* Stub: Infineon GTM timer not available on S32K324 */
    #define GENERAL_TIMER               ((void*)0)
    #define ATOM_FREQ                   (1.0f)
    #define CMU_FREQ                    (1000000.0f)
    #define TIMER_INT_TIME              (100)
#endif

#define USE_UDP
#define DATA_MAX_LEN						1024
#define NET_RING_DEPTH                      100

struct net_device
{
    const char *name;
    uint8_t  channel;
    device_t dev;
};

void hw_net_init(void);
device_t hw_net_find(void);
#endif /* __NET_H_ */
