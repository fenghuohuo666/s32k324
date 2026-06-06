#ifndef COMPILERTASKING_H
#define COMPILERTASKING_H

/* Minimal stub for Tasking compiler directives - GCC compatible */
#define IFX_EXTERN extern
#ifndef IFX_INLINE
#define IFX_INLINE inline
#endif
#define IFX_LOCAL_INLINE static inline

#endif /* COMPILERTASKING_H */
