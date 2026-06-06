/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Wyj          Demo
 */

#ifndef __CAN_INTERFACE_H_
#define __CAN_INTERFACE_H_

#include <stdio.h>
#include <string.h>
#include "Ifx_Types.h"
#include "IfxCan_Can.h"
#include "IfxCan.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"
#include "IfxStm.h"
#include "Platform_Types.h"
#include "device.h"
/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

#define RX_INTERRUPT_SRC_ID IfxMultican_SrcId_1 /* RX interrupt service request ID                   */
#define INVALID_TX_DATA_VALUE (uint8)0x00       /* Used to invalidate TX message data content        */
#define INVALID_RX_DATA_VALUE (uint8)0x00       /* Used to invalidate RX message data content        */


#define MAXIMUM_CAN_FD_DATA_PAYLOAD 64 /* Define maximum CANFD payload in bytes               */
#define MAXIMUM_CAN_DATA_PAYLOAD    8 /* Define maximum CAN payload in bytes               */
#define max_BufferLength  0x5000
#define G_DLC_LOOKUP_TABLE(index) ((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64}[(index)])
#define CANFD_BRS              0x01
#define CANFD_ESI              0x02
#define CANFD_FDF              0x04

#define PROTOCOL_VERSION 0x01
#define PROTOCOL_DEVICEID 0x01
#define RCPMESSAGE_HEAD_SIZE 20
/*********************************************************************************************************************/
/*--------------------------------------------------Data Structures--------------------------------------------------*/
/*********************************************************************************************************************/
typedef struct
{
    IfxCan_Can_Config canConfig;               /* CAN module configuration structure                        */
    IfxCan_Can canModule;                      /* CAN module handle                                         */
    IfxCan_Can_Node canNode;                   /* CAN Node                                                   */
    IfxCan_Can_NodeConfig canNodeConfig;       /* CAN node configuration structure                          */
    IfxCan_Filter canFilter;                   /* CAN filter configuration structure                        */
    IfxCan_Message txMsg;                      /* Transmitted CAN message structure                         */
    IfxCan_Message rxMsg;                      /* Received CAN message structure                            */
    uint8 txData[MAXIMUM_CAN_FD_DATA_PAYLOAD]; /* Transmitted CAN data array                                */
    uint8 rxData[MAXIMUM_CAN_FD_DATA_PAYLOAD]; /* Received CAN data array                                   */
} CanInstance_Type;

typedef struct {
    uint8_t ch;
    uint8_t fd;
    IfxCan_Message Msg;
    uint8_t data[MAXIMUM_CAN_FD_DATA_PAYLOAD];
} CAN_Frame_t;

typedef struct {
    uint8_t open;
    uint8_t is_can_fd;
    uint16_t pad;
    uint32_t bitrate;
    uint32_t sample_point;
    uint32_t data_bitrate;
    uint32_t data_sample_point;
} CAN_Config_t;

/*********************************************************************************************************************/
/*-----------------------------------------------Function Prototypes-------------------------------------------------*/
/*********************************************************************************************************************/
void init_Mcmcan_module(void);
void init_CAN_PHY(void);
void can_sleep_control(void);
IfxCan_Can_NodeConfig configCanNode(uint8 node_channel, CAN_Config_t *node_config);
boolean switchCanNode(uint8 can_channel,IfxCan_Can_NodeConfig node_config);
uint32 transmitMsg(uint8 channel, CAN_Frame_t *frame);
//RcpMessage *build_canframe(uint8_t channel, CanInstance_Type *recv_can_msg, bool is_fd);

#endif /* __CAN_INTERFACE_H_ */
