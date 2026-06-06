/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Wyj          Demo
 */


/*********************************************************************************************************************/
/*-------------------------------------------------Include files-----------------------------------------------------*/
/*********************************************************************************************************************/
#include <stdlib.h>
#include "i2c.h"
#include "ConfigurationIsr.h"
#include "log.h"
#include "mcmcan_Interface.h"
#include "can_device.h"
#include "GTM_TOM_FAN.h"
#include "tc387_irq_priority.h"
#include "SPI_init.h"
#include "tlf35584.h"
#include "StandBy_Conf.h"

#define TEST_CPU 0
/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
static IfxCan_Can_Node Node_t[8];
static IfxCan_Can      Module_t[3];
static CanInstance_Type g_can_instance;
tlf35584 g_tlfDevice;
int g_powerMode;
int g_mode;

extern struct can_device can_devices[8];

typedef struct {
    IfxCan_Message Msg;
    uint8_t ch;
    uint32_t data[16];
} Test_CAN_Frame;

static uint32_t can_frame_counter[CAN_CHANNEL_NUM] = {0};
static uint32_t can_frame_drop[CAN_CHANNEL_NUM] = {0};

/* Initialize CAN transceiver */
void init_CAN_PHY(void)
{
    init_I2C_module();

    /* Set port 0, 1, 2 as output */
    i2c_write_register(I2C_REG_CONFIG_PORT_0, 0x00);
    i2c_write_register(I2C_REG_CONFIG_PORT_1, 0x00);
    i2c_write_register(I2C_REG_CONFIG_PORT_2, 0x00);

    /* Set CAN transceiver enable and STB pins low level first */
    i2c_write_register(I2C_REG_OUTPUT_PORT_0, 0x00);
    i2c_write_register(I2C_REG_OUTPUT_PORT_1, 0x00);
    i2c_write_register(I2C_REG_OUTPUT_PORT_2, 0x00);

    /* Then set CAN transceiver enable and STB pins high level (wake up CAN transceiver)
     * PORT0: bit0/2/4/6 = CAN1/2/3/4 EN, bit1/3/5/7 = CAN1/2/3/4 STB
     * PORT1: bit0/2/4/6 = CAN5/6/7/8 EN, bit1/3/5/7 = CAN5/6/7/8 STB
     * 0xFF = 11111111b: EN=1, STB=1 for each CAN channel
     */
    i2c_write_register(I2C_REG_OUTPUT_PORT_0, 0xFF);
    i2c_write_register(I2C_REG_OUTPUT_PORT_1, 0xFF);
    i2c_write_register(I2C_REG_OUTPUT_PORT_2, 0x00);
}

/* Control CAN transceiver sleep mode */
void can_sleep_control(void)
{
    /* Set all STB pins low level (enter sleep mode)
     * PORT0: bit0/2/4/6 = CAN1/2/3/4 EN, bit1/3/5/7 = CAN1/2/3/4 STB
     * PORT1: bit0/2/4/6 = CAN5/6/7/8 EN, bit1/3/5/7 = CAN5/6/7/8 STB
     * 0x55 = 01010101b: EN=1, STB=0 for each CAN channel
     */
    i2c_write_register(I2C_REG_OUTPUT_PORT_0, 0x55);
    i2c_write_register(I2C_REG_OUTPUT_PORT_1, 0x55);

    /* Stop fan to reduce power consumption in sleep mode */
    stopFan();

    g_powerMode = standbyMode;
    // initQSPI();                     /* Initialize the QSPI module       */
    // initTLF35584(&g_tlfDevice);     /* Initialize the TLF35584 device   */
    init_Standby();
    init_StandbyController();

    /* Clear the status flag */
    globalClearStatus(&g_tlfDevice);

    goToNormal();
    goToStandby();
}

