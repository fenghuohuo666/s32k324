/*
 * Copyright (c) 2025, Grce Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     Wenjc     AscLin_Driver
 */

#include "asclin.h"
#include "CODEC.h"
#include "IfxCpu.h"
#include "IfxCpu_Irq.h"
#include "clock.h"
#include "log.h"
#include "tc387_irq_priority.h"
#include <assert.h>

static uint32_t lin_frame_counter[LIN_CHANNEL_NUM] = {0};

IfxCpu_spinLock lin_spinLock;
volatile IfxCpu_syncEvent lin_frame_bit_map = 0;
volatile uint64 udp_send_linframe_cnt = 0;
volatile LIN_UDP_Frame_t lin_frame[LIN_CHANNEL_NUM][LIN_STORE_FRAME_NUM];
volatile LIN_Frame_index_t lin_ring_index[LIN_CHANNEL_NUM];
static uint8 g_lin_pending_pid[LIN_CHANNEL_NUM] = {0};
static bool  g_lin_header_pending[LIN_CHANNEL_NUM] = {false};

/* Global variable */
struct list_head node;
/* Global device list */
static AscLin_Device lin_devices[LIN_CHANNEL_NUM] = {
        {"lin0", 0, RemoteIfIndex_t_LIN0, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin1", 1, RemoteIfIndex_t_LIN1, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin2", 2, RemoteIfIndex_t_LIN2, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin3", 3, RemoteIfIndex_t_LIN3, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin4", 4, RemoteIfIndex_t_LIN4, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin5", 5, RemoteIfIndex_t_LIN5, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin6", 6, RemoteIfIndex_t_LIN6, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin7", 7, RemoteIfIndex_t_LIN7, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin8", 8, RemoteIfIndex_t_LIN8, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin9", 9, RemoteIfIndex_t_LIN9, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin10", 10, RemoteIfIndex_t_LIN10, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin11", 11, RemoteIfIndex_t_LIN11, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin12", 12, RemoteIfIndex_t_LIN12, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin13", 13, RemoteIfIndex_t_LIN13, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin14", 14, RemoteIfIndex_t_LIN14, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
        {"lin15", 15, RemoteIfIndex_t_LIN15, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },

};

/* LIN Module src list */
volatile Ifx_SRC_SRCR *LIN_Moudle_Src_List[LIN_CHANNEL_NUM] = {
    &SRC_ASCLIN0RX,
    &SRC_ASCLIN1RX,
    &SRC_ASCLIN2RX,
    &SRC_ASCLIN3RX,
    &SRC_ASCLIN4RX,
    &SRC_ASCLIN5RX,
    &SRC_ASCLIN6RX,
    &SRC_ASCLIN7RX,
    &SRC_ASCLIN8RX,
    &SRC_ASCLIN9RX,
    &SRC_ASCLIN10RX,
    &SRC_ASCLIN11RX,
    &SRC_ASCLIN13RX,
    &SRC_ASCLIN14RX,
    &SRC_ASCLIN15RX,
    &SRC_ASCLIN16RX
};

/* LIN Module point list */
Ifx_ASCLIN *g_Lin_Moudle_List[LIN_CHANNEL_NUM] = {
    &MODULE_ASCLIN0,
    &MODULE_ASCLIN1,
    &MODULE_ASCLIN2,
    &MODULE_ASCLIN3,
    &MODULE_ASCLIN4,
    &MODULE_ASCLIN5,
    &MODULE_ASCLIN6,
    &MODULE_ASCLIN7,
    &MODULE_ASCLIN8,
    &MODULE_ASCLIN9,
    &MODULE_ASCLIN10,
    &MODULE_ASCLIN11,
    &MODULE_ASCLIN13,
    &MODULE_ASCLIN14,
    &MODULE_ASCLIN15,
    &MODULE_ASCLIN16
};

/* LIN pins list, wait for fill !!!*/
IfxAsclin_Lin_Pins g_Asclin_Pins[LIN_CHANNEL_NUM] = {
        {
                &LIN_0_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_0_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_1_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_1_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_2_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_2_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_3_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_3_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_4_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_4_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_5_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_5_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_6_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_6_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_7_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_7_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_8_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_8_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_9_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_9_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_10_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_10_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_11_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_11_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_13_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_13_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_14_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_14_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_15_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_15_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        },
        {
                &LIN_16_RX_PIN, IfxPort_InputMode_pullUp,
                &LIN_16_TX_PIN, IfxPort_OutputMode_pushPull,
                IfxPort_PadDriver_cmosAutomotiveSpeed1
        }
};

/**********************************************************************************************/
/**********************************************************************************************/
bool get_lin_frame(LIN_UDP_t *payload)
{
    uint16_t map = (uint16_t)lin_frame_bit_map;
    if (!map)
        return 0;

    payload->bitmap = map;
    payload->flag = LIN_FRAME_DATA;

    for (int i = 0; i < LIN_CHANNEL_NUM; i++) {
        if (map & (1U << i)) {
            LIN_UDP_Frame_t *frame = put_next_lin_frame_buffer(i);
            if (frame != NULL) {
                payload->frame[i].channel = frame->channel;
                payload->frame[i].pid = frame->pid;
                payload->frame[i].dir = frame->dir;
                payload->frame[i].dl = frame->dl;
                memcpy(payload->frame[i].data, frame->data, payload->frame[i].dl);
                udp_send_linframe_cnt++;
                lin_frame_read_done(i);
            }
        }
    }

    return 1;
}

static inline lin_frame_ringbuf_state_e ringbuffer_status(int ch)
{
    if (lin_ring_index[ch].read_index == lin_ring_index[ch].write_index) {
        if (lin_ring_index[ch].read_mirror == lin_ring_index[ch].write_mirror)
            return LIN_FRAME_EMPTY;
        else
            return LIN_FRAME_FULL;
    }
    return LIN_FRAME_HALFFULL;
}

LIN_UDP_Frame_t* get_next_lin_frame_buffer(int ch)
{
    if (LIN_FRAME_FULL == ringbuffer_status(ch))
        return NULL;

    if (lin_ring_index[ch].write_index == LIN_STORE_FRAME_NUM - 1) {
        lin_ring_index[ch].write_index = 0;
        lin_ring_index[ch].write_mirror = ~lin_ring_index[ch].write_mirror;
    }

    LIN_UDP_Frame_t *frame = &lin_frame[ch][lin_ring_index[ch].write_index];

    return frame;
}


void lin_frame_write_done(int ch)
{
    lin_ring_index[ch].write_index++;

    Ifx__imaskldmst(&lin_frame_bit_map, 1, ch, 1);
}

LIN_UDP_Frame_t* put_next_lin_frame_buffer(int ch)
{
    if (LIN_FRAME_EMPTY == ringbuffer_status(ch))
        return NULL;
    
    if (lin_ring_index[ch].read_index == LIN_STORE_FRAME_NUM - 1) {
        lin_ring_index[ch].read_index = 0;
        lin_ring_index[ch].read_mirror = ~lin_ring_index[ch].read_mirror;
    }

    LIN_UDP_Frame_t *frame = &lin_frame[ch][lin_ring_index[ch].read_index];

    return frame;
}

void lin_frame_read_done(int ch)
{
    lin_ring_index[ch].read_index++;
    if (LIN_FRAME_EMPTY == ringbuffer_status(ch)) {
        Ifx__imaskldmst(&lin_frame_bit_map, 0, ch, 1);
    }
}

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/

device_t channel_find_hwlin(uint8_t channel)
{
    for (int i = 0; i < sizeof(lin_devices)/sizeof(lin_devices[0]); i++) {
        if (lin_devices[i].channel == channel) {
            return &lin_devices[i].dev;
        }
    }
    return NULL;
}

device_t ifindex_find_hwlin(uint8_t ifindex)
{
    for (int i = 0; i < sizeof(lin_devices)/sizeof(lin_devices[0]); i++) {
        if (lin_devices[i].ifindex == ifindex) {
            return &lin_devices[i].dev;
        }
    }
    return NULL;
}

/**
 * @brief Add the scheduling table
 *
 * @param[in] table: Scheduling table node
 */
static int add_asclin_schedule(AscLin_Device *lin_dev, LIN_UDP_ConfigScheduleTable_t *table)
{
    if (lin_dev == NULL || table == NULL)
    {
        LOG_ERROR(LOG_MODULE_LIN, "lin_dev or table is NULL\r\n");
        return ERROR;
    }
    struct asc_lin_schedule_table *add;

    add = malloc(sizeof(struct asc_lin_schedule_table));
    if (NULL == add)
    {
        LOG_ERROR(LOG_MODULE_LIN, "add is NULL\r\n");
        return ERROR;
    }

    memcpy((void *)&add->response, (void *)table, sizeof(*table));
    list_add_tail(&add->list, &node);
    return EOK;
}

/**
 * @brief Delete the scheduling table
 *
 * @param[in] table: Scheduling table node
 */
static int del_asclin_schedule(AscLin_Device *lin_dev, LIN_UDP_ConfigScheduleTable_t *table)
{
    struct asc_lin_schedule_table *find;

    list_for_each_entry(find, &node, list, struct asc_lin_schedule_table)
    {
        if (find->response.channel == table->channel && find->response.pid == table->pid)
        {
            list_del(&find->list);
            free(find);
            find = NULL;
            return EOK;
        }
    }

    return ERROR;
}

/**
 * @brief Obtain the scheduling table
 *
 * @param[in] table: Scheduling table node
 */
// static int get_asclin_schedule(AscLin_Device *lin_dev,
// LinConfigScheduleTable_t *table)
//{
//     struct asc_lin_schedule_table *find;
//
//     list_for_each_entry(find, &node, list, struct asc_lin_schedule_table) {
//         if (find->response.channel == table->channel &&
//                 find->response.pid == table->pid) {
//             memcpy((void *)table, (void *)&find->response,
//             sizeof(LinConfigScheduleTable_t)); return 0;
//         }
//     }
//
//     return 1;
// }

/**
 * @brief Verify the legitimacy of the scheduling table operations
 * @param lin_dev: LIN device instance
 * @param pid: The PID value to be verified
 * @param interrupcState: Current interrupt status
 * @return 0 Verification successful; otherwise, an error code will be returned
 *
 * @note Interrupts must be disabled before calling this function
 */
static int validate_schedule_operation(AscLin_Device *lin_dev, uint32_t pid)
{
    /* Check whether the mode is slave mode */
    if (lin_dev->config.linMode != IfxAsclin_LinMode_slave)
    {
        return ERROR; // Error codes are recommended to be defined as positive values
    }

    /* Check the PID range (0-0x3F) */
    if (pid > 0x3F)
    {
        return ERROR;
    }
    return EOK;
}

/**
 * @brief Send the Header and Response data of the LIN message.
 * @param[in] lin: A pointer to the IfxAsclin_Lin module instance, used to
 * control LIN communication.
 * @param[in] tx_msg: A pointer to the LIN message structure to be sent,
 * including the PID, data length, and data pointer.
 */
void Lin_Send_HeaderAndResponse(IfxAsclin_Lin *lin, LIN_UDP_Frame_t *tx_msg)
{
    IfxAsclin_Lin_PduType pdu = {
            .pid = tx_msg->pid,
            .direction = IfxAsclin_Lin_Direction_TransmitHeaderAndResponse,
            .dataPtr = (uint8 *)tx_msg->data,
            .dataLength = (uint8)tx_msg->dl,
            .checksumMode = IfxAsclin_Checksum_enhanced
    };

    IfxAsclin_Lin_sendFrame(lin, &pdu);
}

/**
 * @brief Send the LIN message Header and receive the corresponding Response
 * data.
 * @param[in] lin: A pointer to the IfxAsclin_Lin module instance, used to
 * control LIN communication.
 * @param[in] tx_msg: A pointer to the LIN message structure to be sent,
 * including the PID, data length, and data pointer.
 */
void Lin_SendHeader_And_RecvResponse(IfxAsclin_Lin *lin, LIN_UDP_Frame_t *tx_msg)
{
    IfxAsclin_Lin_PduType pdu = {
            .pid = tx_msg->pid,
            .direction = IfxAsclin_Lin_Direction_TransmitHeaderAndReceiveResponse,
            .dataLength = (uint8)tx_msg->dl,
            .checksumMode = IfxAsclin_Checksum_enhanced
    };

    IfxAsclin_Lin_sendFrame(lin, &pdu);
}

void IfxAsclin_Lin_isrsendResponse(IfxAsclin_Lin *asclin, uint8 *data, uint8 length)
{
    Ifx_ASCLIN *asclinSFR = asclin->asclin;
    IfxAsclin_setDataLength(asclinSFR, (IfxAsclin_DataLength)(length - 1));

    /*Configure TXFIFO*/
    IfxAsclin_flushTxFifo(asclinSFR);
    IfxAsclin_enableTxFifoOutlet(asclinSFR, TRUE);
    IfxAsclin_enableRxFifoInlet(asclinSFR, FALSE);

    /*Clear interrupt event flags*/
    IfxAsclin_clearAllFlags(asclinSFR); /* clearing all flags*/
    IfxAsclin_Lin_clearFlagsStatus(asclin);

    IfxAsclin_enableTxResponseEndFlag(asclinSFR, TRUE);   // TRE
    IfxAsclin_enableTxFifoOverflowFlag(asclinSFR, TRUE);  // TFOE
    IfxAsclin_enableCollisionDetectionErrorFlag(asclinSFR, TRUE); // CEE
    IfxAsclin_enableBreakDetectedFlag(asclinSFR, TRUE);     // RTE
    IfxAsclin_enableResponseTimeoutFlag(asclinSFR, TRUE); // BDE

    asclin->linFrameData.flags.txSendResponse = TRUE;
    asclin->linFrameData.flags.txSendHeaderOnly = FALSE;
    asclin->linFrameData.txResponseLength = length;
    asclin->linFrameData.flags.txResponseInProgress = TRUE;
    asclin->linFrameData.flags.txResponseErrorOccurred = FALSE;
    IfxAsclin_write8(asclinSFR, data, length);           /* writing data bytes */
    IfxAsclin_setTransmitResponseRequestFlag(asclinSFR); /* set TRRQS flag */
}

/**
 * @brief Send LIN frames
 * @param tx_msg: LIN data frame structure
 */
void transmitLinMsg(AscLin_Device *lin_dev, LIN_UDP_Frame_t *tx_msg)
{
    IfxAsclin_Lin *lin_handle = &lin_dev->handle;
    switch (tx_msg->dir)
    {
        case LinDirection_t_Master_TransmitHeader_UDP:
            IfxAsclin_Lin_sendHeader(lin_handle, &tx_msg->pid);
            break;
        case LinDirection_t_Master_TransmitHeaderAndResponse_UDP:
            Lin_Send_HeaderAndResponse(lin_handle, tx_msg);
            break;
        case LinDirection_t_Master_TransmitHeaderAndReceiveResponse_UDP:
            Lin_SendHeader_And_RecvResponse(lin_handle, tx_msg);
            break;
        default:
            break;
    }
}

void pack_Lin_Msg_Type(LIN_UDP_Frame_t *rx_msg, AscLin_Device *lin_dev)
{
    rx_msg->channel = lin_dev->channel;
    rx_msg->dl = lin_dev->handle.linFrameData.rxResponseLength;
    memcpy(rx_msg->data, lin_dev->handle.linFrameData.rxResponseData, rx_msg->dl);
}

void IfxAsclin_Lin_isrReceiveHeader(IfxAsclin_Lin *asclin)
{
    Ifx_ASCLIN *asclinSFR = asclin->asclin; /* getting the pointer to ASCLIN registers from module handler*/

    if (IfxAsclin_getRxHeaderEndFlagStatus(asclinSFR))
    {
        IfxAsclin_clearRxHeaderEndFlag(asclinSFR);
        IfxAsclin_Lin_readHeader(asclin, &(asclin->linFrameData.headerID)); /*read the ID byte*/
        asclin->acknowledgmentFlags.rxHeaderEnd       = 1;
        asclin->linFrameData.flags.rxHeaderInProgress = FALSE;
        asclinSFR->TXFIFOCON.B.ENO                    = 0;                  /*TX FIFO Outlet is disabled*/
    }
}

void IfxAsclin_Lin_isrReceiveResponse(IfxAsclin_Lin *asclin)
{
    Ifx_ASCLIN *asclinSFR = asclin->asclin;

    if (IfxAsclin_getRxResponseEndFlagStatus(asclinSFR))
    {
        IfxAsclin_clearRxResponseEndFlag(asclinSFR);
        IfxAsclin_Lin_readResponse(asclin, &(asclin->linFrameData.rxResponseData[0]), (asclin->linFrameData.rxResponseLength)); /* read the data bytes */
        asclin->acknowledgmentFlags.rxResponseEnd       = 1;
        asclin->linFrameData.flags.rxResponseInProgress = FALSE;
    }
}

/**
 * @brief Reset LIN frame state
 * @param lin_handle LIN handle
 * @param channel Channel number
 */
static void reset_Lin_FrameState(IfxAsclin_Lin *lin_handle, uint8_t channel)
{
    // Reset header ID
    lin_handle->linFrameData.headerID = 0;
    
    // Clear response data buffer
    memset(lin_handle->linFrameData.rxResponseData, 0, LIN_MAX_MESSAGE_LENGTH);
    
    // Flush receive FIFO and clear all flags
    IfxAsclin_flushRxFifo(g_Lin_Moudle_List[channel]);
    IfxAsclin_clearAllFlags(g_Lin_Moudle_List[channel]);
}

/**
 * @brief Handle slave mode frame header with schedule table
 * @param lin_dev LIN device pointer
 * @param lin_handle LIN handle
 * @param channel Channel number
 * @param pid Protected ID
 */
static void handle_Slave_Header_WithSchedule(AscLin_Device *lin_dev, IfxAsclin_Lin *lin_handle, 
                                           uint8_t channel, uint8_t pid)
{
    struct asc_lin_schedule_table *table;
    bool found = false;

    list_for_each_entry(table, &node, list, struct asc_lin_schedule_table) {
        if ((table->response.pid == pid) && (table->response.channel == channel)) {
            found = true;
            switch (table->response.dir) {
                case LinDirection_t_Slave_TransmitResponse_UDP:
                {
                    // Receive header and send response
                    IfxAsclin_Lin_isrsendResponse(&lin_dev->handle,
                                                (uint8 *)table->response.data,
                                                (uint8)table->response.dl);
                    break;
                }
                case LinDirection_t_Slave_ReceiveResponse_UDP:
                {
                    IfxAsclin_setDataLength(lin_handle->asclin, (uint8)table->response.dl - 1);
                    lin_handle->linFrameData.rxResponseLength = (uint8)table->response.dl;
                    break;
                }
                case LinDirection_t_Slave_Response_Ignore_UDP:
                {
                    // Do nothing
                    break;
                }
                default:
                    break;
            }
            break; // Exit loop after finding matching item
        }
    }

    // If no matching schedule table entry found, set default data length
    if (!found) {
        IfxAsclin_setDataLength(lin_handle->asclin, IfxAsclin_DataLength_8);
        lin_handle->linFrameData.rxResponseLength = LIN_MAX_MESSAGE_LENGTH;
    }
}

/**
 * @brief Process LIN frame header
 * @param lin_dev LIN device pointer
 * @param lin_handle LIN handle
 * @param channel Channel number
 */
static void process_Lin_Header(AscLin_Device *lin_dev, IfxAsclin_Lin *lin_handle, uint8_t channel)
{
    IfxAsclin_Lin_isrReceiveHeader(lin_handle);
    uint8_t pid = lin_handle->linFrameData.headerID & 0x3F;

    // Store pending PID and header information flag
    g_lin_pending_pid[channel] = pid;
    g_lin_header_pending[channel] = true;

    if (lin_dev->handle.linMode == IfxAsclin_LinMode_slave) {
        if (lin_dev->schedule_enabled) {
            handle_Slave_Header_WithSchedule(lin_dev, lin_handle, channel, pid);
        } else {
            // Non-schedule table mode: Receive all valid data
            IfxAsclin_setDataLength(lin_handle->asclin, IfxAsclin_DataLength_8);
            lin_handle->linFrameData.rxResponseLength = LIN_MAX_MESSAGE_LENGTH;
        }
    }
    // In master mode, only store PID and flag, no additional processing needed
}

/**
 * @brief Handle slave mode frame response with schedule table
 * @param lin_dev LIN device pointer
 * @param frame Frame buffer pointer
 * @param channel Channel number
 */
static void handle_Slave_Response_WithSchedule(AscLin_Device *lin_dev, LIN_UDP_Frame_t *frame, 
                                             uint8_t channel)
{
    if (frame == NULL) {
        // Frame buffer is NULL, directly clear header information flag
        g_lin_header_pending[channel] = false;
        return;
    }
    struct asc_lin_schedule_table *table;

    list_for_each_entry(table, &node, list, struct asc_lin_schedule_table) {
        if ((table->response.pid == g_lin_pending_pid[channel]) && 
            (table->response.channel == channel)) {
            if (table->response.dir == LinDirection_t_Slave_ReceiveResponse_UDP) {
                frame->dir = LinDirection_t_Slave_ReceiveResponse_UDP;
                lin_frame_counter[channel]++;
                lin_frame_write_done(channel);
            }
            // No processing needed for other directions
            break;
        }
    }
    // No matching schedule table entry found, discard the frame and clear header information flag
    g_lin_header_pending[channel] = false;
}

/**
 * @brief Process LIN frame response
 * @param lin_dev LIN device pointer
 * @param lin_handle LIN handle
 * @param channel Channel number
 */
static void process_Lin_Response(AscLin_Device *lin_dev, IfxAsclin_Lin *lin_handle, uint8_t channel)
{
    IfxAsclin_Lin_isrReceiveResponse(lin_handle);
    
    // Get frame buffer
    LIN_UDP_Frame_t *frame = get_next_lin_frame_buffer(channel);
    if (frame == NULL) {
        // Buffer is full, clear header information flag
        g_lin_header_pending[channel] = false;
        return;
    }

    // Pack LIN message type
    pack_Lin_Msg_Type(frame, lin_dev);
    frame->pid = g_lin_pending_pid[channel];

    if (lin_dev->handle.linMode == IfxAsclin_LinMode_slave && lin_dev->schedule_enabled) {
            handle_Slave_Response_WithSchedule(lin_dev, frame, channel);
    } else {
        if (lin_dev->handle.linMode == IfxAsclin_LinMode_slave) {
            frame->dir = LinDirection_t_Slave_ReceiveResponse_UDP;
        } else {
            frame->dir = LinDirection_t_Master_TransmitHeaderAndReceiveResponse_UDP;
        }
        // Non-schedule table mode
        lin_frame_counter[channel]++;
        lin_frame_write_done(channel);
        g_lin_header_pending[channel] = false;
    }
}

/**
 * @brief Process the LIN frame data of the specified channel.
 *
 * This function processes the received LIN frame data based on the current LIN
 * module status of the channel. Supports two processing methods: scheduling
 * table mode and non-scheduling table mode:
 * - In the scheduling table mode, the corresponding scheduling table entry is
 * found based on the PID, and the sending or receiving operation is performed.
 * - In non-scheduling table mode, all valid data is directly received and
 * pushed to the receiving FIFO.
 *
 * @param[in] channel: LIN channel index, used to specify the currently
 * processed LIN channel.
 *
 * @note In the scheduling table mode, if a matching scheduling table entry is
 * found, the sending or receiving operation is performed based on the item
 * configuration.
 * @note In the non-scheduling table mode, all valid data will be received and
 * pushed to the receiving FIFO.
 * @note The function internally uses dynamic memory allocation (malloc) to
 * create a message structure. If the FIFO is full, the memory is released to
 * prevent memory leaks.
 * @note The function will finally clear the RX FIFO and remove all interrupt
 * flags to ensure that the next reception proceeds normally.
 */
void process_Lin_Frame(AscLin_Device *lin_dev)
{
    if (lin_dev == NULL) {
        return;
    }

    IfxAsclin_Lin *lin_handle = &lin_dev->handle;
    uint8_t channel = lin_dev->channel;
    if (channel >= LIN_CHANNEL_NUM) {
        return;
    }

    /* Handle Receive Header (RH) */
    if (IfxAsclin_getRxHeaderEndFlagStatus(lin_handle->asclin)) {
        process_Lin_Header(lin_dev, lin_handle, channel);
    }

    /* Handle Receive Response (RR) */
    if (IfxAsclin_getRxResponseEndFlagStatus(lin_handle->asclin)) {
        if (g_lin_header_pending[channel]) {
            process_Lin_Response(lin_dev, lin_handle, channel);
        }
    }

    /* Cleanup and reset */
    reset_Lin_FrameState(lin_handle, channel);
}

// Safe conversion function
static float32 safe_uint32_to_float32(uint32_t value)
{
    return (value <= (1U << 24)) ? (float32)value : 0.0f; // Ensure 24-bit accuracy
}

static void lin_module_config(AscLin_Device *lin_dev, LIN_UDP_Config_t *cfg)
{
    uint8_t ch = lin_dev->channel;
    /* Basic configuration */
    lin_dev->schedule_enabled = cfg->schedule_flag;
    lin_dev->config.brg.baudrate = safe_uint32_to_float32(cfg->bitrate);
    lin_dev->config.linMode = (IfxAsclin_LinMode)cfg->lin_mode;
    lin_dev->config.data.responseTimeout = cfg->response_timeout;
    lin_dev->handle.linFrameData.txResponseLength = LIN_MAX_MESSAGE_LENGTH;
    lin_dev->handle.linFrameData.rxResponseLength = LIN_MAX_MESSAGE_LENGTH;

    lin_dev->config.interrupt.rxPriority = (Ifx_Priority)(ISR_PRIORITY_ASCLIN0_RX + ch);
    lin_dev->config.interrupt.txPriority = (Ifx_Priority)(ISR_PRIORITY_ASCLIN0_TX + ch);
    lin_dev->config.interrupt.exPriority = (Ifx_Priority)(ISR_PRIORITY_ASCLIN0_ERR + ch);
//    lin_dev->config.interrupt.typeOfService = IfxCpu_Irq_getTos(IfxCpu_getCoreIndex());
    lin_dev->config.interrupt.typeOfService = IfxSrc_Tos_cpu2;

    /* Pattern-related configuration */
    if (lin_dev->config.linMode == IfxAsclin_LinMode_master)
    {
        lin_dev->config.interrupt.enabledInterrupt.txTransmittedHeader = TRUE;
        lin_dev->config.interrupt.enabledInterrupt.txTransmittedResponse = TRUE;
        lin_dev->config.interrupt.enabledInterrupt.rxReceivedResponse = TRUE;
    }

    if (lin_dev->config.linMode == IfxAsclin_LinMode_slave)
    {
        /* From the specific configuration of the mode */
        lin_dev->config.interrupt.enabledInterrupt.rxReceivedHeader = TRUE;
        lin_dev->config.interrupt.enabledInterrupt.rxReceivedResponse = TRUE;
        lin_dev->config.interrupt.enabledInterrupt.txTransmittedResponse = TRUE;
    }

    IfxAsclin_Lin_initModule(&lin_dev->handle, &lin_dev->config);
}

/**************************************************************************************************************/

static f_err_t control_resp(device_t dev, void *buffer, size_t size)
{

    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "(control_resp)Invalid parameters: dev=%p, buffer=%p", dev, buffer);
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_LIN, "device_%s control_resp\r\n",dev->name);

    Item item;
    item.data = buffer;
    item.len = size;

    if (dev->ring_buf == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "device_%s ring_buf is NULL\r\n", dev->name);
        return ERROR;
    }
    size_t len = ringbuffer_put(dev->ring_buf, (uint8_t *)&item, sizeof(Item));
    if(len == 0)
    {
        LOG_ERROR(LOG_MODULE_LIN, "(control_resp)ringbuffer_put faild");
        return ERROR;
    }

    lock(&dev->spinlock);
    dev->frames++;
    unlock(&dev->spinlock);

    return EOK;
}

