/*
 * _sbrk() implementation for bare-metal S32K324
 * Links against the heap region defined in linker script:
 *   _heap_start / _heap_end (HEAP_SIZE = 0x2000)
 *
 * Once this file is compiled and linked, Newlib's malloc() becomes functional.
 */

#include <stddef.h>
#include <errno.h>

/* Linker script symbols: addresses of heap boundaries */
extern char _heap_start;
extern char _heap_end;

void *_sbrk(ptrdiff_t incr)
{
    static char *heap_end = NULL;
    char *prev_heap_end;

    if (heap_end == NULL) {
        heap_end = &_heap_start;
    }

    prev_heap_end = heap_end;

    /* Heap overflow check */
    if (heap_end + incr > &_heap_end) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return prev_heap_end;
}