/* Module initialization */
void init_Mcmcan_module(void)
{
    CanInstance_Type can_instance;
    /*Initialization of the CAN module 0 */
    IfxCan_Can_initModuleConfig(&can_instance.canConfig, &MODULE_CAN0);
    IfxCan_Can_initModule(&Module_t[0], &can_instance.canConfig);
    /*Initialization of the CAN module 1 */
    IfxCan_Can_initModuleConfig(&can_instance.canConfig, &MODULE_CAN1);
    IfxCan_Can_initModule(&Module_t[1], &can_instance.canConfig);
    /*Initialization of the CAN module 2 */
    IfxCan_Can_initModuleConfig(&can_instance.canConfig, &MODULE_CAN2);
    IfxCan_Can_initModule(&Module_t[2], &can_instance.canConfig);
}

/* Config Port_Pin */
static IfxCan_Can_Pins CanPinConfig(uint8 can_channel)
{
    IfxCan_Can_Pins pins;

    switch (can_channel)
    {
        /*can0: module_can0  node0*/
        case 0:
            pins.txPin = &IfxCan_TXD00_P20_8_OUT;
            pins.rxPin = &IfxCan_RXD00B_P20_7_IN;
            break;
        /*can1: module_can0  node1*/
        case 1:
            pins.txPin = &IfxCan_TXD01_P14_0_OUT;
            pins.rxPin = &IfxCan_RXD01B_P14_1_IN;
            break;
        /*can2: module_can0  node2*/
        case 2:
            pins.txPin = &IfxCan_TXD02_P02_2_OUT;
            pins.rxPin = &IfxCan_RXD02B_P02_3_IN;
            break;
        /*can3: module_can0  node3*/
        case 3:
            pins.txPin = &IfxCan_TXD03_P00_2_OUT;
            pins.rxPin = &IfxCan_RXD03A_P00_3_IN;
            break;
        /*can4: module_can1  node0*/
        case 4:
            pins.txPin = &IfxCan_TXD10_P00_0_OUT;
            pins.rxPin = &IfxCan_RXD10A_P00_1_IN;
            break;
        /*can5: module_can1  node1*/
        case 5:
            pins.txPin = &IfxCan_TXD11_P00_4_OUT;
            pins.rxPin = &IfxCan_RXD11B_P00_5_IN;
            break;
        /*can6: module_can1  node2*/
        case 6:
            pins.txPin = &IfxCan_TXD12_P10_7_OUT;
            pins.rxPin = &IfxCan_RXD12B_P10_8_IN;
            break;
        /*can7: module_can2  node1*/
        case 7:
            pins.txPin = &IfxCan_TXD21_P32_3_OUT;
            pins.rxPin = &IfxCan_RXD21D_P32_2_IN;
            break;
        default:
            pins.txPin = NULL;
            pins.rxPin = NULL;
            //TODO:ERROR: This channel does not exist
            break;
    }

    pins.txPinMode = IfxPort_OutputMode_pushPull;
    pins.rxPinMode = IfxPort_InputMode_pullUp;
    pins.padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    return pins;
}

