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
#include "log.h"
#include "ringbuffer.h"
#include "codec.h"
#include "IfxCpu.h" 
#include "can_device.h" 
#include "board.h"

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
static IfxCan_Can_NodeConfig can_nodeconfig;
volatile CAN_Frame_t can_frame[CAN_CHANNEL_NUM][CAN_STORE_FRAME_NUM];
volatile CAN_Frame_index_t can_ring_index[CAN_CHANNEL_NUM];
volatile IfxCpu_syncEvent can_frame_bit_map = 0;
IfxCpu_spinLock can_spinLock;
volatile uint64 udp_send_frame_cnt = 0;

struct can_device  can_devices[CAN_NODE_NUM] = {
    { "can0", 0, RemoteIfIndex_t_CAN0, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can1", 1, RemoteIfIndex_t_CAN1, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can2", 2, RemoteIfIndex_t_CAN2, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can3", 3, RemoteIfIndex_t_CAN3, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can4", 4, RemoteIfIndex_t_CAN4, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can5", 5, RemoteIfIndex_t_CAN5, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can6", 6, RemoteIfIndex_t_CAN6, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "can7", 7, RemoteIfIndex_t_CAN7, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false },
    { "cansleep", 8, RemoteIfIndex_t_CAN_SLEEP, DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM, false }
};

bool get_can_frame(CAN_UDP_t *payload)
{
    uint8_t map = (uint8_t)can_frame_bit_map;
    if (!map)
        return 0;

    payload->bitmap = map;
    payload->flag = CAN_FRAME_DATA;

    for (int i = 0; i < CAN_CHANNEL_NUM; i++) {
        if (map & (1U << i)) {
            CAN_Frame_t *frame = put_next_can_frame_buffer(i);
            if (frame != NULL) {
                payload->frame[i].ch = frame->ch;
                payload->frame[i].fd = frame->Msg.frameMode;
                payload->frame[i].dlc = frame->Msg.dataLengthCode;
                payload->frame[i].messageId = frame->Msg.messageId;
                memcpy(payload->frame[i].data, frame->data, G_DLC_LOOKUP_TABLE(payload->frame[i].dlc));
                udp_send_frame_cnt++;
                can_frame_read_done(i);
            }
        }
    }

    return 1;
}

static inline can_frame_ringbuf_state_e ringbuffer_status(int ch)
{
    if (can_ring_index[ch].read_index == can_ring_index[ch].write_index) {
        if (can_ring_index[ch].read_mirror == can_ring_index[ch].write_mirror)
            return CAN_FRAME_EMPTY;
        else
            return CAN_FRAME_FULL;
    }
    return CAN_FRAME_HALFFULL;
}

CAN_Frame_t* get_next_can_frame_buffer(int ch)
{
    if (CAN_FRAME_FULL == ringbuffer_status(ch))
        return NULL;

    if (can_ring_index[ch].write_index == CAN_STORE_FRAME_NUM - 1) {
        can_ring_index[ch].write_index = 0;
        can_ring_index[ch].write_mirror = ~can_ring_index[ch].write_mirror;
    }

    CAN_Frame_t *frame = &can_frame[ch][can_ring_index[ch].write_index];

    return frame;
}


void can_frame_write_done(int ch)
{
    can_ring_index[ch].write_index++;

    Ifx__imaskldmst(&can_frame_bit_map, 1, ch, 1);
}

CAN_Frame_t* put_next_can_frame_buffer(int ch)
{
    if (CAN_FRAME_EMPTY == ringbuffer_status(ch))
        return NULL;
    
    if (can_ring_index[ch].read_index == CAN_STORE_FRAME_NUM - 1) {
        can_ring_index[ch].read_index = 0;
        can_ring_index[ch].read_mirror = ~can_ring_index[ch].read_mirror;
    }

    CAN_Frame_t *frame = &can_frame[ch][can_ring_index[ch].read_index];

    return frame;
}

void can_frame_read_done(int ch)
{
    can_ring_index[ch].read_index++;
    if (CAN_FRAME_EMPTY == ringbuffer_status(ch)) {
        Ifx__imaskldmst(&can_frame_bit_map, 0, ch, 1);
    }
}


/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/


device_t channel_find_hwcan(uint8_t channel)
{
    for (int i = 0; i < sizeof(can_devices) / sizeof(can_devices[0]); i++) {
        if (channel == can_devices[i].channel) {
            return &can_devices[i].dev;
        }
    }
    return NULL;
}

