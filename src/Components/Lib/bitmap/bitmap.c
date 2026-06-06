/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-28     Alex_min	   first version
 */

#include "bitmap.h"

/**
 * @brief Initialize the bitmap.
 *
 * @param:bmp           A pointer to the bitmap struct.
 * @param:buffer        A pointer to the bitmap buffer.
 * @param:total_bits    The number of the bits in bitmap.
 */
void bitmap_init(bitmap_t *bmp, uint32_t *buffer, int total_bits)
{
    int i;
    int words = BITMAP_SIZE(total_bits);

    bmp->map = buffer;
    bmp->size = total_bits;

    for (i = 0; i < words; i++)
        bmp->map[i] = 0;
}

/**
 * @brief Set the bitmap.
 *
 * @param:bmp           A pointer to the bitmap struct.
 * @param:bit           Needs to set the bits in the bitmap.
 */
void bitmap_set(bitmap_t *bmp, int bit)
{
    if (bit >= bmp->size)
        return;
    
    bmp->map[bit / BITS_PER_WORD] |= (1U << (bit % BITS_PER_WORD));
}


/**
 * @brief Clear the bitmap.
 *
 * @param:bmp           A pointer to the bitmap struct.
 * @param:bit           Needs to clear the bits in the bitmap.
 */
void bitmap_clear(bitmap_t *bmp, int bit)
{
    if (bit >= bmp->size)
        return;
    
    bmp->map[bit / BITS_PER_WORD] &= ~(1U << (bit % BITS_PER_WORD));
}

/**
 * @brief Search the bitmap.
 *
 * @param:bmp           A pointer to the bitmap struct.
 * @param:bit           Search the status of this bit in the bitmap.
 * 
 *  @return             Return the status of this bit in the bitmap.
 */
int bitmap_test(bitmap_t *bmp, int bit)
{
    if (bit >= bmp->size)
        return 0;

    return (bmp->map[bit / BITS_PER_WORD] >> (bit % BITS_PER_WORD)) & 1U;
}

/**
 * @brief Search the first zero bit in the bitmap.
 *
 * @param:bmp           A pointer to the bitmap struct.
 * 
 *  @return             Return the first zero bit in the bitmap.
 */
int bitmap_find_first_zero(bitmap_t *bmp)
{
    int i, j;
    int bit;
    int words = BITMAP_SIZE(bmp->size);

    for (i = 0; i < words; ++i)
    {
        if (bmp->map[i] != 0xFFFFFFFF)
        {
            for (j = 0; j < BITS_PER_WORD; ++j)
            {
                bit = i * BITS_PER_WORD + j;
                if (bit >= bmp->size)
                    return -1;
                if (!(bmp->map[i] & (1U << j)))
                    return bit;
            }
        }
    }

    return -1;
}

/**
 * @brief Search the first non-zero bit in the bitmap.
 *
 * @param:bmp           A pointer to the bitmap struct.
 * 
 *  @return             Return the first non-zero bit in the bitmap.
 */
int bitmap_find_first_bit(bitmap_t *bmp)
{
    int i, j;
    int bit;
    int words = BITMAP_SIZE(bmp->size);

    for (i = 0; i < words; ++i)
    {
        if (bmp->map[i] != 0)
        {
            for (j = 0; j < BITS_PER_WORD; ++j)
            {
                bit = i * BITS_PER_WORD + j;
                if (bit >= bmp->size)
                    return -1;
                if ((bmp->map[i] & (1U << j)))
                    return bit;
            }
        }
    }

    return -1;
}