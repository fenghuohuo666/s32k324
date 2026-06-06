/*
 * Copyright (c) 2025, Grce Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-07-10     Wenjc     AscLin_Driver
 */

#ifndef __ASCLIN_H_
#define __ASCLIN_H_

#include "device.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "IfxAsclin_PinMap.h"
#include "IfxAsclin_Lin.h"
#include "IfxAsclin.h"
#include "IfxSrc_regdef.h"
#include "list.h"
#include "err.h"
#include "log.h"
#include "Ifx_Assert.h"
#include "Ifx_Types.h"
#include "CompilerTasking.h"
#include "pb_type.h"
#include "tc387_response.h"
#include "ringbuffer.h"
#include "rcp_build.h"


#define LIN_CHANNEL_NUM                 16
#define LIN_STORE_FRAME_NUM             40
#define DEVICE_NAME_MAX                 8
#define LIN_MESSAGE_BUFFER_SIZE         0x5000 /* 20k bytes */
#define LIN_MAX_MESSAGE_LENGTH          8
#define LIN_MSG_MAX_NUMS (LIN_MESSAGE_BUFFER_SIZE / sizeof(LinMsg_Type))
#define PROTOCOL_VERSION                0x01
#define PROTOCOL_DEVICEID               0x01
#define RCPMESSAGE_HEAD_SIZE            20

/**
 * 引脚定义
 */
#define LIN_0_RX_PIN IfxAsclin0_RXB_P15_3_IN
#define LIN_0_TX_PIN IfxAsclin0_TX_P15_2_OUT

#define LIN_1_RX_PIN IfxAsclin1_RXA_P15_1_IN
#define LIN_1_TX_PIN IfxAsclin1_TX_P15_0_OUT

#define LIN_2_RX_PIN IfxAsclin2_RXC_P02_10_IN
#define LIN_2_TX_PIN IfxAsclin2_TX_P02_9_OUT

#define LIN_3_RX_PIN IfxAsclin3_RXA_P15_7_IN
#define LIN_3_TX_PIN IfxAsclin3_TX_P15_6_OUT

#define LIN_4_RX_PIN IfxAsclin4_RXA_P00_12_IN
#define LIN_4_TX_PIN IfxAsclin4_TX_P00_9_OUT

#define LIN_5_RX_PIN IfxAsclin5_RXA_P00_6_IN
#define LIN_5_TX_PIN IfxAsclin5_TX_P00_7_OUT

#define LIN_6_RX_PIN IfxAsclin6_RXA_P23_3_IN
#define LIN_6_TX_PIN IfxAsclin6_TX_P23_5_OUT

#define LIN_7_RX_PIN IfxAsclin7_RXF_P22_4_IN
#define LIN_7_TX_PIN IfxAsclin7_TX_P22_1_OUT

#define LIN_8_RX_PIN IfxAsclin8_RXD_P33_6_IN
#define LIN_8_TX_PIN IfxAsclin8_TX_P33_7_OUT

#define LIN_9_RX_PIN IfxAsclin9_RXA_P01_5_IN
#define LIN_9_TX_PIN IfxAsclin9_TX_P01_7_OUT

#define LIN_10_RX_PIN IfxAsclin10_RXD_P13_1_IN
#define LIN_10_TX_PIN IfxAsclin10_TX_P13_0_OUT

#define LIN_11_RX_PIN IfxAsclin11_RXD_P21_1_IN
#define LIN_11_TX_PIN IfxAsclin11_TX_P21_0_OUT

#define LIN_13_RX_PIN IfxAsclin13_RXB_P00_11_IN
#define LIN_13_TX_PIN IfxAsclin13_TX_P00_10_OUT

#define LIN_14_RX_PIN IfxAsclin14_RXA_P33_3_IN
#define LIN_14_TX_PIN IfxAsclin14_TX_P33_2_OUT

#define LIN_15_RX_PIN IfxAsclin15_RXA_P32_4_IN
#define LIN_15_TX_PIN IfxAsclin15_TX_P32_7_OUT

#define LIN_16_RX_PIN IfxAsclin16_RXB_P23_7_IN
#define LIN_16_TX_PIN IfxAsclin16_TX_P23_6_OUT

