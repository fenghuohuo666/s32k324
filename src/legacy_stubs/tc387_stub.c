/*
 * tc387_stub.c - Stub implementations for missing TC387 symbols
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ota.h"
#include "lwip_udp.h"
#include "net.h"
#include "flash.h"
#include "def.h"
#include "can_device.h"
#include "asclin.h"

/* From tc387_def.h - CPU load counters */
#if (USE_CPU0_LOAD)
volatile uint64 core0_idle_cnt = 0;
#endif
#if (USE_CPU1_LOAD)
volatile uint64 core1_idle_cnt = 0;
#endif
#if (USE_CPU2_LOAD)
volatile uint64 core2_idle_cnt = 0;
#endif

/* OTA stubs */
/* Flash stub */
boolean initProgramFlash(uint8 partition)
{
    (void)partition;
    return TRUE;
}

/* LWIP UDP stubs */
err_t send_ota_response(ST_UDP_Pcb_Type *uPcb, const ip_addr_t *addr, u16_t port,
                        int result, uint8_t flag, void *buffer)
{
    (void)uPcb;
    (void)addr;
    (void)port;
    (void)result;
    (void)flag;
    (void)buffer;
    return ERR_OK;
}

const UdpPcbConfig *getUdpPcbConfig(device_class_type intf)
{
    (void)intf;
    static UdpPcbConfig cfg = { 0 };
    return &cfg;
}

/* Net stubs */
void hw_net_init(void)
{
}

device_t hw_net_find(void)
{
    return NULL;
}

/* SCU swap stub for schedule.c */
SCU_SWAPCTRL_t SCU_SWAPCTRL;

/* MCM CAN stubs */
void hw_can_init(void) {}
bool get_can_frame(CAN_UDP_t *payload) { (void)payload; return false; }
bool get_lin_frame(LIN_UDP_t *payload) { (void)payload; return false; }
device_t ifindex_find_hwcan(uint8_t ifindex) { (void)ifindex; return NULL; }
device_t channel_find_hwcan(uint8_t channel) { (void)channel; return NULL; }
CAN_Frame_t* get_next_can_frame_buffer(int ch) { (void)ch; return NULL; }
void can_frame_write_done(int ch) { (void)ch; }
CAN_Frame_t* put_next_can_frame_buffer(int ch) { (void)ch; return NULL; }
void can_frame_read_done(int ch) { (void)ch; }
void can_default_cfg(device_t dev) { (void)dev; }
void init_CAN_PHY(void) {}

/* IfxScuRcu stub functions */
void IfxScuRcu_configureResetRequestTrigger(int trigger, int type)
{
    (void)trigger;
    (void)type;
}

/* IfxPort stubs */
#include "IfxPort.h"
void IfxPort_setPinMode(void *port, uint8 pinIndex, IfxPort_OutputMode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinPadDriver(void *port, uint8 pinIndex, IfxPort_PadDriver driver) { (void)port; (void)pinIndex; (void)driver; }
void IfxPort_setPinHigh(void *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinLow(void *port, uint8 pinIndex) { (void)port; (void)pinIndex; }

/* IfxStdIf_DPipe stubs */
#include "IfxStdIf_DPipe.h"
void IfxStdIf_DPipe_write(IfxStdIf_DPipe *dpipe, uint8 *data, uint32 *size, uint32 timeout) { (void)dpipe; (void)data; (void)size; (void)timeout; }
boolean IfxStdIf_DPipe_read(IfxStdIf_DPipe *dpipe, uint8 *data, uint32 *size, uint32 timeout) { (void)dpipe; (void)data; (void)size; (void)timeout; return FALSE; }

/* Ifx_Console / Ifx_Shell stubs */
#include "Ifx_Console.h"
#include "Ifx_Shell.h"
void Ifx_Console_init(Ifx_Console *console, IfxStdIf_DPipe *io) { (void)console; (void)io; }
void Ifx_Shell_init(Ifx_Shell *shell, Ifx_Console *console) { (void)shell; (void)console; }
void Ifx_Shell_process(Ifx_Shell *shell) { (void)shell; }

/* GTM_TOM_FAN stubs */
#include "GTM_TOM_FAN.h"
void GTM_TOM_FAN_Init(void) {}
void GTM_TOM_FAN_SetSpeed(uint32_t speed) { (void)speed; }

/* SPI_init stub */
#include "SPI_init.h"
void SPI_init(void) {}

/* StandBy_Conf stub */
#include "StandBy_Conf.h"
void StandBy_Conf_Init(void) {}

/* TLF35584 stub */
#include "tlf35584.h"
void TLF35584_init(void) {}

/* IfxAsclin_Asc stubs */
#include "IfxAsclin_Asc.h"

/* IfxDmu_reg stub variable */
#include "IfxDmu_reg.h"
Ifx_DMU_HF_ERRSR DMU_HF_ERRSR;

/* IfxScu_reg stub variables */
#include "IfxScu_reg.h"
Ifx_SCU_CCUCON0 SCU_CCUCON0;
Ifx_SCU_CCUCON1 SCU_CCUCON1;
Ifx_SCU_CCUCON2 SCU_CCUCON2;

/* IfxCpu atomic stub */
#include "IfxCpu.h"
void Ifx__imaskldmst(volatile IfxCpu_syncEvent *address, uint32 value, uint32 offset, uint32 count)
{
    (void)offset;
    (void)count;
    *address = value;
}

/* CAN transmit stub (from excluded mcmcan_Interface.c) */
#include "can_device.h"
uint32 transmitMsg(uint8 channel, CAN_Frame_t *frame)
{
    (void)channel;
    (void)frame;
    return 0;
}

IfxCan_Can_NodeConfig configCanNode(uint8 node_channel, CAN_Config_t *node_config)
{
    (void)node_channel;
    (void)node_config;
    IfxCan_Can_NodeConfig cfg = {0};
    return cfg;
}

/* Mbed TLS stubs for md5/platform_util */
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
typedef int mbedtls_mutex_t;
struct tm *mbedtls_platform_gmtime_r(const time_t *tt, struct tm *tm_buf)
{
    (void)tt;
    (void)tm_buf;
    return NULL;
}
void mbedtls_free(void *ptr) { free(ptr); }
int mbedtls_mutex_lock(mbedtls_mutex_t *mutex) { (void)mutex; return 0; }
int mbedtls_mutex_unlock(mbedtls_mutex_t *mutex) { (void)mutex; return 0; }
mbedtls_mutex_t mbedtls_threading_gmtime_mutex;

/* Flash stubs for OTA support */
#include "flash.h"
#include "ucb_swap.h"

void writeProgramFlash(uint32_t address, uint8_t *data)
{
    (void)address;
    (void)data;
}

int swap_bank(void)
{
    return 0;
}

int test_and_set_STOA(void)
{
    return 0;
}



/* CAN interface stubs from mcmcan_Interface.c */
#include "mcmcan_Interface.h"

void init_Mcmcan_module(void)
{
}

boolean switchCanNode(uint8 can_channel, IfxCan_Can_NodeConfig node_config)
{
    (void)can_channel;
    (void)node_config;
    return TRUE;
}

/* Clock stub for get_sync_timestamp */
#include "clock.h"
uint64_t get_sync_timestamp(void)
{
    return 0;
}
