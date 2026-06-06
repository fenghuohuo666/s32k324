/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-29     Alex_min     first version
 */
#include "device.h"
#include "send_task.h"
#include "def.h"
#include "log.h"
#include "sample_lan7801.h"
#include "clock.h"
#include "IfxCpu_Irq.h"
#include "IfxGeth_Eth.h"
#include "Configuration.h"
#include "ConfigurationIsr.h"
#include "Ifx_Lwip.h"
#include "net.h"
#include "tc387_def.h"
#include "can_device.h"
#include "lwip_udp.h"
#include "ip_set.h"
#include "asclin.h"

struct ringbuffer *tx_buf;
#if (USE_CPU1_LOAD)
extern volatile uint64 core1_idle_cnt;
#endif

#if (USE_CPU2_LOAD)
extern volatile uint64 core2_idle_cnt;
#endif

const UdpPcbConfig *getUdpPcbConfig(device_class_type intf);
volatile uint64 udp_send_cnt = 0;
volatile uint64 udp_send_err_cnt = 0;
volatile uint32 can_udp_packet_counter = 0;

/* send_task 入口验证计数器（纯软件，不涉及硬件操作） */
volatile uint32_t send_task_entry_cnt = 0;
volatile uint32 lin_udp_packet_counter = 0;
IfxCpu_spinLock udp_spinLock;

struct ringbuffer *get_tx_ring(void)
{
    return tx_buf;
}

struct ringbuffer *creat_tx_ring(void)
{
    tx_buf = ringbuffer_create(SEND_BUFF_DEPTH);

    return tx_buf;
}

static device_t try_open_dev(void)
{
    device_t dev;
    int retry = 3;

    do {
        dev = hw_net_find();
    } while (dev == NULL && --retry > 0);

    if (dev == NULL)
        return NULL;

    if (device_init(dev)) {
        return NULL;
    }

    if (device_open(dev, DEVICE_OFLAG_RDWR))
        return NULL;

    return dev;
}

int send_task(void)
{
    int ret;
    device_t dev;

    send_task_entry_cnt++;

    dev = try_open_dev();
    if (NULL == dev) {
        LOG_ERROR(LOG_MODULE_TX_TASK, "open lan7801 failed!\r\n");
        return -1;
    }

    static ip_addr_t udp_addr;
    const UdpPcbConfig *udp_cfg = getUdpPcbConfig(Device_Class_CAN);
    ip4addr_aton("192.168.1.20", &udp_addr);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)sizeof(CAN_UDP_t), PBUF_RAM);
    while (1)
    {
        if (get_can_frame((CAN_UDP_t *)p->payload)) {
            lock(&udp_spinLock);
            ((CAN_UDP_t *)p->payload)->udpcounter = ++can_udp_packet_counter;
            ret = udp_sendto(udp_cfg->pcb, p, &udp_addr, udp_cfg->port);
            if (!ret) {
                udp_send_cnt++;
            } else {
                udp_send_err_cnt++;
            }
            unlock(&udp_spinLock);
            pbuf_free(p);
            p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)sizeof(CAN_UDP_t), PBUF_RAM);
        }

#if (USE_CPU1_LOAD)
        core1_idle_cnt++;
#endif
    }

	return 0;
}

int core2_send_task(void)
{
    int ret;
    device_t dev;

    dev = try_open_dev();
    if (NULL == dev) {
        LOG_ERROR(LOG_MODULE_TX_TASK, "open lan7801 failed!\r\n");
        return -1;
    }

    static ip_addr_t udp_addr;
    const UdpPcbConfig *udp_cfg = getUdpPcbConfig(Device_Class_LIN);
    ip4addr_aton("192.168.1.20", &udp_addr);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)sizeof(LIN_UDP_t), PBUF_RAM);
    while (1)
    {
        if (get_lin_frame((LIN_UDP_t *)p->payload)) {
            lock(&udp_spinLock);
            ((LIN_UDP_t *)p->payload)->udpcounter = ++lin_udp_packet_counter;
            ret = udp_sendto(udp_cfg->pcb, p, &udp_addr, udp_cfg->port);
            if (!ret) {
                udp_send_cnt++;
            } else {
                udp_send_err_cnt++;
            }
            unlock(&udp_spinLock);
            pbuf_free(p);
            p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)sizeof(LIN_UDP_t), PBUF_RAM);
        }

#if (USE_CPU2_LOAD)
        core2_idle_cnt++;
#endif
    }

    return 0;
}