f_err_t ascLin_init(device_t dev)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "(ascLin_init)Invalid parameters: dev=%p", dev);
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_LIN, "device_%s ascLin_init\r\n",dev->name);
    AscLin_Device *lin_dev = &lin_devices[dev->device_id];
    uint8_t ch = lin_dev->channel;

    /* Hardware default configuration */
    IfxAsclin_Lin_initModuleConfig(&lin_dev->config, g_Lin_Moudle_List[ch]);
    lin_dev->config.isInterruptMode = TRUE;
    lin_dev->config.interrupt.enabledInterrupt.exHeaderTimeout = TRUE;
    lin_dev->config.interrupt.enabledInterrupt.exResponseTimeout = TRUE;
    lin_dev->config.interrupt.enabledInterrupt.exLinChecksumError = TRUE;
    lin_dev->config.interrupt.enabledInterrupt.exFramingError = TRUE;
    lin_dev->config.pins = &g_Asclin_Pins[ch];

    INIT_LIST_HEAD(&node); // Initialize schedule table linked list node
    return 0;
}

f_err_t ascLin_open(device_t dev, uint16_t oflag)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "(ascLin_open)Invalid parameters: dev is NULL");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_LIN, "device_%s ascLin_open\r\n",dev->name);

    return 0;
}