typedef enum
{
    LIN_FRAME_EMPTY,
    LIN_FRAME_FULL,
    /* half full is neither full nor empty */
    LIN_FRAME_HALFFULL,
} lin_frame_ringbuf_state_e;

typedef struct {
    uint8_t read_mirror : 1;
    uint8_t read_index  : 7;
    uint8_t write_mirror: 1;
    uint8_t write_index : 7;
} LIN_Frame_index_t;

/* Equipment control structure */
typedef struct {
    const char *name;
    uint8_t channel;                                // Physical channel number (0-11)
    uint8_t ifindex;
    uint32_t flags;
    /* Status flag */
    boolean schedule_enabled;                       // Scheduling table enable
    /* Hardware-related */
    IfxAsclin_Lin handle;                           // Infineon drive handle
    IfxAsclin_Lin_Config config;                    // Hardware configuration
    struct device dev;
} AscLin_Device;

typedef enum {
    LinDirection_t_Master_TransmitHeader_UDP = 0,
    LinDirection_t_Master_TransmitHeaderAndResponse_UDP = 1,
    LinDirection_t_Master_TransmitHeaderAndReceiveResponse_UDP = 2,
    LinDirection_t_Slave_TransmitResponse_UDP = 3,
    LinDirection_t_Slave_ReceiveResponse_UDP = 4,
    LinDirection_t_Slave_Response_Ignore_UDP = 5,
} LinDirection_t;

typedef struct {
	uint8_t channel;
    uint8_t pid;
    uint8_t dir;
    uint8_t dl;
    uint8_t data[8];
} LIN_UDP_Frame_t;

typedef enum {
    LinRemoteCmd_t_MODE_CONFIG_UDP = 0,
    LinRemoteCmd_t_ADD_SCHEDULE_UDP = 1,
    LinRemoteCmd_t_DEL_SCHEDULE_UDP = 2,
    LinRemoteCmd_t_SYSTEM_SCHEDULE_UDP = 255,
} LinRemoteCmd_t;

typedef struct {
    uint8_t cmd;
    uint8_t channel;
    uint8_t pid;
    uint8_t dir;
    uint8_t dl;
    uint8_t _pad[3];
    uint8_t data[8];
} LIN_UDP_ConfigScheduleTable_t;

struct asc_lin_schedule_table {
    LIN_UDP_ConfigScheduleTable_t response;
    struct list_head list;
};

typedef enum {
    LinMode_Slave = 0,
    LinMode_Master = 1,
} LinMode_t;

typedef struct {
	uint8_t cmd;
    uint8_t open;
    uint8_t lin_mode;
    uint8_t schedule_flag;
    uint32_t response_timeout;
    uint32_t bitrate;
} LIN_UDP_Config_t;

typedef enum {
    LIN_FRAME_DATA = 0,
    LIN_FRAME_CONTROL = 1,
    LIN_FRAME_CONTROL_SCHEDULE_TABLE = 2,
    LIN_CONFIG_RESPONSE = 3,
    LIN_SCHEDULE_TABLE_RESPONSE = 4,
} LIN_Frame_Type_t;

typedef struct {
    uint8_t flag;
    uint8_t pad;
    uint16_t bitmap;
    uint32_t udpcounter;
    union {
        /* data struct */
        LIN_UDP_Frame_t frame[LIN_CHANNEL_NUM];
        /* control struct */
        LIN_UDP_Config_t ctl[LIN_CHANNEL_NUM];
        /* control schedule*/
        LIN_UDP_ConfigScheduleTable_t ctlScheTab[LIN_CHANNEL_NUM];
        /* config response struct */
        Config_Response_t response[LIN_CHANNEL_NUM];  
    } payload;
} LIN_UDP_t;

void hw_ascLin_init(void);
device_t channel_find_hwlin(uint8_t channel);
device_t ifindex_find_hwlin(uint8_t ifindex);

bool get_lin_frame(LIN_UDP_t *payload);
LIN_UDP_Frame_t* put_next_lin_frame_buffer(int ch);
LIN_UDP_Frame_t* get_next_lin_frame_buffer(int ch);
void lin_frame_read_done(int ch);
void lin_frame_write_done(int ch);

#endif /* __ASCLIN_H_ */
