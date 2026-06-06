#ifndef IFXPORT_H
#define IFXPORT_H
#include "Ifx_Types.h"
typedef enum { IfxPort_InputMode_noPullDevice = 0, IfxPort_InputMode_pullDown, IfxPort_InputMode_pullUp } IfxPort_InputMode;
typedef enum { IfxPort_OutputMode_pushPull = 0, IfxPort_OutputMode_openDrain } IfxPort_OutputMode;
typedef enum { IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0 } IfxPort_PadDriver;
void IfxPort_setPinMode(void *port, uint8 pinIndex, IfxPort_OutputMode mode);
void IfxPort_setPinPadDriver(void *port, uint8 pinIndex, IfxPort_PadDriver driver);
void IfxPort_setPinHigh(void *port, uint8 pinIndex);
void IfxPort_setPinLow(void *port, uint8 pinIndex);
#endif