f_err_t ascLin_close(device_t dev)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "(ascLin_close)Invalid parameters: dev is NULL");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_LIN, "device_%s ascLin_close\r\n",dev->name);
    AscLin_Device *lin_dev = &lin_devices[dev->device_id];

    IfxAsclin_Lin_disableModule(&lin_dev->handle);

    return 0;
}

f_err_t ascLin_read(device_t dev, off_t pos, void *buffer, size_t size)
{
    return 0;
}

f_err_t ascLin_write(device_t dev, off_t pos, const void *buffer, size_t size)
{
    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "(ascLin_write)Invalid parameters: dev or buffer is NULL\r\n");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_LIN, "device_%s ascLin_write\r\n",dev->name);

    AscLin_Device *lin_dev = &lin_devices[dev->device_id];
    LIN_UDP_Frame_t *tx_msg = (LIN_UDP_Frame_t *)buffer;
    transmitLinMsg(lin_dev, tx_msg);

    return EOK;
}

f_err_t ascLin_control(device_t dev, int cmd, void *args)
{
    f_err_t ret = ERROR;
    if (dev == NULL || args == NULL)
    {
        LOG_ERROR(LOG_MODULE_LIN, "Invalid parameters: dev or args is NULL\r\n");
        return ret;
    }
    LOG_DEBUG(LOG_MODULE_LIN, "device_%s ascLin_control\r\n",dev->name);
    AscLin_Device *lin_dev = &lin_devices[dev->device_id];

    switch (cmd) {
        case LinRemoteCmd_t_MODE_CONFIG_UDP:
        {
            LIN_UDP_Config_t *cfg = (LIN_UDP_Config_t *)args;

            lin_module_config(lin_dev, cfg);

            ret = cfg->open ? device_open(dev, DEVICE_OFLAG_RDWR):device_close(dev);
            if (ret != EOK) {
                LOG_ERROR(LOG_MODULE_LIN, "Device %s %s failed\r\n",
                        dev->name, cfg->open ? "open" : "close");
            }
            break;
        }
        case LinRemoteCmd_t_ADD_SCHEDULE_UDP:
        case LinRemoteCmd_t_DEL_SCHEDULE_UDP:
        {
            LIN_UDP_ConfigScheduleTable_t *table = (LIN_UDP_ConfigScheduleTable_t *)args;
            ret = validate_schedule_operation(lin_dev, table->pid);
            if (ret != EOK)
            {
                LOG_ERROR(LOG_MODULE_LIN, "validate_schedule_operation is error\r\n");
                return ret;
            }

            if (cmd == LinRemoteCmd_t_ADD_SCHEDULE_UDP)
            {
                ret = add_asclin_schedule(lin_dev, table);
                if (ret != EOK)
                {
                    LOG_ERROR(LOG_MODULE_LIN, "add_asclin_schedule is error\r\n");
                    return ret;
                }
            }
            else
            {
                ret = del_asclin_schedule(lin_dev, table);
                if (ret != EOK)
                {
                    LOG_ERROR(LOG_MODULE_LIN, "del_asclin_schedule is error\r\n");
                    return ret;
                }
            }
            break;
        }
        default:
            return ERROR;
    }

    return ret;
}

