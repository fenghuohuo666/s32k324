/*
 * Copyright (c) 2025,Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-24     Alex_min     first version
 */
#include <stdlib.h>
#include "IfxCpu.h"
#include "Bsp.h"
#include "schedule.h"
#include "list.h"
#include "log.h"
#include "send_task.h"
#include "receive_task.h"
#include "schedule_algo.h"
#include "board.h"
#include "mcmcan_Interface.h"
#include "tc387_def.h"
#include "ota.h"
#include "lwip_udp.h"
#include "ringbuffer.h"

#include "ucb_swap.h"

extern struct list_head device_list;
#if (USE_CPU0_LOAD)
extern volatile uint64 core0_idle_cnt;
#endif

static inline void set_traffic_dev(traffic *traffic, device_t dev)
{
	traffic->device = dev;
}

static int init_schedule_resource(sched_maintenance *sche)
{
	uint32_t bitmap_buf[BITMAP_SIZE(SCHEDULE_RESOURCE_MAX)];
	bitmap_t *map = (bitmap_t *)malloc(sizeof(bitmap_t));
	if (map == NULL)
	{
		LOG_ERROR(LOG_MODULE_SCHEDULE, "alloc schedule memory failed!\r\n");
		return -1;
	}

	bitmap_init(map, bitmap_buf, SCHEDULE_RESOURCE_MAX);
	sche->sched_map = map;

	sche->bypass = false;

	traffic *tra = (traffic *)malloc(sizeof(traffic));
	if (tra == NULL)
	{
		LOG_ERROR(LOG_MODULE_SCHEDULE, "alloc traffic memory failed!\r\n");
		return -1;
	}
	sche->traffic = tra;

	return 0;
}

static int scan_available_resource(sched_maintenance *sche)
{
	device_t dev;
	int free_bit;
	bitmap_t *map = sche->sched_map;

	list_for_each_entry(dev, &device_list, node, struct device)
	{
		if ((dev->flag & DEVICE_FLAG_ACTIVATED) && (dev->frames != 0))
		{
			//Record bypass shcedule
			if (dev->immediate_trans == true)
			{
				sche->bypass = dev->immediate_trans;
				set_traffic_dev(sche->traffic, dev);
				return 0;
			}

			free_bit = bitmap_find_first_zero(map);
			if (free_bit < 0)
			{
				LOG_WARN(LOG_MODULE_SCHEDULE, "schedule task is full.\r\n");
				return 0;
			}
			bitmap_set(map, free_bit);
			set_traffic_dev(sche->traffic, dev);
		}
	}
	return 1;
}

static inline bool schedule(sched_maintenance *sche)
{
	return !sche->bypass;
}


static int extra_data(traffic *traffic)
{
	device_t dev = traffic->device;
	sche_packages *package = (sche_packages *)malloc(sizeof(sche_packages));

	if (package == NULL)
	{
		LOG_ERROR(LOG_MODULE_SCHEDULE, "alloc shcedule package failed!\r\n");
		return -1;
	}

	package->number = dev->frames;
	package->ringbuf = dev->ring_buf;
	package->loopback = dev->loopback;

	traffic->package = package;

	return 0;
}

static inline bool is_codec(device_t dev)
{
	return (dev->type == Device_Class_Remote) ? true : false;
}

volatile struct ringbuffer *rx;
volatile struct ringbuffer *tx;

static void distribute_data(traffic *traffic)
{
	Item item;
	uint32_t loop;
	sche_packages *package = traffic->package;
	device_t dev = traffic->device;
	bool codec = is_codec(dev);
	f_err_t (*codec_ptr)(Item *);
	codec_ptr = dev->encode;
	struct ringbuffer *ringbuf = codec ? get_rx_ring() : get_tx_ring();
	rx = get_rx_ring();
	tx = get_tx_ring();
	assert(ringbuf != NULL);

	for (loop = 0; loop < package->number; loop++)
	{
	    LOG_DEBUG(LOG_MODULE_SCHEDULE, "package->number = %d, device->name = %s\r\n",package->number,traffic->device->name);
		ringbuffer_get(package->ringbuf, (uint8_t *)&item, sizeof(Item));
		codec_ptr(&item);

		ringbuffer_put(ringbuf, (uint8_t *)&item, sizeof(Item));
		lock(dev->spinlock);
		dev->frames--;
		unlock(dev->spinlock);
	}
	free(package);
}

/*改为 1 可周期性执行 SPI DMA 测试 */
#define SPI_TEST_BACKDOOR  1

#if (SPI_TEST_BACKDOOR)
extern void SPI_DMA_Test_Async(void);
#endif

/* 全局可见的循环计数器，用于验证主框架空转 */
volatile uint32_t loop_cnt = 0;

