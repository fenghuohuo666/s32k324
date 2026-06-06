# Lin data acquisition and SPI+DMA transmission based on s32k324

## 文档说明


目标：

- 支持1/4/8/16路LIN扩展
- LIN Slave全PID采集
- SPI DMA转发
- RingBuffer缓存
- Ping-Pong DMA发送
- 可扩展架构

---

# 1. 系统架构

```text
LIN Bus
   |
   V
Lpuart_Lin_Ip
   |
   V
Lin_Callback()
   |
   V
LinFrameReceived()
   |
   V
LinRingPush()
   |
   V
ProcessLinPackets()
   |
   V
SpiQueuePush()
   |
   V
SpiDmaKick()
   |
   V
Lpspi_Ip_AsyncTransmit()
```

# 2. 软件分层

```text
Application
 ├─ LIN Manager
 ├─ Packet Manager
 ├─ SPI Manager
 └─ Runtime Monitor

RTD Layer
 ├─ Lpuart_Lin_Ip
 ├─ Lpspi_Ip
 ├─ Dma_Ip
 └─ IntCtrl_Ip
```

# 3. 数据结构设计

## LinPacket_t

```c
typedef struct
{
    uint8_t  channel;
    uint8_t  pid;
    uint8_t  dlc;
    uint8_t  reserve;
    uint32_t timestamp;
    uint8_t  data[8];
    uint16_t crc;
} LinPacket_t;
```

设计目标：统一LIN与SPI数据格式。

# 4. RingBuffer设计

当前容量：

```c
#define LIN_RX_RING_SIZE 128
```

特点：

- 无动态内存
- O(1)访问
- ISR安全
- 未来支持16路独立Ring

## Push流程

```text
head+1
   |
检查满
   |
写入
   |
head更新
```

## Pop流程

```text
检查空
   |
读取
   |
tail更新
```

# 5. SPI发送队列

```c
#define SPI_TX_QUEUE_SIZE 256
```

作用：

解耦LIN接收与SPI发送。

# 6. Ping Pong DMA设计

```text
DMA发送A

CPU组装B

DMA发送B

CPU组装A
```

对应变量：

```c
gSpiDmaBufA[512]
gSpiDmaBufB[512]
```

# 7. CRC设计

当前实现：

```c
CRC16 Modbus
Polynomial 0xA001
```

用于检测SPI链路数据完整性。

# 8. 时间戳设计

```c
GetTimestamp()
```

当前：软件递增。

后续：切换STM定时器。

# 9. LIN接收流程

## 回调入口

```c
Lin_Callback()
```

流程：

1. 获取PID
2. 获取Data
3. 生成Packet
4. Push Ring

# 10. Packet生成

```c
LinFrameReceived()
```

负责：

- Channel
- PID
- DLC
- Timestamp
- CRC

统一封装。

# 11. 主循环

```c
for(;;)
{
    ProcessLinPackets();
    SpiDmaKick();
}
```

特点：

- 无阻塞
- 无动态内存
- DMA驱动

# 12. 多路LIN扩展

当前：

```c
#define LIN_MAX_CHANNELS 16
```

运行时结构：

```c
LinRuntime_t gLinRuntime[16];
```

未来：

- 每路独立回调
- 每路独立统计

# 13. SPI DMA流程

```text
Queue Pop
   |
Copy DMA Buffer
   |
AsyncTransmit
   |
DMA Complete
   |
SpiTxComplete
```

# 14. 已实现内容

- LIN Slave架构
- 全PID采集框架
- RingBuffer
- SPI Queue
- CRC16
- Timestamp
- PingPong DMA
- 主循环调度

# 15. 未实现内容

- 实际16路验证
- 自动波特率扫描
- UDS on LIN
- LDF解析
- Watchdog统计
- Runtime Monitor

# 16. 性能分析



# 17. 建议

1. 使用硬件时间戳
2. 增加Overflow统计
3. 增加DMA错误恢复
4. 增加SPI超时处理
5. 增加诊断接口
6. 增加运行日志

# 18. 最终代码基线

本文档对应架构为：

- main.c
- Lpuart_Lin_Ip
- Lpspi_Ip
- Dma_Ip