device_t ifindex_find_hwcan(uint8_t ifindex)
{
    for (int i = 0; i < sizeof(can_devices)/sizeof(can_devices[0]); i++) {
        if (can_devices[i].ifindex == ifindex) {
            return &can_devices[i].dev;
        }
    }
    return NULL;
}

//static f_err_t control_resp(device_t dev, void *buffer, size_t size)
//{
//    LOG_DEBUG(LOG_MODULE_CAN, "device_%s control_resp\r\n",dev->name);
//    if (dev == NULL || buffer == NULL) {
//        LOG_ERROR(LOG_MODULE_CAN, "(control_resp)Invalid parameters: dev=%p, buffer=%p", dev, buffer);
//        return ERROR;
//    }
//
//    Item item;
//    item.data = buffer;
//    item.len  = size;
//
//#if HAVE_BUFFER
//    dev->encode(&item);
//
//    device_t device;
//    device = device_find("lan7801");
//    device_write(device, 0, item.data, item.len - 1);
//#else
//    assert(dev->ring_buf != NULL);
//    size_t len = ringbuffer_put(dev->ring_buf, (uint8_t *)&item, sizeof(Item));
//    if(len == 0)
//    {
//        LOG_ERROR(LOG_MODULE_CAN, "(control_resp)ringbuffer_put faild");
//        return ERROR;
//    }
//
//    lock(&dev->spinlock);
//    dev->frames++;
//    unlock(&dev->spinlock);
//#endif
//    return EOK;
//}

/* Forward declaration */
f_err_t can_control(device_t dev, int cmd, void *args);

void can_default_cfg(device_t dev)
{
    CAN_Config_t ctl = {.open = 1, .is_can_fd = 0, .bitrate = 500000};
    can_control(dev, 0, (void *)(&ctl));
}

f_err_t can_init(device_t dev)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_CAN, "(can_init)Invalid parameters: dev=%p", dev);
        return ERROR;
    }

	return EOK;
}

f_err_t can_open(device_t dev, uint16_t oflag)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_CAN, "(can_open)Invalid parameters: dev is NULL");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_CAN, "device_%s can_open\r\n",dev->name);

    const uint8 device_id = dev->device_id;
    if (!switchCanNode(device_id, can_nodeconfig)) {
        return ERROR;
    }

    can_devices[device_id].if_fd = (can_nodeconfig.frame.mode != 0);

	return EOK;
}

f_err_t can_close(device_t dev)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_CAN, "(can_close)Invalid parameters: dev is NULL");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_CAN, "device_%s can_close\r\n",dev->name);

    const uint8 device_id = dev->device_id;
    if (!switchCanNode(device_id, can_nodeconfig)) {
        return ERROR;
    }

	return EOK;
}

f_err_t can_read(device_t dev, off_t pos, void *buffer, size_t size)
{
    //Not for now
	return 0;
}

f_err_t can_write(device_t dev, off_t pos, const void *buffer, size_t size)
{
    CAN_Frame_t frame;
    CAN_UDP_Frame_t *input = (CAN_UDP_Frame_t *)buffer;

    if (dev == NULL || buffer == NULL) {
        return ERROR;
    }

   IfxCan_Can_initMessage(&frame.Msg);
   frame.ch = input->ch;
   frame.Msg.messageId = input->messageId;
   frame.Msg.dataLengthCode = (IfxCan_DataLengthCode)input->dlc;
   frame.Msg.frameMode = (IfxCan_FrameMode)input->fd;
   memcpy(frame.data, input->data, G_DLC_LOOKUP_TABLE(frame.Msg.dataLengthCode));

   if(transmitMsg(frame.ch, &frame) != IfxCan_Status_ok) {
       return ERROR;
   }

	return EOK;
}

f_err_t can_control(device_t dev, int cmd, void *args)
{
    f_err_t ret = ERROR;
    CAN_Config_t *node_config;

    if (dev == NULL || args == NULL) {
        return ret;
    }

    node_config = (CAN_Config_t *)args;

    if (strcmp(dev->name, "cansleep") != 0) {
        memset(&can_nodeconfig, 0, sizeof(IfxCan_Can_NodeConfig));
        can_nodeconfig = configCanNode(dev->device_id, node_config);
        if (&can_nodeconfig == NULL) {
            return ret;
        }

        ret = node_config->open ? device_open(dev, DEVICE_OFLAG_RDWR) : device_close(dev);

        if (ret != EOK) {
            return ret;
        }
    } else {
        // ret = Can_Sleep();
    }

    return ret;
}


