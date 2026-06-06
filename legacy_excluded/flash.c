/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-10     Wyj          Flash
 */

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include <string.h>
#include <stdbool.h>
#include "IfxCpu.h"
#include "IfxScu_reg.h"

#include "flash.h"
/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* Reserved space for erase and program routines in bytes */
#define ERASESECTOR_LEN             (110)
#define WAITUNBUSY_LEN              (110)
#define ENTERPAGEMODE_LEN           (110)
#define LOADPAGE2X32_LEN            (110)
#define WRITEPAGE_LEN               (110)
#define ERASEPFLASH_LEN             (0x186)
#define WRITEPFLASH_LEN             (0x228)

/* Relocation address for the erase and program routines: Program Scratch-Pad SRAM (PSPR) of CPU0 */
/* psram0: 0x70100000
 * psram1: 0x60100000
 * psram2: 0x50100000
 * psram3: 0x40100000
 * psram4: 0x30100000
 * psram5: 0x10100000
 */
#define RELOCATION_START_ADDR       (0x50100000U)

/* Definition of the addresses where to relocate the erase and program routines, given their reserved space */
#define ERASESECTOR_ADDR            (RELOCATION_START_ADDR)
#define WAITUNBUSY_ADDR             (ERASESECTOR_ADDR + ERASESECTOR_LEN)
#define ENTERPAGEMODE_ADDR          (WAITUNBUSY_ADDR + WAITUNBUSY_LEN)
#define LOAD2X32_ADDR               (ENTERPAGEMODE_ADDR + ENTERPAGEMODE_LEN)
#define WRITEPAGE_ADDR              (LOAD2X32_ADDR + LOADPAGE2X32_LEN)
#define ERASEPFLASH_ADDR            (WRITEPAGE_ADDR + WRITEPAGE_LEN)
#define WRITEPFLASH_ADDR            (ERASEPFLASH_ADDR + ERASEPFLASH_LEN)

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
static void erasePFLASH(uint32 sectorAddr);
static void writePFLASH(uint32 startingAddr, uint8 *data, IfxFlash_FlashType flash_type);
static void copyFunctionsToPSPR(void);

typedef struct
{
    void (*eraseSectors)(uint32 sectorAddr, uint32 numSector);
    uint8 (*waitUnbusy)(uint32 flash, IfxFlash_FlashType flashType);
    uint8 (*enterPageMode)(uint32 pageAddr);
    void (*load2X32bits)(uint32 pageAddr, uint32 wordL, uint32 wordU);
    void (*writePage)(uint32 pageAddr);
    void (*eraseFlash)(uint32 sectorAddr);
    void (*writeFlash)(uint32 startingAddr, uint8 *data, IfxFlash_FlashType flash_type);
} Function;

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
Function g_commandFromPSPR;
uint8    g_tmp_data[64] = {0};
static IfxFlash_FlashType  g_flash_type;
/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* This function copies the erase and program routines to the Program Scratch-Pad SRAM (PSPR) of the CPU0 and assigns
 * function pointers to them.
 */
static void copyFunctionsToPSPR()
{
    /* Copy the IfxFlash_eraseMultipleSectors() routine and assign it to a function pointer */
    memcpy((void *)ERASESECTOR_ADDR, (const void *)IfxFlash_eraseMultipleSectors, ERASESECTOR_LEN);
    g_commandFromPSPR.eraseSectors = (void *)ERASESECTOR_ADDR;

    /* Copy the IfxFlash_waitUnbusy() routine and assign it to a function pointer */
    memcpy((void *)WAITUNBUSY_ADDR, (const void *)IfxFlash_waitUnbusy, WAITUNBUSY_LEN);
    g_commandFromPSPR.waitUnbusy = (void *)WAITUNBUSY_ADDR;

    /* Copy the IfxFlash_enterPageMode() routine and assign it to a function pointer */
    memcpy((void *)ENTERPAGEMODE_ADDR, (const void *)IfxFlash_enterPageMode, ENTERPAGEMODE_LEN);
    g_commandFromPSPR.enterPageMode = (void *)ENTERPAGEMODE_ADDR;

    /* Copy the IfxFlash_loadPage2X32() routine and assign it to a function pointer */
    memcpy((void *)LOAD2X32_ADDR, (const void *)IfxFlash_loadPage2X32, LOADPAGE2X32_LEN);
    g_commandFromPSPR.load2X32bits = (void *)LOAD2X32_ADDR;

    /* Copy the IfxFlash_writePage() routine and assign it to a function pointer */
    memcpy((void *)WRITEPAGE_ADDR, (const void *)IfxFlash_writePage, WRITEPAGE_LEN);
    g_commandFromPSPR.writePage = (void *)WRITEPAGE_ADDR;

    /* Copy the erasePFLASH() routine and assign it to a function pointer */
    memcpy((void *)ERASEPFLASH_ADDR, (const void *)erasePFLASH, ERASEPFLASH_LEN);
    g_commandFromPSPR.eraseFlash = (void *)ERASEPFLASH_ADDR;

    /* Copy the erasePFLASH() routine and assign it to a function pointer */
    memcpy((void *)WRITEPFLASH_ADDR, (const void *)writePFLASH, WRITEPFLASH_LEN);
    g_commandFromPSPR.writeFlash = (void *)WRITEPFLASH_ADDR;
}

