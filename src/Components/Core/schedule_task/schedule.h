/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-24     Alex_min     first version
 */
#ifndef __SCHEDULE_H_
#define __SCHEDULE_H_

#include <stdbool.h>
#include "bitmap.h"
#include "device.h"

#define SCHEDULE_RESOURCE_MAX               64

/**
 * transmission priority
 */
#define SCHEDULE_PRIORITY_MAX               8               /**< highest priority */
#define SCHEDULE_PRIORITY_MIN               0               /**< lowest priority */
#define SCHEDULE_PRIORITY_DEFAULT           5               /**< default priority */

typedef struct {
    void *ringbuf;
    uint32_t number;
    uint8_t priority;
    bool loopback;
} sche_packages;

typedef struct {
    sche_packages *package;
    device_t device;
} traffic;

typedef struct {
    bitmap_t *sched_map;
    //sche_traffic traffic;
    bool bypass;
    traffic *traffic;
} sched_maintenance;

int main_schedule(void);

#endif /* __SCHEDULE_H_ */
