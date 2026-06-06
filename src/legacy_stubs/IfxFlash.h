#ifndef IFXFLASH_H
#define IFXFLASH_H

#include <stdint.h>
#include <stdbool.h>

#define IFXFLASH_PFLASH_PAGE_LENGTH 32

typedef enum {
    IfxFlash_FlashType_D0 = 0,
    IfxFlash_FlashType_P0,
    IfxFlash_FlashType_P1,
    IfxFlash_FlashType_P2
} IfxFlash_FlashType;

static inline void IfxFlash_enterPageMode(uint32_t pageAddr) { (void)pageAddr; }
static inline void IfxFlash_eraseMultipleSectors(uint32_t sectorAddr, IfxFlash_FlashType flashType) { (void)sectorAddr; (void)flashType; }
static inline void IfxFlash_loadPage2X32(uint32_t pageAddr, uint32_t wordL, uint32_t wordU) { (void)pageAddr; (void)wordL; (void)wordU; }
static inline uint8_t IfxFlash_verifyErasedPage(uint32_t pageAddr) { (void)pageAddr; return 0; }
static inline void IfxFlash_waitUnbusy(uint32_t flash, IfxFlash_FlashType flashType) { (void)flash; (void)flashType; }
static inline void IfxFlash_writePage(uint32_t pageAddr) { (void)pageAddr; }

#endif /* IFXFLASH_H */
