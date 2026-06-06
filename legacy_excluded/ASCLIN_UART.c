/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-24     Alex_min     first version
 */

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include <stdint.h>
#include "Ifx_Types.h"
#include "IfxAsclin_Asc.h"
#include "Ifx_Shell.h"
#include "Ifx_Console.h"
#include "IfxPort.h"
#include "ASCLIN_UART.h"
#include "tc387_irq_priority.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* Communication parameters */
#define ASC_TX_BUFFER_SIZE          256                                     /* Define the TX buffer size in byte    */
#define ASC_RX_BUFFER_SIZE          256                                     /* Define the RX buffer size in byte    */
#define ASC_BAUDRATE                115200                                  /* Define the UART baud rate            */

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initSerialInterface(void);

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
IfxStdIf_DPipe  g_ascStandardInterface;                                     /* Standard interface object            */
IfxAsclin_Asc   g_asclin;                                                   /* ASCLIN module object                 */

/* The transfer buffers allocate memory for the data itself and for FIFO runtime variables.
 * 8 more bytes have to be added to ensure a proper circular buffer handling independent from
 * the address to which the buffers have been located.
 */
uint8 g_uartTxBuffer[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
uint8 g_uartRxBuffer[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];


/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* Macro to define Interrupt Service Routine.
 * This macro makes following definitions:
 * 1) Define linker section as .intvec_tc<vector number>_<interrupt priority>.
 * 2) define compiler specific attribute for the interrupt functions.
 * 3) define the Interrupt service routine as ISR function.
 *
 * IFX_INTERRUPT(isr, vectabNum, priority)
 *  - isr: Name of the ISR function.
 *  - vectabNum: Vector table number.
 *  - priority: Interrupt priority. Refer Usage of Interrupt Macro for more details.
 */
IFX_INTERRUPT(asc0TxISR, 0, (uint8_t)ISR_PRIORITY_DEBUG_UART_TX);
void asc0TxISR(void)
{
    IfxStdIf_DPipe_onTransmit(&g_ascStandardInterface);
}

IFX_INTERRUPT(asc0RxISR, 0, (uint8_t)ISR_PRIORITY_DEBUG_UART_RX);
void asc0RxISR(void)
{
    IfxStdIf_DPipe_onReceive(&g_ascStandardInterface);
}

IFX_INTERRUPT(asc0ErrISR, 0, (uint8_t)ISR_PRIORITY_DEBUG_UART_ERR);
void asc0ErrISR(void)
{
    IfxStdIf_DPipe_onError(&g_ascStandardInterface);
}

/* Function to initialize ASCLIN module */
void initSerialInterface(void)
{
    IfxAsclin_Asc_Config ascConf;

    /* Set default configurations */
    IfxAsclin_Asc_initModuleConfig(&ascConf, &MODULE_ASCLIN0); /* Initialize the structure with default values      */

    /* Set the desired baud rate */
    ascConf.baudrate.baudrate = ASC_BAUDRATE;                                   /* Set the baud rate in bit/s       */
    ascConf.baudrate.oversampling = IfxAsclin_OversamplingFactor_16;            /* Set the oversampling factor      */

    /* Configure the sampling mode */
    ascConf.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three;             /* Set the number of samples per bit*/
    ascConf.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8;    /* Set the first sample position    */

    /* ISR priorities and interrupt target */
    ascConf.interrupt.txPriority = ISR_PRIORITY_DEBUG_UART_TX;  /* Set the interrupt priority for TX events             */
    ascConf.interrupt.rxPriority = ISR_PRIORITY_DEBUG_UART_RX;  /* Set the interrupt priority for RX events             */
    ascConf.interrupt.erPriority = ISR_PRIORITY_DEBUG_UART_ERR;  /* Set the interrupt priority for Error events          */
    ascConf.interrupt.typeOfService = IfxSrc_Tos_cpu0;

    /* Pin configuration */
    const IfxAsclin_Asc_Pins pins = {
            .cts        = NULL_PTR,                         /* CTS pin not used                                     */
            .ctsMode    = IfxPort_InputMode_pullUp,
            .rx         = &IfxAsclin0_RXA_P14_1_IN,         /* Select the pin for RX connected to the USB port      */
            .rxMode     = IfxPort_InputMode_pullUp,         /* RX pin                                               */
            .rts        = NULL_PTR,                         /* RTS pin not used                                     */
            .rtsMode    = IfxPort_OutputMode_pushPull,
            .tx         = &IfxAsclin0_TX_P14_0_OUT,         /* Select the pin for TX connected to the USB port      */
            .txMode     = IfxPort_OutputMode_pushPull,      /* TX pin                                               */
            .pinDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1
    };
    ascConf.pins = &pins;

    /* FIFO buffers configuration */
    ascConf.txBuffer = g_uartTxBuffer;                      /* Set the transmission buffer                          */
    ascConf.txBufferSize = ASC_TX_BUFFER_SIZE;              /* Set the transmission buffer size                     */
    ascConf.rxBuffer = g_uartRxBuffer;                      /* Set the receiving buffer                             */
    ascConf.rxBufferSize = ASC_RX_BUFFER_SIZE;              /* Set the receiving buffer size                        */

    /* Init ASCLIN module */
    IfxAsclin_Asc_initModule(&g_asclin, &ascConf);          /* Initialize the module with the given configuration   */
}

/* Function to initialize the Shell */
void early_console_init(void)
{
    /* Initialize the hardware peripherals */
    initSerialInterface();

    /* Initialize the Standard Interface */
    IfxAsclin_Asc_stdIfDPipeInit(&g_ascStandardInterface, &g_asclin);

}
