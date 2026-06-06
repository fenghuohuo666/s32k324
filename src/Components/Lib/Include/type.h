/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-04     Alex_min     define the basic types
 */
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * basic data types definition
 */

#if defined(_WIN64) || defined(__x86_64__)
#ifndef ARCH_64
#define ARCH_64
#endif // ARCH_64
#endif // defined(_WIN64) || defined(__x86_64__)

#ifdef ARCH_64
typedef int64_t                      base_t;      /**< Nbit CPU related data type */
typedef uint64_t                     ubase_t;     /**< Nbit unsigned CPU related data type */
#else
typedef int32_t                      base_t;      /**< Nbit CPU related data type */
typedef uint32_t                     ubase_t;     /**< Nbit unsigned CPU related data type */
#endif

typedef base_t                    f_err_t;       /**< Type for error number */
typedef base_t                    off_t;       /**< Type for offset */

struct list_head {
	struct list_head *next, *prev;
};

#ifdef __cplusplus1
}
#endif

#endif /* __TYPES_H__ */