f_err_t lin_rx_irq(device_t dev, void *buffer)
{
    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_LIN, "Invalid parameters: dev or buffer is NULL\r\n");
        return ERROR;
    }

    // LinMsg_Type *recv_lin_msg = (LinMsg_Type *)buffer;
    // if(recv_lin_msg == NULL)
    // {
    //     LOG_ERROR(LOG_MODULE_LIN, "Invalid parameters: lin_msg is NULL\r\n");
    //     return ERROR;
    // }

    // RcpMessage *message = build_linframe(dev->device_id, recv_lin_msg);
    // if (message == NULL) {
    //     LOG_ERROR(LOG_MODULE_LIN, "lin_rx_irq rcp_build_message failed\r\n");
    //     return ERROR;
    // }

    // Item item;
    // item.data = (void *)message;
    // item.len = RCPMESSAGE_HEAD_SIZE + sizeof(LinFrame_t);

    // if (dev->ring_buf == NULL) {
    //     LOG_ERROR(LOG_MODULE_LIN, "device_%s ring_buf is NULL\r\n", dev->name);
    //     return ERROR;
    // }
    // size_t len = ringbuffer_put(dev->ring_buf, (uint8_t *)&item, sizeof(Item));
    // if(len == 0)
    // {
    //     LOG_ERROR(LOG_MODULE_LIN, "(lin_rx_irq)ringbuffer_put faild");
    //     free(message);
    //     return ERROR;
    // }

    // lock(&dev->spinlock);
    // dev->frames++;
    // unlock(&dev->spinlock);

    return EOK;
}

