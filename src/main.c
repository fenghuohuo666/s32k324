/*==================================================================================================
* Project              : RTD AUTOSAR 4.7
* Platform             : CORTEXM
* Peripheral           : S32K3XX
* Dependencies         : none
*
* Autosar Version      : 4.7.0
* Autosar Revision     : ASR_REL_4_7_REV_0000
* Autosar Conf.Variant :
* SW Version           : 3.0.0
* Build Version        : S32K3_RTD_3_0_0_D2303_ASR_REL_4_7_REV_0000_20230331
*
* Copyright 2020 - 2023 NXP Semiconductors
==================================================================================================*/

/**
* @file main.c
* @brief S32K324 SPI Test + TC387 Framework Smoke Test
*        Phase 0: Pure software framework validation (no hardware dependency)
*        Phase 1: Device framework registration/lookup/match
*        Phase 2: Scheduler core logic (scan/schedule/extra/distribute)
*        Phase 3: Hardware driver replacement (future)
*/

/* NXP RTD 驱动，用于 S32K324 硬件初始化 */
#include "Clock_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Lpspi_Ip.h"
#include "IntCtrl_Ip.h"
#include "CDD_Rm.h"
#include "Dma_Ip.h"
#include "Lpuart_Lin_Ip.h"

#include "stdio.h"

/* SJA1110 SPI 实例/配置/外部设备宏定义 */
#include "SJA1110_SPI.h"

/* S32K324 LPUART9 基地址（官方头文件 S32K324_LPUART.h 定义） */
#define LPUART9_BASE  (0x40490000U)
#define LPUART9       ((LPUART_Type *)LPUART9_BASE)

/* TC387 框架头文件 */
#include "list.h"
#include "ringbuffer.h"
#include "device.h"
#include <string.h>
#include "send_task.h"
#include "receive_task.h"
#include "md5.h"
#include "bitmap.h"

/* 应用入口的前向声明 */
extern void core0_main(void);

/* 全局测试数据缓冲区 */
volatile uint8_t main_read_buf[4] = {0};

/* MD5 验证结果（"hello" 的 MD5 = 5d41402abc4b2a76b9719d911017c592） */
volatile uint8_t md5_output[16] = {0};
volatile uint32_t md5_ok = 0;

/* 验证计数器 */
volatile uint32_t bitmap_full_ok = 0;
volatile uint32_t rb_boundary_ok = 0;
volatile uint32_t dev_unregister_ok = 0;

/* 环形缓冲区边界调试变量 */
volatile uint32_t rb_put_len_dbg = 0;
volatile uint32_t rb_get_len_dbg = 0;
volatile uint8_t  rb_readbuf_0_dbg = 0;
volatile uint8_t  rb_readbuf_7_dbg = 0;

/* ========== 设备框架验证计数器 ========== */
volatile uint32_t dev_reg_cnt = 0;      /* device_register 成功计数 */
volatile uint32_t dev_find_cnt = 0;     /* device_find 成功计数 */
volatile uint32_t dev_match_cnt = 0;    /* match_device 成功计数 */

/* 虚拟设备实例 */
static struct device test_vdev;
static spinLock test_vdev_lock;
static struct ringbuffer test_vdev_rb_obj;
static uint8_t test_vdev_pool[64];

static f_err_t test_vdev_encode(Item *item)
{
    (void)item;
    return EOK;
}

/* 设备生命周期验证计数器（位掩码：bit0=init, bit1=open, bit2=close, bit3=read, bit4=write, bit5=control） */
volatile uint32_t dev_lifecycle_mask = 0;

