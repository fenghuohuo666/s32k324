/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-08     Wyj          OTA
 */
/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include <stdio.h>
#include <string.h>

#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "IfxScuRcu.h"

#include "ota.h"
#include "codec.h"
#include "ringbuffer.h"
#include "log.h"
#include "md5.h"
#include "clock.h"
#include "rcp_build.h"
#include "ucb_swap.h"

#include "Bsp.h"
/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
struct ota_device  ota_devices[] = {
    { .name = "ota", .channel = 0, .ifindex = RemoteIfIndex_t_FIRMWARE_UPDATE },
};

IfxCpu_spinLock ota_spinLock;
static DeviceInfo g_Device;
static mbedtls_md5_context g_ctx;
static uint32_t g_seqnum;
static int8_t  g_partition;
static const char* version_info __attribute__((unused)) = "ota_version_0000";

#define MIN(a, b) ((a) < (b) ? (a) : (b))
static uint32_t g_current_add;
static uint8_t  g_page_buffer[PFLASH_PAGE_LENGTH];
static bool page_dirty = false;
static bool earse_flag = false;
/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
bool get_earse_flag(void)
{
    return earse_flag;
}

void set_earse_flag(bool flag)
{
    earse_flag = flag;
}

static void triggerSwReset(void)
{
    /* Get the CPU EndInit password */
    uint16 CPUEndinitPw = IfxScuWdt_getCpuWatchdogPassword();

    /* Configure the request trigger in the Reset Configuration Register */
    IfxScuRcu_configureResetRequestTrigger(IfxScuRcu_Trigger_sw, IfxScuRcu_ResetType_system);

    /* Clear CPU EndInit protection to write in the SWRSTCON register of SCU */
    IfxScuWdt_clearCpuEndinit(CPUEndinitPw);

    /* Trigger a software reset based on the configuration of RSTCON register */
    IfxCpu_triggerSwReset();

    /* The following instructions are not executed if a SW reset occurs */
    /* Set CPU EndInit protection */
    IfxScuWdt_setCpuEndinit(CPUEndinitPw);
}

static void get_partition(void)
{
    switch (SCU_SWAPCTRL.B.ADDRCFG) {
        case 0x01:
            g_partition = PARTITION_A;
            break;
        case 0x02:
            g_partition = PARTITION_B;
            break;
        default:
            g_partition = -1;
            break;
    }
}

static void flush_page(void) {
    if (page_dirty) {
        writeProgramFlash(g_current_add, g_page_buffer);
        page_dirty = false;
        memset(g_page_buffer, 0xFF, PFLASH_PAGE_LENGTH);
    }
}

static void fls_data_dispose(uint32 startingAddr, uint8 *data, size_t data_len)
{
    uint32_t offset = 0;
    while (offset < data_len) {
        uint32_t byte_addr = startingAddr + offset;
        uint32_t page_start = byte_addr & ~(PFLASH_PAGE_LENGTH-1); // 计算当前字节所在页起始地址

        if (page_start != g_current_add) {
            flush_page();
            g_current_add = page_start;
        }

        uint32_t page_offset = byte_addr & (PFLASH_PAGE_LENGTH-1);
        uint32_t count = MIN(data_len - offset, PFLASH_PAGE_LENGTH - page_offset);

        memcpy(g_page_buffer + page_offset, data + offset, count);
        page_dirty = true;
        offset += count;
    }
}


static void md5_init(void)
{
    mbedtls_md5_init(&g_ctx);
    mbedtls_md5_starts(&g_ctx);
}

static void ota_reset_state(void)
{
    md5_init();
    g_seqnum = 0;
    memset(g_page_buffer, 0xFF, PFLASH_PAGE_LENGTH);
}

//static f_err_t req_resp(device_t dev, void *buffer, size_t size)
//{
//    LOG_DEBUG(LOG_MODULE_OTA, "device_%s req_resp\r\n",dev->name);
//    if (dev == NULL || buffer == NULL) {
//        LOG_ERROR(LOG_MODULE_OTA, "(req_resp)Invalid parameters: dev or buffer is NULL");
//        return ERROR;
//    }
//
//    Item item;
//    item.data = buffer;
//    item.len  = size;
//
//    assert(dev->ring_buf != NULL);
//    size_t len = ringbuffer_put(dev->ring_buf, (uint8_t *)&item, sizeof(Item));
//    if(len == 0)
//    {
//        LOG_ERROR(LOG_MODULE_OTA, "(control_resp)ringbuffer_put faild");
//        return ERROR;
//    }
//
//    lock(&dev->spinlock);
//    dev->frames++;
//    unlock(&dev->spinlock);
//
//    return EOK;
//}

device_t hw_ota_find(uint8_t ifindex)
{
    (void)ifindex;
    return &ota_devices[0].dev;
}

f_err_t ota_init(device_t dev)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "(ota_init)Invalid parameters: dev is NULL\r\n");
        return ERROR;
    }
    ota_reset_state();
    return 0;
}