/* Node configuration: Configure nodes using the specified structure */
IfxCan_Can_NodeConfig configCanNode(uint8 node_channel, CAN_Config_t *node_config)
{
    IfxCan_Can_Pins pins;
    CanInstance_Type can_instance;
    int enable;
    uint8 ModuleIdx;
    uint8 NodeIdx;

    if (node_config == NULL) {
        return can_instance.canNodeConfig;
    }

    enable = node_config->open;
    ModuleIdx = node_channel / 4;
    NodeIdx   = node_channel % 4;
    /* initialize CAN node by configuration */
    if (node_channel == 7)
    {
        // can7 Module2 Node1
        ModuleIdx = 2;
        NodeIdx = 1;
    }
   IfxCan_Can_initNodeConfig(&can_instance.canNodeConfig, &Module_t[ModuleIdx]);

   pins = CanPinConfig(node_channel);
   can_instance.canNodeConfig.pins = &pins;

   if(1 == enable)
   {
       can_instance.canNodeConfig.nodeId = (IfxCan_NodeId)NodeIdx;
       can_instance.canNodeConfig.baudRate.baudrate = node_config->bitrate;
       can_instance.canNodeConfig.fastBaudRate.baudrate = node_config->data_bitrate;
       can_instance.canNodeConfig.baudRate.samplePoint = (uint16_t)node_config->sample_point * 100;
       can_instance.canNodeConfig.fastBaudRate.samplePoint = (uint16_t)node_config->data_sample_point * 100;
       can_instance.canNodeConfig.frame.mode = node_config->is_can_fd ? IfxCan_FrameMode_fdLongAndFast : IfxCan_FrameMode_standard;
       can_instance.canNodeConfig.frame.type = IfxCan_FrameType_transmitAndReceive;
       can_instance.canNodeConfig.filterConfig.standardListSize = 0;
       can_instance.canNodeConfig.busLoopbackEnabled = FALSE;
       /*txConfig*/
       can_instance.canNodeConfig.txConfig.txMode = IfxCan_TxMode_dedicatedBuffers;
       can_instance.canNodeConfig.txConfig.txBufferDataFieldSize = IfxCan_DataFieldSize_64;
       can_instance.canNodeConfig.interruptConfig.transmissionCompletedEnabled = TRUE;
       can_instance.canNodeConfig.interruptConfig.traco.priority = (Ifx_Priority)(ISR_PRIORITY_MACN_NODE0_TX + node_channel);
       can_instance.canNodeConfig.interruptConfig.traco.interruptLine = node_channel * 2;
       if (node_channel < 4)
           can_instance.canNodeConfig.interruptConfig.traco.typeOfService = IfxSrc_Tos_cpu0;
       else
           can_instance.canNodeConfig.interruptConfig.traco.typeOfService = IfxSrc_Tos_cpu1;
       /*rxConfig*/
       can_instance.canNodeConfig.rxConfig.rxMode = IfxCan_RxMode_fifo0;
       can_instance.canNodeConfig.rxConfig.rxFifo0DataFieldSize = IfxCan_DataFieldSize_64;
       can_instance.canNodeConfig.rxConfig.rxFifo0Size = 4;
       can_instance.canNodeConfig.interruptConfig.rxFifo0NewMessageEnabled = TRUE;
       can_instance.canNodeConfig.interruptConfig.rxf0n.priority = (Ifx_Priority)(ISR_PRIORITY_MACN_NODE0_RX + node_channel);
       can_instance.canNodeConfig.interruptConfig.rxf0n.interruptLine = (node_channel * 2) + 1;
       if (node_channel < 4)
           can_instance.canNodeConfig.interruptConfig.rxf0n.typeOfService = IfxSrc_Tos_cpu0;
       else
           can_instance.canNodeConfig.interruptConfig.rxf0n.typeOfService = IfxSrc_Tos_cpu1;
   } else{
//        can_instance.canNodeConfig.busLoopbackEnabled = TRUE;
       can_instance.canNodeConfig.nodeId = (IfxCan_NodeId)NodeIdx;
       Module_t[ModuleIdx].can->N[NodeIdx].IE.U = 0x0000;
   }

   return can_instance.canNodeConfig;
}

/* Node switch, configure first and then turn on or off */
boolean switchCanNode(uint8 can_channel,IfxCan_Can_NodeConfig node_config)
{
    boolean init_flag = FALSE;

    init_flag = IfxCan_Can_initNode(&Node_t[can_channel], &node_config);

    return init_flag;
}

/**************************************************************************************************************/

/* CAN's send and receive functions (for bus send and receive) */

/* The sending function assembles the incoming CAN frames into a standard sending format for transmission */
uint32 transmitMsg(uint8 channel, CAN_Frame_t *frame)
{
    uint32 ret = IfxCan_Status_notInitialised;
    uint8 ModuleIdx = (channel) / 4;
    uint8 NodeIdx = (channel) % 4;

    Ifx_CAN_MCR mcr;
    mcr.U = (Module_t[ModuleIdx].can)->MCR.U;
    uint8 isNodeConfigured = 0;
    switch (NodeIdx) {
        case 0:
        isNodeConfigured = mcr.B.CLKSEL0;
        break;
        case 1:
        isNodeConfigured = mcr.B.CLKSEL1;
        break;
        case 2:
        isNodeConfigured = mcr.B.CLKSEL2;
        break;
        case 3:
        isNodeConfigured = mcr.B.CLKSEL3;
        break;
        default :
            return ret;
    }

   if (isNodeConfigured) {
       ret = IfxCan_Can_sendMessage(&Node_t[channel], &frame->Msg, (uint32 *)&frame->data[0]);
   }
   /* 0 means IfxCan_Status_ok */
   return ret;
}

