/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-14     Wenjc        Demo
 */

#include <string.h>
#include "clock.h"
#include "ringbuffer.h"
#include "log.h"
#include "IfxCpu.h"
#include "IfxCpu_Irq.h"
#include "pb_type.h"
#include "CODEC.h"
#include "rcp_build.h"
#include "tc387_irq_priority.h"

ERUconfig g_ERUconfig;
struct clock_device clock_dev = {
        .name = "clock",
        .channel = 0,
        .ifindex = RemoteIfIndex_t_TIME_SYNC,
        .flags = DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM,
        .t1 = 0,
        .remote_time = 0
};

device_t ifindex_find_hwclock(uint8_t ifindex)
{
    return &clock_dev.dev;
}

/*
 * @brief Obtain the current system timestamp.
 *
 * @return Return the current system timestamp, The unit is microseconds.
 */
uint64_t get_timestamp(void)
{
    /* Get the system time (since the last reset of the microcontroller) in seconds */
    return (IfxStm_get(DEFAULT_TIMER) * US_PER_SEC / IfxStm_getFrequency(DEFAULT_TIMER));
}

// IFX_INTERRUPT(SCUERU_Int0_Handler, 0, ISR_PRIORITY_SCUERU_INT0);
/* Interrupt Service Routine */
void SCUERU_Int0_Handler(void)
{
    clock_dev.t1 = get_timestamp();                                      // Record the timestamp T1
    IfxPort_setPinState(LED, IfxPort_State_toggled);                     // Toggle LED
    //IfxScuEru_clearAllEventFlags();
}

void initPeripheralsAndERU()
{
    /* Initialize pins which are used to trigger and visualize the interrupt and set the default states */
    IfxPort_setPinMode(TRIGGER_PIN, IfxPort_Mode_outputPushPullGeneral);    /* Initialize TRIGGER_PIN port pin  */
    IfxPort_setPinMode(LED, IfxPort_Mode_outputPushPullGeneral);            /* Initialize LED port pin          */
    IfxPort_setPinState(TRIGGER_PIN, IfxPort_State_high);                    /* Turn off TRIGGER_PIN             */
    IfxPort_setPinState(LED, IfxPort_State_high);                           /* Turn off LED (LED is low active) */

    /* Trigger pin */
    g_ERUconfig.reqPin = REQ_IN; /* Select external request pin */

    /* Initialize this pin with pull-down enabled
     * This function will also configure the input multiplexers of the ERU (Register EXISx)
     */
    IfxScuEru_initReqPin(g_ERUconfig.reqPin, IfxPort_InputMode_pullDown);

    /* Determine input channel depending on input pin */
    g_ERUconfig.inputChannel = (IfxScuEru_InputChannel)g_ERUconfig.reqPin->channelId;

    /* Input channel configuration */
    IfxScuEru_enableRisingEdgeDetection(g_ERUconfig.inputChannel);          /* Interrupt triggers on
                                                                               rising edge (Register RENx) and  */
//    IfxScuEru_enableFallingEdgeDetection(g_ERUconfig.inputChannel);         /* on falling edge (Register FENx)  */

    /* Signal destination */
    g_ERUconfig.outputChannel = IfxScuEru_OutputChannel_0;                  /* OGU channel 0                    */
    /* Event from input ETL0 triggers output OGU0 (signal TRx0) */
    g_ERUconfig.triggerSelect = IfxScuEru_InputNodePointer_0;

    /* Connecting Matrix, Event Trigger Logic ETL block */
    /* Enable generation of trigger event (Register EIENx) */
    IfxScuEru_enableTriggerPulse(g_ERUconfig.inputChannel);
    /* Determination of output channel for trigger event (Register INPx) */
    IfxScuEru_connectTrigger(g_ERUconfig.inputChannel, g_ERUconfig.triggerSelect);

    /* Configure Output channels, OutputGating Unit OGU (Register IGPy) */
    IfxScuEru_setInterruptGatingPattern(g_ERUconfig.outputChannel, IfxScuEru_InterruptGatingPattern_alwaysActive);

    /* Service request configuration */
    /* Get source pointer depending on outputChannel (SRC_SCUERU0 for outputChannel0) */
    g_ERUconfig.src = &MODULE_SRC.SCU.SCUERU[(int)g_ERUconfig.outputChannel % 4];
    IfxSrc_init(g_ERUconfig.src, IfxSrc_Tos_cpu0, ISR_PRIORITY_SCUERU_INT0);
    IfxSrc_enable(g_ERUconfig.src);
}

// Provide the timestamp
uint64_t get_sync_timestamp(void)
{
    return ((get_timestamp() - clock_dev.t1) + clock_dev.remote_time);
}

static f_err_t control_resp(device_t dev, void *buffer, size_t size)
{
    LOG_DEBUG(LOG_MODULE_TIMESTAMP, "device_%s control_resp\r\n",dev->name);
    if (dev == NULL || buffer == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "(control_resp)Invalid parameters: dev=%p, buffer=%p", dev, buffer);
        return ERROR;
    }

    Item item;
    item.data = buffer;
    item.len  = size;

    if (dev->ring_buf == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "device_%s ring_buf is NULL\r\n", dev->name);
        return ERROR;
    }
    size_t len = ringbuffer_put(dev->ring_buf, (uint8_t *)&item, sizeof(Item));
    if(len == 0)
    {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "(control_resp)ringbuffer_put faild");
        return ERROR;
    }

    lock(&dev->spinlock);
    dev->frames++;
    unlock(&dev->spinlock);

    return EOK;
}

