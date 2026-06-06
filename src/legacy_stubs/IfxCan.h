#ifndef IFXCAN_H
#define IFXCAN_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    IfxCan_DataLengthCode_0 = 0,
    IfxCan_DataLengthCode_1,
    IfxCan_DataLengthCode_2,
    IfxCan_DataLengthCode_4,
    IfxCan_DataLengthCode_6,
    IfxCan_DataLengthCode_8,
    IfxCan_DataLengthCode_12,
    IfxCan_DataLengthCode_16,
    IfxCan_DataLengthCode_20,
    IfxCan_DataLengthCode_24,
    IfxCan_DataLengthCode_32,
    IfxCan_DataLengthCode_48,
    IfxCan_DataLengthCode_64
} IfxCan_DataLengthCode;

typedef enum {
    IfxCan_FrameMode_standard = 0,
    IfxCan_FrameMode_fdLong,
    IfxCan_FrameMode_fdLongAndFast
} IfxCan_FrameMode;

typedef enum {
    IfxCan_Status_ok = 0,
    IfxCan_Status_notInitialised,
    IfxCan_Status_wrongParam
} IfxCan_Status;

typedef struct {
    uint32_t messageId;
    uint32_t dataLengthCode;
    uint32_t frameMode;
    uint8_t  data[64];
} IfxCan_Message;

typedef struct {
    uint32_t dummy;
} IfxCan_Filter;

typedef struct {
    uint32_t mode;
} IfxCan_Can_NodeConfig_Frame;

typedef struct {
    IfxCan_Can_NodeConfig_Frame frame;
    IfxCan_Message Msg;
} IfxCan_Can_NodeConfig;

static inline void IfxCan_Can_initMessage(IfxCan_Message *msg) { (void)msg; }

#endif /* IFXCAN_H */
