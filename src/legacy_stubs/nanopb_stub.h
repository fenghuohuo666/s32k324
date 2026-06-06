#ifndef NANOPB_STUB_H
#define NANOPB_STUB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Remote interface indices (from original TC387 protobuf definitions) */
typedef enum {
    RemoteIfIndex_t_CAN0 = 0,
    RemoteIfIndex_t_CAN1,
    RemoteIfIndex_t_CAN2,
    RemoteIfIndex_t_CAN3,
    RemoteIfIndex_t_CAN4,
    RemoteIfIndex_t_CAN5,
    RemoteIfIndex_t_CAN6,
    RemoteIfIndex_t_CAN7,
    RemoteIfIndex_t_CAN_SLEEP,
    RemoteIfIndex_t_LIN0,
    RemoteIfIndex_t_LIN1,
    RemoteIfIndex_t_LIN2,
    RemoteIfIndex_t_LIN3,
    RemoteIfIndex_t_LIN4,
    RemoteIfIndex_t_LIN5,
    RemoteIfIndex_t_LIN6,
    RemoteIfIndex_t_LIN7,
    RemoteIfIndex_t_FIRMWARE_UPDATE
} RemoteIfIndex_t;

/* Data type tags for serialization */
typedef enum {
    Data_Type_CAN_TYPE = 0,
    Data_Type_CANFD_TYPE,
    Data_Type_LIN_TYPE,
    Data_Type_OTA_TYPE
} Data_Type_t;

/* OTA transfer data request - alias to match original naming */
#include "ota.h"
typedef Ota_Transfer_Data_Req_t OTA_TRANSFER_DATA_REQ_t;

/* RcpMessage payload tags */
#define RcpMessage_can_frame_tag     1
#define RcpMessage_canfd_frame_tag   2
#define RcpMessage_lin_frame_tag     3

/* Minimal RcpMessage stub */
typedef struct {
    uint32_t which_payload;
    uint32_t sequenceCounter;
    union {
        struct { uint8_t data[64]; uint32_t id; uint8_t dlc; } can_frame;
        struct { uint8_t data[64]; uint32_t id; uint8_t dlc; } canfd_frame;
        struct { uint8_t data[8];  uint32_t id; uint8_t dlc; } lin_frame;
    } payload;
} RcpMessage;

/* Stub serialization function - always fails (returns 0) since nanopb is excluded */
static inline int serialize_message(uint8_t *buffer, size_t buffer_size, RcpMessage *message, size_t *encoded_size, Data_Type_t type)
{
    (void)buffer;
    (void)buffer_size;
    (void)message;
    (void)encoded_size;
    (void)type;
    return 0; /* fail - no nanopb */
}

#endif /* NANOPB_STUB_H */
