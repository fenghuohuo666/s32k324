/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-11     Wyj          Demo
 */

#ifndef __LWIP_LAN7801_H_
#define __LWIP_LAN7801_H_

#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include "IfxStm.h"
#include "pb_type.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "def.h"


/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define UDP_SESSION_RECVBUF_SIZE 1600 /* Define the size of the storage array for received strings */

/*********************************************************************************************************************/
/*--------------------------------------------------Data Structures--------------------------------------------------*/
/*********************************************************************************************************************/
typedef struct udp_pcb ST_UDP_Pcb_Type;    /* Define a standard type for "struct udp_pcb" */
typedef struct pbuf ST_UDP_PacketBuf_Type; /* Define a standard type for "struct pbuf" */

typedef struct /* Session data structure used for communicating with remote clients*/
{
    u8_t state;                             /* The current state for the session */
    ST_UDP_Pcb_Type *pcb;                   /* Pointer to the UDP protocol control block */
    char storage[UDP_SESSION_RECVBUF_SIZE]; /* Storage for the received strings */
    uint16 nextFreeStoragePos;              /* Position of the next free position in the storage array */
    ip_addr_t remote_ip;                    /* Remote IP address */
    u16_t remote_port;                      /* Remote port */
} ST_UDP_Session_Type;

typedef struct {
    device_class_type intf;
    uint16_t port;
    ST_UDP_Pcb_Type *pcb;
} UdpPcbConfig;
/*********************************************************************************************************************/
/*-----------------------------------------------Function Prototypes-------------------------------------------------*/
/*********************************************************************************************************************/
/* Type of a function pointer to a UDP port receive hook */
typedef void (*FP_UDP_PortRecvHook_Type)(void *arg,
                                        struct udp_pcb *upcb,
                                        struct pbuf *p,
                                        const ip_addr_t *addr,
                                        u16_t port);

void UDP_SessionRecv_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                             const ip_addr_t *addr, u16_t port); /* Recv callback is called every time data is received */
void UDP_SessionError_Callback(void *arg, err_t err);           /* Error callback is called if an error occurs */

void UDP_SessionSend(ST_UDP_Pcb_Type *uPcb, ST_UDP_Session_Type *es,
                    const ip_addr_t *addr, u16_t port);         /* Send function sends UDP data to remote client */
void UDP_SessionUnpack(ST_UDP_Session_Type *es, struct pbuf *p); /* Unpack function copies data from pbuf to session storage */

void UDP_PortInit(ST_UDP_Pcb_Type **uPcb, u16_t port);
err_t UDP_PortRecvHook_Bind(ST_UDP_Pcb_Type *uPcb, FP_UDP_PortRecvHook_Type recvHook);

void UDP_UserRecvHandle(ST_UDP_Pcb_Type *uPcb, ST_UDP_Session_Type *session,
                       const ip_addr_t *addr, u16_t port);

void udpManagerInit(void);
const UdpPcbConfig *getUdpPcbConfig(device_class_type intf);
err_t send_ota_response(ST_UDP_Pcb_Type *uPcb, const ip_addr_t *addr, u16_t port, int result, uint8_t flag, void *buffer);

#endif /* __MCMCAN_H_ */