/* 框架测试计数器 */
volatile uint32_t sched_scan_cnt = 0;
volatile uint32_t rb_put_cnt = 0;
volatile uint32_t rb_get_cnt = 0;
volatile uint32_t task_send_cnt = 0;
volatile uint32_t task_recv_cnt = 0;

/* 全局测试数据缓冲区（避免局部变量超出作用域后 Expressions 无法观察） */
volatile uint8_t sched_read_buf[4] = {0};

/* malloc 可用性验证指针 */
volatile void *malloc_test_ptr = NULL;

/* 调度器初始化成功计数器 */
volatile uint32_t sched_init_ok = 0;

/* 调度器扫描设备成功计数器 */
volatile uint32_t sched_scan_ok = 0;

/* 调度器 process_schedule 调用计数器 */
volatile uint32_t sched_process_cnt = 0;

/* extra_data / distribute_data 验证计数器 */
volatile uint32_t sched_extra_ok = 0;
volatile uint32_t sched_dist_ok = 0;

int main_schedule(void)
{
	/* 验证 malloc 可用性 */
	malloc_test_ptr = malloc(64);

	sched_maintenance *sche = (sched_maintenance *)malloc(sizeof(sched_maintenance));
	if (sche == NULL) {
		LOG_ERROR(LOG_MODULE_SCHEDULE, "alloc shcedule memory failed!\r\n");
		return -1;
	}

	int ret = init_schedule_resource(sche);
	if (ret != 0) {
		LOG_ERROR(LOG_MODULE_SCHEDULE, "init shcedule resource failed!\r\n");
		return -1;
	}
	sched_init_ok = 1;

	/* 扫描可用设备资源，验证调度器能发现 test_vdev */
	if (scan_available_resource(sche) == 1) {
		sched_scan_ok = 1;
	}

	/* 验证 send_task / receive_task 框架入口可被调用 */
	send_task();
	receive_task();

	uint8_t partition = 0x0;
    switch (SCU_SWAPCTRL.B.ADDRCFG) {
        case 0x01:
            partition = PARTITION_A;
            break;
        case 0x02:
            partition = PARTITION_B;
            break;
        default:
            break;
    }

	/* ========== 框架测试========== */
	{
		/* 测试 list 框架 */
		struct list_head test_list;
		INIT_LIST_HEAD(&test_list);

		/* 测试ringbuffer */
		struct ringbuffer *test_rb = ringbuffer_create(64);
		if (test_rb) {
			uint8_t test_data[4] = {0x11, 0x22, 0x33, 0x44};
			ringbuffer_put(test_rb, test_data, 4);
			rb_put_cnt++;
			ringbuffer_get(test_rb, (uint8_t *)sched_read_buf, 4);
			rb_get_cnt++;
			ringbuffer_destroy(test_rb);
		}
	}

	while (1) {
	     loop_cnt++;
	     sched_scan_cnt++;
	     task_send_cnt++;
	     task_recv_cnt++;

#if (SPI_TEST_BACKDOOR)
	     if ((loop_cnt % 5000000) == 0) {
	         SPI_DMA_Test_Async();
	     }
#endif

	     if(get_earse_flag())
	     {
	         int result = -1;
	         static ip_addr_t udp_addr;
	         ip4addr_aton("192.168.1.20", &udp_addr);
	         const UdpPcbConfig *udp_cfg = getUdpPcbConfig(Device_Class_OTA);
	         if(udp_cfg == NULL)
	         {
	             LOG_ERROR(LOG_MODULE_LAN7801, "Device_Class_OTA not find pcb\r\n", Device_Class_OTA);
	             return ERROR;
	         }
	         waitTime(100000000);
            //Erase the sector
            if(initProgramFlash(partition))
            {
                LOG_DEBUG(LOG_MODULE_OTA, "initProgramFlash success\r\n");
                result = EOK;
            } else {
                LOG_ERROR(LOG_MODULE_OTA, "initProgramFlash failed\r\n");
                result = ERROR;
            }
            OTA_UPGRADE_ERASE_SUC_t erase_suc = {
                .reserved = 1
            };
            send_ota_response(udp_cfg->pcb, &udp_addr, udp_cfg->port, result, (uint8_t)OTA_ERASE_SUC_RESP, (void *)&erase_suc);
            set_earse_flag(false);
        }
#if (USE_CPU0_LOAD)
        core0_idle_cnt++;
#endif
		scan_available_resource(sche);

		if (schedule(sche)) {
			process_schedule(sche);
			sched_process_cnt++;
		}

		ret = extra_data(sche->traffic);
		if (ret == 0) {
			sched_extra_ok = 1;
		}

		distribute_data(sche->traffic);
		sched_dist_ok = 1;
	}

	return 0;
}
