#ifndef IFXSRC_REGDEF_H
#define IFXSRC_REGDEF_H

#include <stdint.h>
#include "IfxSrc.h"

/* Minimal stub for Infineon SRC (Service Request Control) registers */
typedef struct {
    Ifx_SRC_SRCR ASCLIN[16];
} Ifx_SRC_ASCLIN;

#endif /* IFXSRC_REGDEF_H */