/* The receiving function, interrupt trigger, triggers the callback function of the current device */
static void canRxHandler(CanInstance_Type can_instance, uint8 channel)
{
    device_t dev = &can_devices[channel].dev;
    if(dev != NULL)
    {
        dev->rx_indicate(dev, &can_instance);
    }
}

//RcpMessage *build_canframe(uint8_t channel, CanInstance_Type *recv_can_msg, bool is_fd)
//{
//    if(is_fd) {
//        memset(&canfd_frame, 0, sizeof(CanfdFrame_t));
//
//        canfd_frame.can_id    = recv_can_msg->rxMsg.messageId;
//        canfd_frame.len       = recv_can_msg->rxMsg.dataLengthCode;
//        canfd_frame.flags     = (recv_can_msg->rxMsg.frameMode != 0) ? (CANFD_BRS | CANFD_ESI | CANFD_FDF) : 0;
//        canfd_frame.data.size = (recv_can_msg->rxMsg.frameMode != 0) ? 64 : 8;
//        memcpy(canfd_frame.data.bytes, &recv_can_msg->rxData[0], G_DLC_LOOKUP_TABLE(recv_can_msg->rxMsg.dataLengthCode));
//
//        return rcp_build_message(channel, MessageFlagField_t_MESSAGE_FLAG_BIT_DATA,
//                                    (void *)&canfd_frame, sizeof(CanfdFrame_t), RcpMessage_canfd_frame_tag);
//    } else {
//        memset(&can_frame, 0, sizeof(CanFrame_t));
//
//        can_frame.can_id    = recv_can_msg->rxMsg.messageId;
//        can_frame.can_dlc   = recv_can_msg->rxMsg.dataLengthCode;
//        can_frame.data.size = 8;
//        memcpy(can_frame.data.bytes, &recv_can_msg->rxData[0], G_DLC_LOOKUP_TABLE(recv_can_msg->rxMsg.dataLengthCode));
//
//        return rcp_build_message(channel, MessageFlagField_t_MESSAGE_FLAG_BIT_DATA,
//                                    (void *)&can_frame, sizeof(CanFrame_t), RcpMessage_can_frame_tag);
//    }
//}

#if 0
//Not for now
/* Receive the heartbeat packet information and encode a TCP packet to reply */
void RemoteHeartbeatResp(RcpMessage *resp_msg, RcpMessage msg_head)
{
    resp_msg->timestamp     = get_sync_timestamp();
    resp_msg->flag          = (1 << MessageFlagField_t_MESSAGE_FLAG_BIT_RESPONSE); //bit2: 1: response
    resp_msg->ifIndex       = msg_head.ifIndex;
    resp_msg->which_payload = RcpMessage_heart_beat_resp_tag;
    resp_msg->payload.heart_beat_resp.error = MessageError_t_RCM_ERROR_NONE;
}
#endif
/**************************************************************************************************************/

/* Interrupt trigger for CAN node message sending and receiving (bus data) */

/* The second parameter is the vector table index rather than the hardware interrupt number.
 * Different cores correspond to different indexes (0->0,1->1)
 */
