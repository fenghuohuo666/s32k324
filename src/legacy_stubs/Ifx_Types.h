#ifndef IFX_TYPES_H
#define IFX_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* NXP PlatformTypes.h and our stubs both define these basic types.
 * This pragma suppresses -Wpedantic for typedef redefinitions
 * in the remainder of this translation unit. */
#pragma GCC diagnostic ignored "-Wpedantic"

typedef bool     boolean;
typedef float    float32;
typedef double   float64;
typedef int8_t   sint8;
typedef int16_t  sint16;
typedef int32_t  sint32;
typedef int64_t  sint64;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint32_t Ifx_TickTime;

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

#ifndef IFX_INLINE
#define IFX_INLINE static inline
#endif

/* IfxCpu_spinLock is defined in IfxCpu.h */
/* Note: tc387_def.h defines spinLock separately for legacy code */

typedef struct
{
    uint32_t timeout;
} Ifx_Stm_CompareConfig;

typedef Ifx_Stm_CompareConfig IfxStm_CompareConfig;

#define TIME_INFINITE ((Ifx_TickTime)0xFFFFFFFF)

#endif /* IFX_TYPES_H */