static f_err_t test_vdev_init_stub(device_t dev)
{
    dev_lifecycle_mask |= 0x01;
    (void)dev;
    return EOK;
}
static f_err_t test_vdev_open_stub(device_t dev, uint16_t oflag)
{
    dev_lifecycle_mask |= 0x02;
    (void)dev;
    (void)oflag;
    return EOK;
}
static f_err_t test_vdev_close_stub(device_t dev)
{
    dev_lifecycle_mask |= 0x04;
    (void)dev;
    return EOK;
}
static f_err_t test_vdev_read_stub(device_t dev, off_t pos, void *buffer, size_t size)
{
    dev_lifecycle_mask |= 0x08;
    (void)dev;
    (void)pos;
    (void)buffer;
    (void)size;
    return EOK;
}
static f_err_t test_vdev_write_stub(device_t dev, off_t pos, const void *buffer, size_t size)
{
    dev_lifecycle_mask |= 0x10;
    (void)dev;
    (void)pos;
    (void)buffer;
    (void)size;
    return EOK;
}
static f_err_t test_vdev_control_stub(device_t dev, int cmd, void *args)
{
    dev_lifecycle_mask |= 0x20;
    (void)dev;
    (void)cmd;
    (void)args;
    return EOK;
}

static struct device_ops test_vdev_ops = {
    .init = test_vdev_init_stub,
    .open = test_vdev_open_stub,
    .close = test_vdev_close_stub,
    .read = test_vdev_read_stub,
    .write = test_vdev_write_stub,
    .control = test_vdev_control_stub,
};

/*==================================================================================================
* 切换宏选择 SPI 测试模式：
* - 0: 轮询（同步）模式
* - 1: DMA（异步中断）模式
==================================================================================================*/
#define USE_DMA_TEST    1


/*==================================================================================================
* 局部宏
==================================================================================================*/
#define FXOSC_CLOCK_FREQ   40000000U

/* LIN Slave 配置 */
#define LIN_SLAVE_INSTANCE       9U
#define LIN_SLAVE_RX_BUF_SIZE    8U
#define LIN_SLAVE_DEFAULT_BAUD   19200U
#define LIN_SLAVE_TIMEOUT        100000U

/* LIN PDU 定义（参考 NXP Lin_Ip_FrameTransfer 示例） */
#define LIN_SEND_PID              (0x1A)
#define LIN_RECV_PID              (0x2B)
#define LIN_SEND                  0
#define LIN_RECV                  1

void DevAssert_local(volatile boolean x)
{
    if(x) { } else { for(;;) {} }
}
#define BSP_DEV(x) DevAssert_local(x)

/* 驱动初始化状态返回值 */
Siul2_Port_Ip_PortStatusType port_init_status = SIUL2_PORT_SUCCESS;
IntCtrl_Ip_StatusType IntCtrl_Status = INTCTRL_IP_STATUS_SUCCESS;
Dma_Ip_ReturnType dma_init_status = DMA_IP_STATUS_SUCCESS;
Lpspi_Ip_StatusType spi_status_ret = LPSPI_IP_STATUS_SUCCESS;

/* SPI 测试缓冲区放置于不可缓存区域以保证 DMA 一致性，
   32 字节对齐以防止缓存不一致问题 */
#pragma GCC section bss ".mcal_bss_no_cacheable"
__attribute__(( aligned(32) )) uint8_t spi_txBuffer[8];
__attribute__(( aligned(32) )) uint8_t spi_rxBuffer[8];
#pragma GCC section bss

/* DMA 异步传输完成标志 */
volatile uint8_t g_spi_async_tx_complete = 0;

/* LIN Slave 运行时配置（static 防止野指针） */
static Lpuart_Lin_Ip_UserConfigType g_lin_slave_cfg;

/* LIN Slave 收发缓冲 */
uint8_t LpuartTxBuff[LIN_SLAVE_RX_BUF_SIZE] = {0x01, 0x02, 0x03, 0x1f, 0x32, 0xac, 0x76, 0xee};
uint8_t *RxBuff = NULL;
volatile uint32_t LinSendCounter = 0;
volatile uint32_t LinRecvCounter = 0;

/* LIN PDU 数组：[0]=发送, [1]=接收 */
Lpuart_Lin_Ip_PduType LinLpuartPdu[] =
{
    {
        .Pid = (uint8)LIN_SEND_PID,
        .Cs  = LPUART_LIN_IP_ENHANCED_CS,
        .SduPtr = LpuartTxBuff,
        .Drc = LPUART_LIN_IP_FRAMERESPONSE_TX,
        .Dl  = (uint8)LIN_SLAVE_RX_BUF_SIZE
    },
    {
        .Pid = (uint8)LIN_RECV_PID,
        .Cs  = LPUART_LIN_IP_ENHANCED_CS,
        .SduPtr = NULL,
        .Drc = LPUART_LIN_IP_FRAMERESPONSE_RX,
        .Dl  = (uint8)LIN_SLAVE_RX_BUF_SIZE
    }
};