IFX_INTERRUPT(canIsrTxHandler_0, 0, (uint8_t)ISR_PRIORITY_MACN_NODE0_TX);
void canIsrTxHandler_0()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[0].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[0].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_1, 0, (uint8_t)ISR_PRIORITY_MACN_NODE1_TX);
void canIsrTxHandler_1()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[1].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[1].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_2, 0, (uint8_t)ISR_PRIORITY_MACN_NODE2_TX);
void canIsrTxHandler_2()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[2].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[2].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_3, 0, (uint8_t)ISR_PRIORITY_MACN_NODE3_TX);
void canIsrTxHandler_3()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[3].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[3].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_4, 1, (uint8_t)ISR_PRIORITY_MACN_NODE4_TX);
void canIsrTxHandler_4()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[4].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[4].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_5, 1, (uint8_t)ISR_PRIORITY_MACN_NODE5_TX);
void canIsrTxHandler_5()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[5].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[5].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_6, 1, (uint8_t)ISR_PRIORITY_MACN_NODE6_TX);
void canIsrTxHandler_6()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[06].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[6].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrTxHandler_7, 1, (uint8_t)ISR_PRIORITY_MACN_NODE7_TX);
void canIsrTxHandler_7()
{
    /* The node has completed sending. */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[7].node, IfxCan_Interrupt_transmissionCompleted))
    {
        /* Clear the "transmissionCompleted" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[7].node, IfxCan_Interrupt_transmissionCompleted);
    }
}

IFX_INTERRUPT(canIsrRxHandler_0, 0, (uint8_t)ISR_PRIORITY_MACN_NODE0_RX);
void canIsrRxHandler_0()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[0].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[0].node, IfxCan_Interrupt_rxFifo0NewMessage);

        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(0);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[0], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[0]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 0;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[0], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[0]++;
            can_frame_write_done(0);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_1, 0, (uint8_t)ISR_PRIORITY_MACN_NODE1_RX);
void canIsrRxHandler_1()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[1].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[1].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(1);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[1], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[1]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 1;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[1], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[1]++;
            can_frame_write_done(1);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_2, 0, (uint8_t)ISR_PRIORITY_MACN_NODE2_RX);
void canIsrRxHandler_2()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[2].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[2].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(2);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[2], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[2]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 2;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[2], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[2]++;
            can_frame_write_done(2);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_3, 0, (uint8_t)ISR_PRIORITY_MACN_NODE3_RX);
void canIsrRxHandler_3()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[3].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[3].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(3);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[3], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[3]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 3;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[3], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[3]++;
            can_frame_write_done(3);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_4, 1, (uint8_t)ISR_PRIORITY_MACN_NODE4_RX);
void canIsrRxHandler_4()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[4].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[4].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(4);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[4], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[4]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 4;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[4], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[4]++;
            can_frame_write_done(4);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_5, 1, (uint8_t)ISR_PRIORITY_MACN_NODE5_RX);
void canIsrRxHandler_5()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[5].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[5].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(5);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[5], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[5]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 5;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[5], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[5]++;
            can_frame_write_done(5);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_6, 1, (uint8_t)ISR_PRIORITY_MACN_NODE6_RX);
void canIsrRxHandler_6()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[6].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[6].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(6);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[6], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[6]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 6;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[6], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[6]++;
            can_frame_write_done(6);
        }
    }
}

IFX_INTERRUPT(canIsrRxHandler_7, 1, (uint8_t)ISR_PRIORITY_MACN_NODE7_RX);
void canIsrRxHandler_7()
{
    /* The node receives a new message */
    if (IfxCan_Node_getInterruptFlagStatus(Node_t[7].node, IfxCan_Interrupt_rxFifo0NewMessage)) {
        /* Clear the "RX FIFO 0 new message" interrupt flag */
        IfxCan_Node_clearInterruptFlag(Node_t[7].node, IfxCan_Interrupt_rxFifo0NewMessage);
        /* Received message content should be updated with the data stored in the RX FIFO 0 */
        CAN_Frame_t *frame = get_next_can_frame_buffer(7);
        if (NULL == frame) {
            CAN_Frame_t drop_frame;
            drop_frame.Msg.readFromRxFifo0 = TRUE;
            IfxCan_Can_readMessage(&Node_t[7], &drop_frame.Msg, (uint32 *)&drop_frame.data[0]);
            can_frame_drop[7]++;
        } else {
            frame->Msg.readFromRxFifo0 = TRUE;
            frame->ch = 7;
            /* Read the received CAN message */
            IfxCan_Can_readMessage(&Node_t[7], &frame->Msg, (uint32 *)&frame->data[0]);
            can_frame_counter[7]++;
            can_frame_write_done(7);
        }
    }
}
