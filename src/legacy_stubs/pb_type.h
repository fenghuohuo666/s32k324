#ifndef PB_TYPE_H
#define PB_TYPE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef struct { uint8_t *buf; size_t bufsize; size_t bytes_written; } pb_ostream_t;
typedef struct { const uint8_t *buf; size_t bufsize; size_t bytes_left; } pb_istream_t;
typedef bool (*pb_callback_t)(pb_ostream_t *stream, const void *field);
#define PB_WT_STATIC    0
#define PB_WT_CALLBACK  1
#define PB_WT_POINTER   2
#define PB_WT_ONEOF     3
#define PB_WT_OPTIONAL  4
#define PB_WT_REPEATED  5
#define PB_WT_REQUIRED  6
#define PB_WT_FIXARRAY  7
#define PB_WT_MANY      8
#define PB_WT_LARGEST   9
#define PB_SIZE_STATIC  0
#define PB_SIZE_VARIABLE 1
#endif
