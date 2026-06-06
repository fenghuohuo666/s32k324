#ifndef ASCLIN_H
#define ASCLIN_H

#include <stdint.h>
#include "def.h"
#include "tc387_response.h"

/* Stub for ASC LIN */
#define LIN_CHANNEL_NUM 2

typedef struct {
    uint8_t channel;
    uint8_t pid;
    uint8_t dir;
    uint8_t dl;
    uint8_t data[8];
} LIN_UDP_Frame_t;

typedef struct {
    uint8_t cmd;
    uint8_t open;
    uint8_t lin_mode;
    uint8_t schedule_flag;
    uint32_t response_timeout;
    uint32_t bitrate;
} LIN_UDP_Config_t;

typedef struct {
    uint8_t cmd;
    uint8_t channel;
    uint8_t pid;
    uint8_t dir;
    uint8_t dl;
    uint8_t _pad[3];
    uint8_t data[8];
} LIN_UDP_ConfigScheduleTable_t;

typedef struct {
    uint8_t flag;
    uint8_t pad;
    uint16_t bitmap;
    uint32_t udpcounter;
    union {
        LIN_UDP_Frame_t frame[LIN_CHANNEL_NUM];
        LIN_UDP_Config_t ctl[LIN_CHANNEL_NUM];
        LIN_UDP_ConfigScheduleTable_t ctlScheTab[LIN_CHANNEL_NUM];
        Config_Response_t response[LIN_CHANNEL_NUM];
    } payload;
} LIN_UDP_t;

static inline void hw_ascLin_init(void) {}
static inline device_t channel_find_hwlin(uint8_t num) { (void)num; return NULL; }
bool get_lin_frame(LIN_UDP_t *payload);

#endif /* ASCLIN_H */