/**
 * @brief       PIT 定时器中断回调函数
 * @details
 * 当前无任务，函数体内留空。
 */
void PitNotification(void)
{
    /* 留空 */
}

void Siul2Callback(void)
{
    /* ICU callback stub */
}

void AutobaudCallback(const uint8 Instance, uint32 *NanoSeconds)
{
    (void)Instance;
    (void)NanoSeconds;
}

void FlexioSlaveCallback(uint8 Instance, void *FlexioStateStruct)
{
    (void)Instance;
    (void)FlexioStateStruct;
}

void LpuartSlaveCallback(uint8 Instance, const Lpuart_Lin_Ip_StateStructType *LpuartStateStruct)
{
    switch (LpuartStateStruct->CurrentEventId)
    {
        /* Header 接收成功，根据 PID 决定发送或接收 */
        case LPUART_LIN_IP_RECV_HEADER_OK:
            if (LpuartStateStruct->CurrentPid == LinLpuartPdu[LIN_SEND].Pid)
            {
                /* Slave 发送数据 */
                (void)Lpuart_Lin_Ip_SendFrame(Instance, (const Lpuart_Lin_Ip_PduType *)&LinLpuartPdu[LIN_SEND]);
            }
            if (LpuartStateStruct->CurrentPid == LinLpuartPdu[LIN_RECV].Pid)
            {
                /* Slave 接收数据 */
                (void)Lpuart_Lin_Ip_SendFrame(Instance, (const Lpuart_Lin_Ip_PduType *)&LinLpuartPdu[LIN_RECV]);
            }
            break;

        /* 发送完成 */
        case LPUART_LIN_IP_TX_COMPLETED:
            LinSendCounter++;
            break;

        /* 接收完成 */
        case LPUART_LIN_IP_RX_COMPLETED:
            LinRecvCounter++;
            break;

        default:
            /* do nothing */
            break;
    }
}

void delay_dummy(uint32_t count)
{
    volatile uint32_t i;
    for(i = 0; i < count; i++) {
        __asm volatile ("nop");
    }
}

/* SPI 异步回调前向声明 */
void SPI_Test_Callback(uint8 u8Instance, Lpspi_Ip_EventType event);
void SPI_DMA_Test_Async(void);

/**
 * @brief       LIN Slave 收发测试（参考 NXP Lin_Ip_FrameTransfer 示例）
 * @details     LIN Master 发送/接收 → LPUART9 Slave 响应
 */
void LIN_Slave_Test(void)
{
    volatile Lpuart_Lin_Ip_TransferStatusType LpuartSlaveStatus = LPUART_LIN_IP_STATUS_OPERATIONAL;

    /* 初始化 LIN Slave（使用 PBcfg 中 Slave 实例配置） */
    Lpuart_Lin_Ip_Init(Lpuart_Lin_Ip_Sa_pHwConfigPB_0.Instance, &Lpuart_Lin_Ip_Sa_pHwConfigPB_0);

    while (1)
    {
        /* 等待接收完成 */
        LpuartSlaveStatus = Lpuart_Lin_Ip_GetStatus(Lpuart_Lin_Ip_Sa_pHwConfigPB_0.Instance, (const uint8 **)&RxBuff);

        if (LPUART_LIN_IP_STATUS_RX_OK == LpuartSlaveStatus)
        {
            /* LIN 数据收到 → 拷贝到 SPI txBuffer → 发送 */
            memcpy((void *)spi_txBuffer, RxBuff, LIN_SLAVE_RX_BUF_SIZE);
            SPI_DMA_Test_Async();
        }

        if (LPUART_LIN_IP_STATUS_TX_OK == LpuartSlaveStatus)
        {
            /* LIN 发送完成 */
        }
    }
}

/**
 * @brief SPI 异步传输事件回调
 * @details 仅在 DMA 模式下有效。DMA TCD 完成后由硬件中断调用。
 */
