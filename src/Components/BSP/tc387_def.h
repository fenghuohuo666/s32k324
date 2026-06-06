#ifndef __TC387_DEF_H_
#define __TC387_DEF_H_

#include <stdint.h>
#include <stdbool.h>

/* Stub out Infineon types for S32K324 */
#define TC3XX
/* uart debug */
#define USE_DEBUG_UART              0
/* cpu load monitor */
#define USE_CPU0_LOAD               0
#define USE_CPU1_LOAD               0
#define USE_CPU2_LOAD               0
#define USE_CPU3_LOAD               0
/* can */
#define USE_CAN                     0

/* lin */
#define USE_LIN                     0

/* ota */
#define USE_OTA                     0

/* UDP communicationm port*/
#define UDP_CAN_PORT                2345
#define UDP_LIN_PORT                2346
#define UDP_OTA_PORT                2349
#define UDP_TEMP_PORT               2350

//Pin define
#define LM25148_EN                  &MODULE_P10,0
#define LAN7801_REST                &MODULE_P15,8

//TIMER define
#define CPU_FULL_LOAD                       (333300)
#if (USE_CPU0_LOAD)
    #define CPU0_LOADER_TIMER               (&MODULE_STM0)
#endif
#if (USE_CPU1_LOAD)
    #define CPU1_LOADER_TIMER               (&MODULE_STM1)
#endif
#if (USE_CPU2_LOAD)
    #define CPU2_LOADER_TIMER               (&MODULE_STM2)
#endif
#if (USE_CPU3_LOAD)
    #define CPU3_LOADER_TIMER               (&MODULE_STM3)
#endif

#define UNUSED(x) (void)(x)

typedef volatile uint32_t spinLock;

/* Stub for SCU swap control register used in schedule.c */
typedef struct {
    struct {
        uint32_t ADDRCFG;
    } B;
} SCU_SWAPCTRL_t;

extern SCU_SWAPCTRL_t SCU_SWAPCTRL;

#endif /* __TC387_DEF_H_ */