f_err_t clock_init(device_t dev)
{
    if (dev == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "(clock_init)Invalid parameters: dev=%p", dev);
        return ERROR;
    }
    initPeripheralsAndERU();

    return 0;
}

f_err_t clock_open(device_t dev, uint16_t oflag)
{
    return 0;
}

f_err_t clock_close(device_t dev)
{
    return 0;
}

f_err_t clock_read(device_t dev, off_t pos, void *buffer, size_t size)
{
    return 0;
}

f_err_t clock_write(device_t dev, off_t pos, const void *buffer, size_t size)
{
    return 0;
}

f_err_t clock_control(device_t dev, int cmd, void *args)
{
    f_err_t ret = ERROR;
    if (dev == NULL || args == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "Invalid parameters: dev or args is NULL\r\n");
        return EINVAL;
    }
    LOG_DEBUG(LOG_MODULE_TIMESTAMP, "device_%s clock_control\r\n",dev->name);

    TimeSync_t *time_sync = (TimeSync_t *)args;
    clock_dev.remote_time = time_sync->timestamp;

    TimeSyncResp_t resp;
    resp.error = MessageError_t_RCM_ERROR_NONE;
    RcpMessage *resp_msg = rcp_build_message(dev->device_id + RemoteIfIndex_t_TIME_SYNC,
            MessageFlagField_t_MESSAGE_FLAG_BIT_RESPONSE, &resp,
            sizeof(TimeSyncResp_t), RcpMessage_time_sync_resp_tag);
    if (resp_msg == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "clock_control rcp_build_message failed\r\n");
        return ret;
    }

    ret = control_resp(dev, (void *)resp_msg, RCPMESSAGE_HEAD_SIZE + sizeof(TimeSyncResp_t));
    if(ret != EOK)
    {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "control_resp failed\r\n");
    }
    return ret;
}

f_err_t clock_rx_irq(device_t dev, void *buffer)
{
    return EOK;
}

f_err_t clock_encode(Item *item)
{
    if (item == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "Invalid parameters: item is NULL\r\n");
        return ERROR;
    }
    LOG_DEBUG(LOG_MODULE_TIMESTAMP, "clock_encode\r\n");
    RcpMessage   *message = (RcpMessage *)item->data;

    static uint16_t seq_count = 0;
    if(message->which_payload == RcpMessage_time_sync_tag)
        message->sequenceCounter = seq_count++;

    uint8_t *serialized_data = NULL;
    size_t encoded_size = 0;
    size_t buffer_size = 128;

    serialized_data = malloc(buffer_size);
    if (serialized_data == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "serialized_data malloc failed!\r\n");
        return ERROR;
    }

    if (serialize_message(serialized_data, buffer_size, message, &encoded_size, Data_Type_CMD_TYPE)) {
        LOG_DEBUG(LOG_MODULE_TIMESTAMP, "serialize SUCCESS!\r\n");

        free(item->data);
        item->data = serialized_data;
        item->len = encoded_size;

        return EOK;
    } else {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "serialize FAIL!!!\r\n");
        free(serialized_data);
        return ERROR;
    }
}

f_err_t clock_decode(Item *item)
{
    return EOK;
}

struct device_ops clock_ops = {
    .init    = clock_init,
    .open    = clock_open,
    .close   = clock_close,
    .read    = clock_read,
    .write   = clock_write,
    .control = clock_control,
};

f_err_t hw_clock_register(device_t device, const char *name, uint32_t flag,
        uint8_t channel, struct clock_device *clock)
{
    assert(device != NULL);

    device->device_id       = channel;
    device->type            = Device_Class_Char;
    device->rx_indicate     = clock_rx_irq;
    device->tx_complete     = NULL;
    device->ops             = &clock_ops;
    device->user_data       = NULL;
    device->frames          = 0;
    device->encode          = clock_encode;
    device->decode          = clock_decode;
    device->spinlock        = 0;

    //Bypass schedule core, just for test.
    device->immediate_trans = true;
    device->loopback        = false;

    device->ring_buf = ringbuffer_create(CLOCK_RING_DEPTH);
    if (device->ring_buf == NULL) {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "Ring buffer creation failed for %s", name);
        return ERROR;
    }

    /* register a character device */
    return device_register(device, name, DEVICE_FLAG_RDWR | flag);
}

void hw_clock_init(void)
{
    LOG_DEBUG(LOG_MODULE_TIMESTAMP, "hw_clock_init\r\n");
    /* init clock hardware */

    /* register clock device */
    if (hw_clock_register(&clock_dev.dev,
                           clock_dev.name,
                           clock_dev.flags,
                           clock_dev.channel,
                           NULL) != EOK)
    {
        LOG_ERROR(LOG_MODULE_TIMESTAMP, "clock device %s init failed", clock_dev.name);
    }
}