void SPI_Test_Callback(uint8 u8Instance, Lpspi_Ip_EventType event)
{
    if(SJA1110_SPI_INSTANCE == u8Instance)
    {
        if(event == LPSPI_IP_EVENT_END_TRANSFER)
        {
            g_spi_async_tx_complete = 1; /* 标记传输完成 */
        }
    }
}

/**
 * @brief 测试 1: SPI 轮询测试（无 DMA）
 * @details 使用 Lpspi_Ip_SyncTransmit 进行同步阻塞传输。
 */
void SPI_Polling_Test_NoDMA(void)
{
    Lpspi_Ip_StatusType sync_status;

    spi_txBuffer[0] = 0x55; spi_txBuffer[1] = 0xAA;
    spi_txBuffer[2] = 0x55; spi_txBuffer[3] = 0xAA;
    spi_txBuffer[4] = 0x11; spi_txBuffer[5] = 0x22;
    spi_txBuffer[6] = 0x33; spi_txBuffer[7] = 0x44;

    for(uint8_t i = 0; i < 8; i++) spi_rxBuffer[i] = 0x00;

    /* 切换传输模式为 POLLING */
    Lpspi_Ip_UpdateTransferMode(SJA1110_SPI_INSTANCE, LPSPI_IP_POLLING);

    /* 调用同步传输（CS 自动拉低，传输结束后拉高） */
    sync_status = Lpspi_Ip_SyncTransmit(SJA1110_SPI_ExtDevice, spi_txBuffer, spi_rxBuffer, 8, 50000);

    if (sync_status == LPSPI_IP_STATUS_SUCCESS)
    {
        printf("[NoDMA] Polling TX/RX done. RX first 4 bytes: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
               spi_rxBuffer[0], spi_rxBuffer[1], spi_rxBuffer[2], spi_rxBuffer[3]);
    }
    else
    {
        printf("[NoDMA] Polling TX/RX failed, error code: 0x%X\r\n", sync_status);
    }
}

/**
 * @brief       测试 2: 使用 DMA 的 SPI 异步测试（DMA + 中断）
 * @details     利用 Lpspi_Ip_AsyncTransmit 启动后由 DMA 自动搬运，死等硬件状态及回调标志。
 */
void SPI_DMA_Test_Async(void)
{
    Lpspi_Ip_StatusType async_status;
    Lpspi_Ip_HwStatusType hw_status;
    uint32_t timeout_cnt = 0xFFFF;

    /* 使用外部已准备的 spi_txBuffer 数据，不再固定填充 */
    for(uint8_t i = 0; i < 8; i++) spi_rxBuffer[i] = 0x00;

    g_spi_async_tx_complete = 0; // 清除软件完成标志

    /* 将传输模式更新为 中断/DMA 模式 (INTERRUPT) */
    Lpspi_Ip_UpdateTransferMode(SJA1110_SPI_INSTANCE, LPSPI_IP_INTERRUPT);

    /* 调用异步发送接口 */
    async_status = Lpspi_Ip_AsyncTransmit(SJA1110_SPI_ExtDevice, spi_txBuffer, spi_rxBuffer, 8, SPI_Test_Callback);

    if (async_status != LPSPI_IP_STATUS_SUCCESS)
    {
        printf("[DMA] Async transfer start failed, error code: 0x%X\r\n", async_status);
        return;
    }

    /* 阻塞等待 DMA 传输结束 */
    do
    {
        hw_status = Lpspi_Ip_GetStatus(SJA1110_SPI_INSTANCE);
        timeout_cnt--;
    } while ((LPSPI_IP_BUSY == hw_status || g_spi_async_tx_complete == 0) && (timeout_cnt > 0));

    if (timeout_cnt == 0)
    {
        printf("[DMA] 传输超时！请确认工具中 DMA 中断是否绑定开启。\r\n");
    }
    else
    {
        printf("[DMA] 异步传输完成. RX 前4字节: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
               spi_rxBuffer[0], spi_rxBuffer[1], spi_rxBuffer[2], spi_rxBuffer[3]);
    }
}

/**
 * @brief       初始化基础核心硬件环境
 */