f_err_t can_rx_irq(device_t dev, void *buffer)
{
    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_CAN, "Invalid parameters: dev or buffer is NULL\r\n");
        return ERROR;
    }



	// CanInstance_Type *recv_can_msg = (CanInstance_Type *)buffer;
	// if(recv_can_msg == NULL)
    // {
	//     LOG_ERROR(LOG_MODULE_CAN, "Invalid parameters: recv_can_msg is NULL\r\n");
    //     return ERROR;
    // }
	// RcpMessage *message;
    // message = build_canframe(dev->device_id, recv_can_msg, can_devices[dev->device_id].if_fd);
    // if (message == NULL) {
    //     LOG_ERROR(LOG_MODULE_CAN, "can_rx_irq rcp_build_message failed\r\n");
    //     return ERROR;
    // }

    // Item item;
    // item.data = (void *)message;
    // item.len = RCPMESSAGE_HEAD_SIZE + (can_devices[dev->device_id].if_fd ? sizeof(CanfdFrame_t) : sizeof(CanFrame_t));

    // assert(dev->ring_buf != NULL);
    // size_t len = ringbuffer_put(dev->ring_buf, (uint8_t *)&item, sizeof(item));
    // if(len == 0)
    // {
    //     LOG_ERROR(LOG_MODULE_CAN, "(can_rx_irq)ringbuffer_put faild");
    //     free(message);
    //     return ERROR;
    // }

    // lock(&dev->spinlock);
    // dev->frames++;
    // unlock(&dev->spinlock);

    return EOK;
}

f_err_t can_encode(Item *item)
{
    if (item == NULL) {
        LOG_ERROR(LOG_MODULE_CAN, "Invalid parameters: item is NULL\r\n");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_CAN, "can_encode\r\n");
    RcpMessage   *message = (RcpMessage *)item->data;

    //Only data frames's sequenceCounter will increase
    static uint16_t seq_count = 0;
    if(message->which_payload == RcpMessage_can_frame_tag || message->which_payload == RcpMessage_canfd_frame_tag)
        message->sequenceCounter = seq_count++;

    uint8_t *serialized_data = NULL;
    size_t encoded_size = 0;
    size_t buffer_size = 128;

    serialized_data = malloc(buffer_size);
    if (serialized_data == NULL) {
        LOG_ERROR(LOG_MODULE_CAN, "serialized_data malloc failed!\r\n");
        return ERROR;
    }

    if (serialize_message(serialized_data, buffer_size, message, &encoded_size, Data_Type_CAN_TYPE)) {
        LOG_DEBUG(LOG_MODULE_CAN, "serialize SUCCESS!\r\n");

        free(item->data);
        item->data = serialized_data;
        item->len = encoded_size;

        return EOK;
    } else {
        LOG_ERROR(LOG_MODULE_CAN, "serialize FAIL!!!\r\n");
        free(serialized_data);
        return ERROR;
    }
}

f_err_t can_decode(Item *item)
{
    return EOK;
}

struct device_ops can_ops = {
    .init    = can_init,
    .open    = can_open,
    .close   = can_close,
    .read    = can_read,
    .write   = can_write,
    .control = can_control
};

f_err_t hw_can_register(device_t device, const char* name,
                         uint32_t flag, uint8_t channel, struct can_device *can)
{
    assert(device != NULL);

    device->device_id       = channel;
    device->type        	= Device_Class_CAN;
    device->rx_indicate 	= can_rx_irq;
    device->tx_complete 	= NULL;
    device->ops             = &can_ops;
    device->user_data       = NULL;
    device->frames          = 0;
    device->encode          = can_encode;
    device->decode          = can_decode;
    device->spinlock        = &can_spinLock;

    // Bypass schedule core, just for test.
    device->immediate_trans = true;
    device->loopback        = false;

    // Initialize the circular buffer (failure check)
    /* Using the static allocation method is safer. */
    device->ring_buf        = can_frame[channel];

    /* register a character device */
    return device_register(device, name, DEVICE_FLAG_RDWR | flag);
}

void hw_can_init(void)
{
	/* init CAN PHY */
    // init_CAN_PHY();

	/* init CAN hardware */
    init_Mcmcan_module();

    /* register CAN0 device */
    for (int i = 0; i < sizeof(can_devices) / sizeof(can_devices[0]); i++) {
       if (hw_can_register(&can_devices[i].dev,
                           can_devices[i].name,
                           can_devices[i].flags,
                           can_devices[i].channel,
                           NULL) != EOK) {
           LOG_ERROR(LOG_MODULE_CAN, "CAN device %s init failed", can_devices[i].name);
       }
    }
}
