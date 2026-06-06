/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-14     Wenjc        Demo
 */

#ifndef __CLOCK_H_
#define __CLOCK_H_

#include <stdint.h>
#include "IfxStm.h"
#include "Ifx_Types.h"

#include "device.h"
#include "Ifx_Types.h"
#include "IfxSrc.h"
#include "IfxScuEru.h"
#include "ringbuffer.h"

#define DEFAULT_TIMER       (&MODULE_STM0)
#define MS_PER_SEC          (1000)
#define US_PER_SEC          (1000 * 1000)

#define ticksFor1us IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, 1)
#define ticksFor1ms IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, MS_PER_SEC)
#define ticksFor1s  IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, US_PER_SEC)

#define CLOCK_RING_DEPTH                    20
#define REQ_IN                              &IfxScu_REQ3C_P02_0_IN   /* External request pin                                 */
#define TRIGGER_PIN                         &MODULE_P02,1            /* Pin which can be controlled via debugger
                                                               to trigger interrupt                                 */
#define LED                                 &MODULE_P33,4            /* LED which gets toggled in Interrupt Service Routine  */

#define PROTOCOL_VERSION                    0x01
#define PROTOCOL_DEVICEID                   0x01
#define RCPMESSAGE_HEAD_SIZE                20

typedef struct
{
        IfxScu_Req_In *reqPin;                      /* External request pin                                             */
        IfxScuEru_InputChannel inputChannel;        /* Input channel EICRm depending on input pin                       */
        IfxScuEru_InputNodePointer triggerSelect;   /* Input node pointer                                               */
        IfxScuEru_OutputChannel outputChannel;      /* Output channel                                                   */
        volatile Ifx_SRC_SRCR *src;                 /* Service request register                                         */
} ERUconfig;

struct clock_device
{
        const char *name;
        uint8_t channel;                                // Physical channel number (0-11)
        uint8_t ifindex;
        uint32_t flags;
        struct device dev;
        uint64_t t1;          // The first interruption timestamp
        uint64_t remote_time; // The remote time sent by TDA4
};

void initPeripheralsAndERU(void);
void hw_clock_init(void);
uint64_t get_sync_timestamp(void);
device_t ifindex_find_hwclock(uint8_t ifindex);

#endif /* __CLOCK_H_ */
