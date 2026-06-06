/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Wyj          Demo
 */

#ifndef __MCMCAN_H_
#define __MCMCAN_H_

#include "device.h"
#include "tc387_response.h"
#include "mcmcan_Interface.h"

#if defined(__TASKING__)
#define PACKED_BEGIN _Pragma("pack 2")
#define PACKED_END   _Pragma("pack 0")
#define PACKED
#elif defined(__GNUC__)
#define PACKED_BEGIN
#define PACKED_END
#define PACKED __attribute__((packed))
#else
#define PACKED_BEGIN
#define PACKED_END
#define PACKED
#endif

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

#define DATA_MAX_LEN            1024
#define CAN_NODE_NUM            9
#define CAN_OPEN_FLAG           TRUE
#define CAN_CLOSE_FLAG          FALSE
#define CAN_STORE_FRAME_NUM     80
#define CAN_CHANNEL_NUM         8
#define CAN_DATA_FRAME_HEAD     2

/*********************************************************************************************************************/
/*--------------------------------------------------Data Structures--------------------------------------------------*/
/*********************************************************************************************************************/
struct can_device
{
    const char *name;
    uint8_t  channel;
    uint8_t  ifindex;
    uint32_t flags;
    boolean  if_fd;
    struct device dev;
};

typedef struct 
{
	void *data;
	void *head;
	void *tail;
} can_protocol;

typedef enum
{
    CAN_FRAME_EMPTY,
    CAN_FRAME_FULL,
    /* half full is neither full nor empty */
    CAN_FRAME_HALFFULL,
} can_frame_ringbuf_state_e;

typedef struct {
    uint8_t read_mirror : 1;
    uint8_t read_index  : 7;
    uint8_t write_mirror: 1;
    uint8_t write_index : 7;
} CAN_Frame_index_t;

typedef enum {
    CAN_FRAME_DATA,         /* CAN frame data */
    CAN_FRAME_CONTROL,       /* CAN frame control */
    CAN_CONFIG_RESPONSE,     /* CAN config response */
    CAN_SLEEP         /* CAN sleep control */
} can_frame_type_e;

typedef struct {
    uint8_t ch;
    uint8_t fd;
    uint8_t dlc;
    uint8_t pad;    /* PAD byte */
    uint32_t  messageId;
    uint8_t data[64];
} CAN_UDP_Frame_t;

/* CAN_Config_t is defined in mcmcan_Interface.h which this header includes */

typedef struct  {
    uint8_t flag;
    uint8_t bitmap;
    uint16_t pad;
    uint32_t udpcounter;
    union {
        /* data struct */
        CAN_UDP_Frame_t frame[CAN_CHANNEL_NUM];
        /* control struct */
        CAN_Config_t ctl[CAN_CHANNEL_NUM];
        /* config response struct */
        Config_Response_t response[CAN_CHANNEL_NUM];
    } payload;
} CAN_UDP_t;

/*********************************************************************************************************************/
/*-----------------------------------------------Function Prototypes-------------------------------------------------*/
/*********************************************************************************************************************/
void hw_can_init(void);
bool get_can_frame(CAN_UDP_t *payload);
device_t ifindex_find_hwcan(uint8_t ifindex);
device_t channel_find_hwcan(uint8_t channel);

CAN_Frame_t* get_next_can_frame_buffer(int ch);
void can_frame_write_done(int ch);
CAN_Frame_t* put_next_can_frame_buffer(int ch);
void can_frame_read_done(int ch);
void can_default_cfg(device_t dev);
#endif /* __MCMCAN_H_ */