void BSP_Init(void)
{
    Clock_Ip_StatusType clockStatus;

    /* 初始化核心系统时钟 */
    clockStatus = Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);
    BSP_DEV(CLOCK_IP_SUCCESS == clockStatus);

    /* 初始化引脚复用配置 */
    port_init_status = Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    BSP_DEV(SIUL2_PORT_SUCCESS == port_init_status);

    /* 初始化 DMA 核心模块 */
    dma_init_status = Dma_Ip_Init(&Dma_Ip_xDmaInitPB);
    BSP_DEV(DMA_IP_STATUS_SUCCESS == dma_init_status);

    /* 初始化资源管理器（RM）以建立并激活 DMAMUX 映射 */
    Rm_Init(&Rm_Config);

    /* 初始化核心中断控制器并使能 NVIC 中的 DMA 中断 */
    IntCtrl_Status = IntCtrl_Ip_Init(&IntCtrlConfig_0);
    BSP_DEV(INTCTRL_IP_STATUS_SUCCESS == IntCtrl_Status);

    // 使能用于 SPI 异步接收和发送的两个 DMA 通道中断（对应 TCD0 和 TCD1 硬件实体）
    IntCtrl_Ip_EnableIrq(DMATCD0_IRQn);
    IntCtrl_Ip_EnableIrq(DMATCD1_IRQn);

    /* 初始化 SPI 外部设备底层硬件寄存器 */
    spi_status_ret = Lpspi_Ip_Init(&SJA1110_SPI_Config);
    BSP_DEV(LPSPI_IP_STATUS_SUCCESS == spi_status_ret);
}

/**
 * @brief       主函数入口
 */