struct device_ops lin_ops = {
        .init       = ascLin_init,
        .open       = ascLin_open,
        .close      = ascLin_close,
        .read       = ascLin_read,
        .write      = ascLin_write,
        .control    = ascLin_control
};

f_err_t hw_ascLin_register(device_t device, const char *name, uint32_t flag,
        uint8_t channel, AscLin_Device *lin)
{
    device->device_id       = channel;
    device->type            = Device_Class_LIN;
    device->rx_indicate     = lin_rx_irq;
    device->tx_complete     = NULL;
    device->ops             = &lin_ops;
    device->user_data       = NULL;
    device->frames          = 0;
    device->encode          = NULL;
    device->decode          = NULL;
    device->spinlock        = &lin_spinLock;

    // Bypass schedule core, just for test.
    device->immediate_trans = true;
    device->loopback        = false;

    // Initialize the circular buffer (failure check)
    device->ring_buf = lin_frame[channel];

    /* register a character device */
    return device_register(device, name, DEVICE_FLAG_RDWR | flag);
}

void hw_ascLin_init(void) {
    LOG_DEBUG(LOG_MODULE_LIN, "hw_ascLin_init \r\n");
    /* init AscLin hardware */

    for (uint8_t i = 0; i < sizeof(lin_devices)/sizeof(lin_devices[0]); i++)
    {
        /* Register to the device frame */
        if (hw_ascLin_register(&lin_devices[i].dev,
                                lin_devices[i].name,
                                lin_devices[i].flags,
                                lin_devices[i].channel,
                                NULL) != EOK)
        {
            LOG_ERROR(LOG_MODULE_LIN, "LIN device %s init failed", lin_devices[i].name);
        }
    }
}