/* This function erases a given sector of the Program Flash memory. The function is copied in the PSPR through
 * copyFunctionsToPSPR(). Because of this, inside the function, only routines from the PSPR or inline functions
 * can be called, otherwise a Context Type (CTYP) trap can be triggered.
 */
static void erasePFLASH(uint32 sectorAddr)
{
    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();

    /* Erase the sector */
    IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);      /* Disable EndInit protection                   */
    g_commandFromPSPR.eraseSectors(sectorAddr, PFLASH_NUM_SECTORS); /* Erase the given sector                       */
    IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);        /* Enable EndInit protection                    */

    /* Wait until the sector is erased */
    g_commandFromPSPR.waitUnbusy(FLASH_MODULE, g_flash_type);
}

/* This function writes the Program Flash memory. The function is copied in the PSPR through copyFunctionsToPSPR().
 * Because of this, inside the function, only routines from the PSPR or inline functions can be called,
 * otherwise a Context Type (CTYP) trap can be triggered.
 */
static void writePFLASH(uint32 startingAddr, uint8 *data, IfxFlash_FlashType flash_type)
{
    uint32 offset;                  /* Variable to cycle over all the words in a page   */

    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();
    /* Enter in page mode */
    g_commandFromPSPR.enterPageMode(startingAddr);
    /* Wait until page mode is entered */
    g_commandFromPSPR.waitUnbusy(FLASH_MODULE, flash_type);

    /* Write the entire page (PFLASH_PAGE_LENGTH bytes) into the assembly buffer */
    for(offset = 0; offset < PFLASH_PAGE_LENGTH; offset += 0x8)     /* Loop over the page length */
    {
        uint32 wordL = 0, wordU = 0;
        uint32 remaining = PFLASH_PAGE_LENGTH - offset;

        if(remaining >= 8) {
            wordL = (uint32)data[offset] |
                    ((uint32)data[offset + 1] << 8) |
                    ((uint32)data[offset + 2] << 16) |
                    ((uint32)data[offset + 3] << 24);
            wordU = (uint32)data[offset + 4] |
                    ((uint32)data[offset + 5] << 8) |
                    ((uint32)data[offset + 6] << 16) |
                    ((uint32)data[offset + 7] << 24);
        }
        else {
            if (remaining >= 4) {
                wordL = (uint32)data[offset] |
                        ((uint32)data[offset + 1] << 8) |
                        ((uint32)data[offset + 2] << 16) |
                        ((uint32)data[offset + 3] << 24);
                if (remaining >= 5) {
                    wordU = (uint32)data[offset + 4];
                    if (remaining >= 6) wordU |= (uint32)data[offset + 5] << 8;
                    if (remaining >= 7) wordU |= (uint32)data[offset + 6] << 16;
                }
            } else {
                wordL = (uint32)data[offset];
                if (remaining >= 2) wordL |= (uint32)data[offset + 1] << 8;
                if (remaining >= 3) wordL |= (uint32)data[offset + 2] << 16;
            }
        }

        /* Load the two 32-bit words into the Flash assembly buffer */
        g_commandFromPSPR.load2X32bits(startingAddr + offset, wordL, wordU);
    }

    /* Write the page */
    IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);      /* Disable EndInit protection               */
    g_commandFromPSPR.writePage(startingAddr);                          /* Write the page                           */
    IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);        /* Enable EndInit protection                */

    /* Wait until the page is written in the Program Flash memory */
    g_commandFromPSPR.waitUnbusy(FLASH_MODULE, flash_type);
}

boolean initProgramFlash(uint8 partition)
{
    boolean interruptState = IfxCpu_disableInterrupts(); /* Get the current state of the interrupts and disable them*/

    /* Copy all the needed functions to the PSPR memory to avoid overwriting them during the flash execution */
    copyFunctionsToPSPR();

    uint32_t num_blocks = FLASH_SIZE/ERASE_BLOCK_SIZE;

    uint32_t start_address = 0x0;

    switch (partition) {
        case PARTITION_A:
            start_address = PFLASH_ZoneA_STARTING_ADDRESS;
            g_flash_type    = IfxFlash_FlashType_P2;
            break;
        case PARTITION_B:
            start_address = PFLASH_ZoneB_STARTING_ADDRESS;
            g_flash_type    = IfxFlash_FlashType_P0;
            break;
        default:
            break;
    }

    if(start_address == 0x0)
    {
        return false;
    }

    /* Erase the Program Flash sector before writing */
    for (uint32_t i = 0; i < num_blocks; i++) {
        uint32_t block_addr = start_address + (i * ERASE_BLOCK_SIZE);
        g_commandFromPSPR.eraseFlash(block_addr);
    }

    IfxCpu_restoreInterrupts(interruptState);            /* Restore the interrupts state */

    return true;
}

/* This function flashes the Program Flash memory calling the routines from the PSPR */
void writeProgramFlash(uint32 address, uint8 *data)
{
    boolean interruptState = IfxCpu_disableInterrupts(); /* Get the current state of the interrupts and disable them*/

    /* Write the Program Flash */
    g_commandFromPSPR.writeFlash(address, data, g_flash_type);

    IfxCpu_restoreInterrupts(interruptState);            /* Restore the interrupts state                            */
}
