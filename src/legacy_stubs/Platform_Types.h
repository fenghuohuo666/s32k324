#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/* This stub shadows the NXP Platform_Types.h when legacy_stubs appears
 * earlier in the include path. The real NXP header is found via other
 * include paths in the S32DS build. If this stub is picked up first,
 * it simply falls back to Ifx_Types.h which provides the same types. */
#include "Ifx_Types.h"

#endif /* PLATFORM_TYPES_H */