// RX Interrupted
 IFX_INTERRUPT(ASCLIN0_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN0_RX);
void ASCLIN0_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[0]);
}

IFX_INTERRUPT(ASCLIN1_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN1_RX);
void ASCLIN1_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[1]);
}

 IFX_INTERRUPT(ASCLIN2_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN2_RX);
 void ASCLIN2_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[2]);
}

 IFX_INTERRUPT(ASCLIN3_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN3_RX);
 void ASCLIN3_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[3]);
}

 IFX_INTERRUPT(ASCLIN4_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN4_RX);
 void ASCLIN4_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[4]);
}

 IFX_INTERRUPT(ASCLIN5_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN5_RX);
 void ASCLIN5_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[5]);
}

 IFX_INTERRUPT(ASCLIN6_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN6_RX);
 void ASCLIN6_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[6]);
}

 IFX_INTERRUPT(ASCLIN7_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN7_RX);
 void ASCLIN7_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[7]);
}

 IFX_INTERRUPT(ASCLIN8_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN8_RX);
 void ASCLIN8_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[8]);
}

 IFX_INTERRUPT(ASCLIN9_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN9_RX);
 void ASCLIN9_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[9]);
}

 IFX_INTERRUPT(ASCLIN10_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN10_RX);
 void ASCLIN10_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[10]);
}

 IFX_INTERRUPT(ASCLIN11_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN11_RX);
 void ASCLIN11_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[11]);
}

 IFX_INTERRUPT(ASCLIN13_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN13_RX);
 void ASCLIN13_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[12]);
}

 IFX_INTERRUPT(ASCLIN14_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN14_RX);
 void ASCLIN14_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[13]);
}

 IFX_INTERRUPT(ASCLIN15_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN15_RX);
 void ASCLIN15_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[14]);
}

 IFX_INTERRUPT(ASCLIN16_RX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN16_RX);
 void ASCLIN16_RX_ISR(void)
{
    process_Lin_Frame(&lin_devices[15]);
}