f_err_t ota_open(device_t dev, uint16_t oflag)
{
    (void)oflag;
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "(ota_open)Invalid parameters: dev is NULL\r\n");
        return ERROR;
    }
    /* When opening OTA_DEV, obtain the version number of the current upgrade package in advance.
       If the acquisition fails, set the flag to flase */
    memset(&g_Device, 0, sizeof(DeviceInfo));
    snprintf(g_Device.version, sizeof(g_Device.version), "%08X", OTA_VERSION);
    g_Device.flag     = true;

    return 0;
}

f_err_t ota_close(device_t dev)
{
    (void)dev;
    return 0;
}

f_err_t ota_read(device_t dev, off_t pos, void *buffer, size_t size)
{
    (void)size;
    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "(ota_read)Invalid parameters: dev or buffer is NULL\r\n");
        return ERROR;
    }

    static ip_addr_t udp_addr;
    ip4addr_aton("192.168.1.20", &udp_addr);
    const UdpPcbConfig *udp_cfg = getUdpPcbConfig(Device_Class_OTA);
    if(udp_cfg == NULL)
    {
        LOG_ERROR(LOG_MODULE_LAN7801, "Device_Class_OTA not find pcb\r\n", Device_Class_OTA);
        return ERROR;
    }

    switch (pos) {
        case OTA_VERSION_MESSAGE: {
            Ota_Version_Resp_t version_resp = {
                .error = g_Device.flag ? CONFIG_SUCCESS_UDP : 1
            };
            snprintf(version_resp.version, sizeof(version_resp.version), "%s", g_Device.version);

            send_ota_response(udp_cfg->pcb, &udp_addr, udp_cfg->port, ERR_OK, (uint8_t)OTA_VERSION_MESSAGE, (void *)&version_resp);
            break;
        }
        case OTA_UPGRADE_MESSAGE: {
            Ota_Upgrade_Resp_t upgrade_resp = {
                .error = (uint8_t)CONFIG_SUCCESS_UDP,
                .isReady = 1
            };
            send_ota_response(udp_cfg->pcb, &udp_addr, udp_cfg->port, ERR_OK, (uint8_t)OTA_UPGRADE_MESSAGE, (void *)&upgrade_resp);

            /*async earse*/
            set_earse_flag(true);
            break;
        }
        case OTA_END_MESSAGE: {
            int result = -1;
            flush_page();
            uint8 md5_sum[16];
            mbedtls_md5_finish(&g_ctx, md5_sum);
            Ota_Transfer_End_Req_t *req_msg  = (Ota_Transfer_End_Req_t*)buffer;
            Ota_Transfer_End_Resp_t end_resp = {0};
            if((g_seqnum == req_msg->totalSeqNum) && (memcmp(req_msg->md5.bytes, md5_sum, sizeof(md5_sum)) == 0))
            {
                LOG_DEBUG(LOG_MODULE_OTA, "The MD5 values are the same\r\n");
                end_resp.error    = (uint8_t)CONFIG_SUCCESS_UDP;
                end_resp.md5Match = true;
                waitTime(1000000);
                swap_bank();//ucb_swap
                result = EOK;
            } else {
                LOG_ERROR(LOG_MODULE_OTA, "The MD5 values are different\r\n");
                //The verification failed. Prepare to receive the OTA package again
                ota_reset_state();
                end_resp.error = 1;
                end_resp.md5Match = false;
                result = ERROR;
            }
            send_ota_response(udp_cfg->pcb, &udp_addr, udp_cfg->port, result, (uint8_t)OTA_END_MESSAGE, (void *)&end_resp);
            break;
        }
        default:
            LOG_ERROR(LOG_MODULE_OTA, "(ota_read)Invalid parameters: pos(%d) is mistake\r\n",pos);
            return ERROR;
    }

    return EOK;
}

f_err_t ota_write(device_t dev, off_t pos, const void *buffer, size_t size)
{
    (void)pos;
    (void)size;
    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "(ota_write)Invalid parameters: dev or buffer is NULL\r\n");
        return ERROR;
    }

    static ip_addr_t udp_addr;
    ip4addr_aton("192.168.1.20", &udp_addr);
    const UdpPcbConfig *udp_cfg = getUdpPcbConfig(Device_Class_OTA);
    if(udp_cfg == NULL)
    {
        LOG_ERROR(LOG_MODULE_LAN7801, "Device_Class_OTA not find pcb\r\n", Device_Class_OTA);
        return ERROR;
    }

    int result = -1;
    uint32_t actual_address = 0;
    OTA_TRANSFER_DATA_REQ_t *req_msg = (OTA_TRANSFER_DATA_REQ_t*)buffer;
    Ota_Transfer_Data_Resp_t data_resp = {0};
    if(req_msg->seqNum == g_seqnum)
    {
        g_seqnum++;
        data_resp.error  = (uint8_t)CONFIG_SUCCESS_UDP;
        data_resp.seqNum = req_msg->seqNum;

        //Stream segment calculation of the MD5 value of OTA packets
        mbedtls_md5_update(&g_ctx, (const unsigned char *)req_msg->data.bytes, req_msg->data.size);
        //FLASH Write
        if(g_partition == PARTITION_B)
        {
            actual_address = req_msg->address - 0x00600000;
        } else {
            actual_address = req_msg->address;
        }
        fls_data_dispose(actual_address, req_msg->data.bytes, req_msg->data.size);
        result = EOK;
    } else {
        LOG_ERROR(LOG_MODULE_OTA, "The received serialization number does not match: expected %d, got %d\r\n",
                         g_seqnum, req_msg->seqNum);
        data_resp.error  = 1;
        data_resp.seqNum = req_msg->seqNum;
        ota_reset_state();
        result = ERROR;
    }
    send_ota_response(udp_cfg->pcb, &udp_addr, udp_cfg->port, result, (uint8_t)OTA_TRABSFER_DATA, (void *)&data_resp);
    return EOK;
}

