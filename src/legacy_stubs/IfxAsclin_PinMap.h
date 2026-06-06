#ifndef IFXASCLIN_PINMAP_H
#define IFXASCLIN_PINMAP_H

#include <stdint.h>

/* Stub pin definitions for ASCLIN - replace with real S32K324 pinmux when implementing LIN/UART */
#define IFXASCLIN_PIN(index, pin) ((uint32_t)(index))

#define IfxAsclin0_RXB_P15_3_IN  IFXASCLIN_PIN(0, 0)
#define IfxAsclin0_TX_P15_2_OUT  IFXASCLIN_PIN(0, 1)
#define IfxAsclin1_RXA_P15_1_IN  IFXASCLIN_PIN(1, 0)
#define IfxAsclin1_TX_P15_0_OUT  IFXASCLIN_PIN(1, 1)
#define IfxAsclin2_RXC_P02_10_IN IFXASCLIN_PIN(2, 0)
#define IfxAsclin2_TX_P02_9_OUT  IFXASCLIN_PIN(2, 1)
#define IfxAsclin3_RXA_P15_7_IN  IFXASCLIN_PIN(3, 0)
#define IfxAsclin3_TX_P15_6_OUT  IFXASCLIN_PIN(3, 1)
#define IfxAsclin4_RXA_P00_12_IN IFXASCLIN_PIN(4, 0)
#define IfxAsclin4_TX_P00_9_OUT  IFXASCLIN_PIN(4, 1)
#define IfxAsclin5_RXA_P00_6_IN  IFXASCLIN_PIN(5, 0)
#define IfxAsclin5_TX_P00_7_OUT  IFXASCLIN_PIN(5, 1)
#define IfxAsclin6_RXA_P23_3_IN  IFXASCLIN_PIN(6, 0)
#define IfxAsclin6_TX_P23_5_OUT  IFXASCLIN_PIN(6, 1)
#define IfxAsclin7_RXF_P22_4_IN  IFXASCLIN_PIN(7, 0)
#define IfxAsclin7_TX_P22_1_OUT  IFXASCLIN_PIN(7, 1)
#define IfxAsclin8_RXD_P33_6_IN  IFXASCLIN_PIN(8, 0)
#define IfxAsclin8_TX_P33_7_OUT  IFXASCLIN_PIN(8, 1)
#define IfxAsclin9_RXA_P01_5_IN  IFXASCLIN_PIN(9, 0)
#define IfxAsclin9_TX_P01_7_OUT  IFXASCLIN_PIN(9, 1)
#define IfxAsclin10_RXD_P13_1_IN IFXASCLIN_PIN(10, 0)
#define IfxAsclin10_TX_P13_0_OUT IFXASCLIN_PIN(10, 1)
#define IfxAsclin11_RXD_P21_1_IN IFXASCLIN_PIN(11, 0)
#define IfxAsclin11_TX_P21_0_OUT IFXASCLIN_PIN(11, 1)
#define IfxAsclin13_RXB_P00_11_IN IFXASCLIN_PIN(13, 0)
#define IfxAsclin13_TX_P00_10_OUT IFXASCLIN_PIN(13, 1)
#define IfxAsclin14_RXA_P33_3_IN IFXASCLIN_PIN(14, 0)
#define IfxAsclin14_TX_P33_2_OUT IFXASCLIN_PIN(14, 1)
#define IfxAsclin15_RXA_P32_4_IN IFXASCLIN_PIN(15, 0)
#define IfxAsclin15_TX_P32_7_OUT IFXASCLIN_PIN(15, 1)
#define IfxAsclin16_RXB_P23_7_IN IFXASCLIN_PIN(16, 0)
#define IfxAsclin16_TX_P23_6_OUT IFXASCLIN_PIN(16, 1)

#endif /* IFXASCLIN_PINMAP_H */