// TX Interrupted
IFX_INTERRUPT(ASCLIN0_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN0_TX);
void ASCLIN0_TX_ISR(void)
{
     IfxAsclin_Lin_isrTransmit(&lin_devices[0].handle);
 }

IFX_INTERRUPT(ASCLIN1_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN1_TX);
void ASCLIN1_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[1].handle);
}

 IFX_INTERRUPT(ASCLIN2_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN2_TX);
 void ASCLIN2_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[2].handle);
}

 IFX_INTERRUPT(ASCLIN3_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN3_TX);
 void ASCLIN3_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[3].handle);
}

 IFX_INTERRUPT(ASCLIN4_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN4_TX);
 void ASCLIN4_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[4].handle);
}

 IFX_INTERRUPT(ASCLIN5_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN5_TX);
 void ASCLIN5_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[5].handle);
}

 IFX_INTERRUPT(ASCLIN6_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN6_TX);
 void ASCLIN6_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[6].handle);
}

 IFX_INTERRUPT(ASCLIN7_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN7_TX);
 void ASCLIN7_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[7].handle);
}

 IFX_INTERRUPT(ASCLIN8_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN8_TX);
 void ASCLIN8_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[8].handle);
}

 IFX_INTERRUPT(ASCLIN9_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN9_TX);
 void ASCLIN9_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[9].handle);
}

 IFX_INTERRUPT(ASCLIN10_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN10_TX);
 void ASCLIN10_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[10].handle);
}

 IFX_INTERRUPT(ASCLIN11_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN11_TX);
 void ASCLIN11_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[11].handle);
}

 IFX_INTERRUPT(ASCLIN13_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN13_TX);
 void ASCLIN13_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[12].handle);
}

 IFX_INTERRUPT(ASCLIN14_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN14_TX);
 void ASCLIN14_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[13].handle);
}

 IFX_INTERRUPT(ASCLIN15_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN15_TX);
 void ASCLIN15_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[14].handle);
}

 IFX_INTERRUPT(ASCLIN16_TX_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN16_TX);
 void ASCLIN16_TX_ISR(void)
{
    IfxAsclin_Lin_isrTransmit(&lin_devices[15].handle);
}