f_err_t ota_control(device_t dev, int cmd, void *args)
{
    (void)cmd;
    if (dev == NULL || args == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "(ota_control)Invalid parameters: dev or args is NULL\r\n");
        return ERROR;
    }

    static ip_addr_t udp_addr;
    ip4addr_aton("192.168.1.20", &udp_addr);
    const UdpPcbConfig *udp_cfg = getUdpPcbConfig(Device_Class_OTA);
    if(udp_cfg == NULL)
    {
        LOG_ERROR(LOG_MODULE_LAN7801, "Device_Class_OTA not find pcb\r\n", Device_Class_OTA);
        return ERROR;
    }

    Ota_Reset_Resp_t reset_resp = {
        .error = (uint8_t)CONFIG_SUCCESS_UDP,
    };
    send_ota_response(udp_cfg->pcb, &udp_addr, udp_cfg->port, EOK, (uint8_t)OTA_RESET_MESSAGE, (void *)&reset_resp);
    //Wait for the OTA to complete the reply
    uint64_t start_time = get_sync_timestamp();
    while(1)
    {
        uint64_t current_time = get_sync_timestamp();
        if((current_time - start_time) >= 1000000) //1s
        {
            LOG_DEBUG(LOG_MODULE_OTA, "Ready to restart\r\n");
            break;
        }
    }
    //Perform a system-level restart
    triggerSwReset();
    return 0;
}

struct device_ops ota_ops = {
    .init    = ota_init,
    .open    = ota_open,
    .close   = NULL,
    .read    = ota_read,
    .write   = ota_write,
    .control = ota_control,
};

f_err_t ota_encode(Item *item)
{
    if (item == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "Invalid parameters: item is NULL\r\n");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_OTA, "ota_encode\r\n");
    RcpMessage   *message = (RcpMessage *)item->data;

    uint8_t *serialized_data = NULL;
    size_t encoded_size = 0;
    size_t buffer_size = 256;

    serialized_data = malloc(buffer_size);
    if (serialized_data == NULL) {
        LOG_ERROR(LOG_MODULE_OTA, "serialized_data malloc failed!\r\n");
        return ERROR;
    }

    if (serialize_message(serialized_data, buffer_size, message, &encoded_size, Data_Type_OTA_TYPE)) {
        LOG_DEBUG(LOG_MODULE_OTA, "serialize SUCCESS!\r\n");

        free(item->data);
        item->data = serialized_data;
        item->len = encoded_size;

        return EOK;
    } else {
        LOG_ERROR(LOG_MODULE_OTA, "serialize FAIL!!!\r\n");
        return ERROR;
    }

    return EOK;
}

f_err_t hw_ota_register(device_t device, const char* name,
                         uint32_t flag, struct ota_device *ota)
{
    (void)flag;
    (void)ota;
    device->type            = Device_Class_OTA;
    device->rx_indicate     = NULL;
    device->tx_complete     = NULL;
    device->ops             = &ota_ops;
    device->user_data       = NULL;
    device->frames          = 0;
    device->encode          = ota_encode;
    device->decode          = NULL;
    device->spinlock        = &ota_spinLock;

    //Bypass schedule core, just for test.
    device->immediate_trans = true;

    device->loopback        = false;

    // Initialize the circular buffer (failure check)
//    device->ring_buf = ringbuffer_create(OTA_RING_DEPTH);
//    if (device->ring_buf == NULL) {
//        LOG_ERROR(LOG_MODULE_OTA, "Ring buffer creation failed for %s", name);
//        return ERROR;
//    }
    device->ring_buf = NULL;

    /* register a character device */
    return device_register(device, name, DEVICE_FLAG_RDWR | flag);
}

void hw_ota_init(void)
{
    LOG_DEBUG(LOG_MODULE_OTA, "hw_ota_init\r\n");
    /* init OTA hardware */
    get_partition();
    test_and_set_STOA();

    /* register OTA device */
    hw_ota_register(&ota_devices[0].dev,ota_devices[0].name,
                       DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM,NULL);
}
