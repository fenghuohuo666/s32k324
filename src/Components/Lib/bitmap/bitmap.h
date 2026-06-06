/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-28     Alex_min	   first version
 */

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <stdint.h>

#define BITS_PER_WORD 32
#define BITMAP_SIZE(bits) (((bits) + BITS_PER_WORD - 1) / BITS_PER_WORD)

typedef struct {
    uint32_t *map;
    int size;
} bitmap_t;

void bitmap_init(bitmap_t *bmp, uint32_t *buffer, int total_bits);
void bitmap_set(bitmap_t *bmp, int bit);
void bitmap_clear(bitmap_t *bmp, int bit);
int bitmap_test(bitmap_t *bmp, int bit);
int bitmap_find_first_zero(bitmap_t *bmp);
int bitmap_find_first_bit(bitmap_t *bmp);

#endif /* __BITMAP_H__ */