// ERR Interrupted
IFX_INTERRUPT(ASCLIN0_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN0_ERR);
void ASCLIN0_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[0].handle);
}

IFX_INTERRUPT(ASCLIN1_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN1_ERR);
void ASCLIN1_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[1].handle);
}

IFX_INTERRUPT(ASCLIN2_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN2_ERR);
void ASCLIN2_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[2].handle);
}

IFX_INTERRUPT(ASCLIN3_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN3_ERR);
void ASCLIN3_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[3].handle);
}

IFX_INTERRUPT(ASCLIN4_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN4_ERR);
void ASCLIN4_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[4].handle);
}

IFX_INTERRUPT(ASCLIN5_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN5_ERR);
void ASCLIN5_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[5].handle);
}

IFX_INTERRUPT(ASCLIN6_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN6_ERR);
void ASCLIN6_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[6].handle);
}

IFX_INTERRUPT(ASCLIN7_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN7_ERR);
void ASCLIN7_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[7].handle);
}

IFX_INTERRUPT(ASCLIN8_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN8_ERR);
void ASCLIN8_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[8].handle);
}

IFX_INTERRUPT(ASCLIN9_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN9_ERR);
void ASCLIN9_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[9].handle);
}

IFX_INTERRUPT(ASCLIN10_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN10_ERR);
void ASCLIN10_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[10].handle);
}

IFX_INTERRUPT(ASCLIN11_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN11_ERR);
void ASCLIN11_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[11].handle);
}

IFX_INTERRUPT(ASCLIN13_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN13_ERR);
void ASCLIN13_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[12].handle);
}

IFX_INTERRUPT(ASCLIN14_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN14_ERR);
void ASCLIN14_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[13].handle);
}

IFX_INTERRUPT(ASCLIN15_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN15_ERR);
void ASCLIN15_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[14].handle);
}

IFX_INTERRUPT(ASCLIN16_ERR_ISR, 2, (uint8_t)ISR_PRIORITY_ASCLIN16_ERR);
void ASCLIN16_ERR_ISR(void)
{
    IfxAsclin_Lin_isrError(&lin_devices[15].handle);
}
