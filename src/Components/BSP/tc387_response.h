#ifndef __TC387_RESPONSE_H__
#define __TC387_RESPONSE_H__
#include <stdint.h>

typedef enum {
    CONFIG_SUCCESS_UDP = 0,                    // Configuration success
    CONFIG_ERROR_CAN_UDP = 1,                  // CAN configuration error
    CONFIG_ERROR_LIN_UDP = 2,                  // LIN configuration error
    CONFIG_ERROR_FLEXRAY_UDP = 3,              // FlexRay configuration error
    CONFIG_ERROR_LIN_SCHEDULE_TABLE_UDP = 4    // LIN schedule table configuration error
} ConfigResult_UDP_t;

typedef struct {
    uint8_t result;
} Config_Response_t;

#endif