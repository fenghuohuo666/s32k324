/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-08     Wyj          OTA
 */

#ifndef _OTA_H_
#define _OTA_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "device.h"
#include "flash.h"
#include "tc387_response.h"
#include "lwip_udp.h"
/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define OTA_RING_DEPTH      20

#define OTA_VERSION         0x00000001
/*********************************************************************************************************************/
/*-------------------------------------------------------TYPE--------------------------------------------------------*/
/*********************************************************************************************************************/
struct ota_device
{
    const char *name;
    uint8_t  channel;
    uint8_t  ifindex;
    struct device dev;
};

typedef struct {
    char version[64];
    bool flag;
} DeviceInfo;

typedef enum {
    OTA_VERSION_MESSAGE,
    OTA_UPGRADE_MESSAGE,
    OTA_ERASE_SUC_RESP,
    OTA_TRABSFER_DATA,
    OTA_END_MESSAGE,
    OTA_RESET_MESSAGE,
} ota_message_type_t;

typedef struct {
    uint8_t reserved;
} Ota_Req_t;

typedef struct {
    uint8_t error;
    char version[64];
} Ota_Version_Resp_t;

typedef struct {
    uint8_t error;
    uint8_t isReady;
} Ota_Upgrade_Resp_t;

typedef struct {
    bool reserved;
} Ota_Upgrade_Erase_Suc_t;

typedef struct {
    uint16_t size;
    uint8_t  bytes[32];
} Ota_Transfer_Data_t;

typedef struct {
    Ota_Transfer_Data_t data; /*Block*/
    uint32_t address;
    uint32_t seqNum;         /*Serial*/
} Ota_Transfer_Data_Req_t;

typedef struct {
    uint8_t error;
    uint32_t seqNum; /* Confirm the received serial number */
} Ota_Transfer_Data_Resp_t;

typedef struct {
    uint16_t size;
    uint8_t  bytes[16];
} Ota_Transfer_End_Md5_t;

typedef struct {
    uint32_t totalSeqNum;
    Ota_Transfer_End_Md5_t md5;
} Ota_Transfer_End_Req_t;

typedef struct {
    uint8_t error;
    bool md5Match; /* 1=Match,0=NotMatch */
} Ota_Transfer_End_Resp_t;

typedef struct {
    uint8_t error;
} Ota_Reset_Resp_t;

typedef struct  {
    uint8_t flag;
    uint16_t pad;
    union {
        /* req */
        Ota_Req_t req;
        Ota_Transfer_Data_Req_t  ota_transfer_data_req;
        Ota_Transfer_End_Req_t   ota_transfer_end_req;
        /* resp */
        Ota_Version_Resp_t       version_resp;
        Ota_Upgrade_Resp_t       ota_upgrade_resp;
        Ota_Upgrade_Erase_Suc_t  ota_upgrade_erase;
        Ota_Transfer_Data_Resp_t ota_transfer_data_resp;
        Ota_Transfer_End_Resp_t  ota_transfer_end_resp;
        Ota_Reset_Resp_t         ota_reset_resp;
    } u;
} OTA_UDP_t;

/* Compatibility alias for schedule.c */
#define OTA_UPGRADE_ERASE_SUC_t Ota_Upgrade_Erase_Suc_t
/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void hw_ota_init(void);
device_t hw_ota_find(uint8_t ifindex);
bool get_earse_flag(void);
void set_earse_flag(bool flag);

#endif /* _OTA_H_ */
