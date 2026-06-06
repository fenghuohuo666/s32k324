/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-10     Wyj          Flash
 */

#ifndef _FLASH_H_
#define _FLASH_H_

#include <stdint.h>
#include <stdio.h>
#include "IfxFlash.h"
#include "Ifx_Types.h"
/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define PFLASH_PAGE_LENGTH          IFXFLASH_PFLASH_PAGE_LENGTH /* 0x20 = 32 Bytes (smallest unit that can be
                                                                 * programmed in the Program Flash memory (PFLASH)) */
#define FLASH_MODULE                0                           /* Macro to select the flash (PMU) module           */

#define PARTITION_A                 0
#define PARTITION_B                 1
#define PFLASH_ZoneA_STARTING_ADDRESS   0xA0600000              /* Address of the PFLASH where the data is written  */
#define PFLASH_ZoneB_STARTING_ADDRESS   0xA0000000              /* Address of the PFLASH where the data is written  */
#define FLASH_SIZE                  (3 * 1024 * 1024)    // 3MB
#define ERASE_BLOCK_SIZE            (16 * 1024)          // 16KB
#define PFLASH_NUM_SECTORS          1                          /* Number of PFLASH sectors to be erased            */

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initLEDs(void);            /* Function that initializes the LEDs                                               */
/* Copy all the needed functions to the PSPR memory to avoid overwriting them during the flash execution */
/* Erase all the required sectors at one time(16) */
boolean initProgramFlash(uint8 partition);
/* Function that flashes the Program Flash memory calling the routines from the PSPR*/
void writeProgramFlash(uint32 address, uint8 *data);

#endif /* _FLASH_H_ */