int main(void)
{
    /* 初始化 SPI 和 DMA 硬件环境 */
    BSP_Init();

    printf("\r\n==================================================\r\n");
    printf(" Minimal SPI (LPSPI) standalone wave test program started\r\n");
    printf("==================================================\r\n");

    // 上电防干扰缓冲延时
    delay_dummy(4000000);

    // /* ========== SPI 验证========== */
    // for (int spi_round = 0; spi_round < 1; spi_round++)
    // {
    //     #if (USE_DMA_TEST == 0)
    //         SPI_Polling_Test_NoDMA();
    //     #else
    //         SPI_DMA_Test_Async();
    //     #endif
    //     delay_dummy(12000000);
    // }

    // printf("\r\n========== SPI 硬件验证完成 ==========\r\n");

    // /* ========== 框架测试========== */
    // printf("========== 框架冒烟测试开始 ==========\r\n");

    // /* 测试 list 链表框架 */
    // struct list_head test_list;
    // INIT_LIST_HEAD(&test_list);
    // printf("[PASS] list 框架: INIT_LIST_HEAD 执行成功\r\n");

    // /* 测试 ringbuffer 环形缓冲框架 */
    // struct ringbuffer *test_rb = ringbuffer_create(64);
    // if (test_rb != NULL)
    // {
    //     uint8_t test_data[4] = {0x11, 0x22, 0x33, 0x44};
    //     size_t put_len = ringbuffer_put(test_rb, test_data, 4);
    //     size_t get_len = ringbuffer_get(test_rb, (uint8_t *)main_read_buf, 4);
    //     ringbuffer_destroy(test_rb);

    //     if (put_len == 4 && get_len == 4 && main_read_buf[0] == 0x11)
    //     {
    //         printf("[PASS] ringbuffer 框架: init/put/get 成功\r\n");
    //     }
    //     else
    //     {
    //         printf("[FAIL] ringbuffer 框架: 数据不一致\r\n");
    //     }
    // }

    // /* 测试 device 设备框架 */
    // memset(&test_vdev, 0, sizeof(test_vdev));
    // test_vdev.type = Device_Class_Char;
    // test_vdev.device_id = 0;
    // test_vdev.frames = 1;   /* 让 scan_available_resource() 能扫描到该设备 */
    // if (device_register(&test_vdev, "test_vdev", DEVICE_FLAG_ACTIVATED) == EOK) {
    //     dev_reg_cnt = 1;
    // }
    // if (device_find("test_vdev") != NULL) {
    //     dev_find_cnt = 1;
    // }
    // if (match_device(Device_Class_Char, 0) != NULL) {
    //     dev_match_cnt = 1;
    // }

    // /* 给虚拟设备补齐 distribute_data 所需的字段 */
    // ringbuffer_init(&test_vdev_rb_obj, test_vdev_pool, 64);
    // test_vdev.ring_buf = &test_vdev_rb_obj;
    // test_vdev.encode = test_vdev_encode;
    // test_vdev.spinlock = &test_vdev_lock;

    // /* 初始化全局 rx/tx ringbuffer（distribute_data 需要） */
    // creat_rx_ring();
    // creat_tx_ring();

    // /* 测试 schedule 调度器入口（进 while(1) 前打印确认） */
    // printf("[PASS] 框架冒烟测试全部通过，准备进入 main_schedule()\r\n");
    // printf("========== 框架冒烟测试结束 ==========\r\n");

    // /* ==========  TC387 业务框架 ========== */
    // /* 测试 md5 */
    // {
    //     unsigned char temp_md5[16];
    //     const char *test_str = "hello";
    //     int md5_ret = mbedtls_md5((const unsigned char *)test_str, strlen(test_str), temp_md5);
    //     if (md5_ret == 0) {
    //         for (int i = 0; i < 16; i++) {
    //             md5_output[i] = temp_md5[i];
    //         }
    //         md5_ok = 1;
    //     }
    // }

    // /* 测试 device 生命周期 (init/open/read/write/control/close) */
    // test_vdev.ops = &test_vdev_ops;
    // test_vdev.flag &= ~DEVICE_FLAG_ACTIVATED;
    // device_init(&test_vdev);
    // device_open(&test_vdev, DEVICE_OFLAG_RDWR);
    // device_read(&test_vdev, 0, NULL, 0);
    // device_write(&test_vdev, 0, NULL, 0);
    // device_control(&test_vdev, 0, NULL);
    // device_close(&test_vdev);

    // /* bitmap 全 API 测试 */
    // {
    //     uint32_t bmp_buf[BITMAP_SIZE(64)];
    //     bitmap_t bmp;
    //     bitmap_init(&bmp, bmp_buf, 64);
    //     bitmap_set(&bmp, 5);
    //     int t1 = bitmap_test(&bmp, 5);
    //     bitmap_clear(&bmp, 5);
    //     int t2 = bitmap_test(&bmp, 5);
    //     bitmap_set(&bmp, 3);
    //     int first = bitmap_find_first_bit(&bmp);
    //     if (t1 == 1 && t2 == 0 && first == 3) {
    //         extern volatile uint32_t bitmap_full_ok;
    //         bitmap_full_ok = 1;
    //     }
    // }

    // /* ringbuffer 边界测试（ALIGN_SIZE = 8，buffer 是 8 字节对齐） */
    // {
    //     struct ringbuffer rb;
    //     uint8_t pool[8];
    //     uint8_t data[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    //     uint8_t readbuf[16] = {0};
    //     ringbuffer_init(&rb, pool, 8);
    //     size_t put_len = ringbuffer_put(&rb, data, 16);
    //     size_t get_len = ringbuffer_get(&rb, readbuf, 16);
    //     rb_put_len_dbg = put_len;
    //     rb_get_len_dbg = get_len;
    //     rb_readbuf_0_dbg = readbuf[0];
    //     rb_readbuf_7_dbg = readbuf[7];
    //     if (put_len == 8 && get_len == 8 && readbuf[0] == 1 && readbuf[7] == 8) {
    //         rb_boundary_ok = 1;
    //     }
    // }

    // /* device_unregister 测试 */
    // {
    //     static struct device test_vdev2;
    //     memset(&test_vdev2, 0, sizeof(test_vdev2));
    //     device_register(&test_vdev2, "test2", 0);
    //     device_unregister(&test_vdev2);
    //     extern volatile uint32_t dev_unregister_ok;
    //     dev_unregister_ok = 1;
    // }

    /* ========== LIN Slave 接收 → SPI 转发测试 ========== */
    LIN_Slave_Test();

    // core0_main();

    return 0;
}
