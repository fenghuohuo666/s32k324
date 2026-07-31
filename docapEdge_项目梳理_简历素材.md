# docapEdge 项目梳理与简历素材

> 项目：车规级数据采集终端（T-BOX）嵌入式软件
> 硬件平台：NXP S32G（Linux 主控）+ 英飞凌 TC387（MCU 子板）+ 移远 AG55（5G 模组）
> 本文档整理两大核心模块：**OTA 升级系统** 与 **TC387 子板通信/CAN/LIN 数据采集**，供简历撰写与面试准备使用。

---

## 目录

- [一、项目整体概述](#一项目整体概述)
- [二、OTA 升级系统](#二ota-升级系统)
  - [2.1 总体设计](#21-总体设计)
  - [2.2 云端 OTA 主流程（状态机驱动）](#22-云端-ota-主流程状态机驱动)
  - [2.3 A/B 双分区与自解压升级包](#23-ab-双分区与自解压升级包)
  - [2.4 分片下载与断点续传](#24-分片下载与断点续传)
  - [2.5 本地 Web 升级](#25-本地-web-升级)
  - [2.6 5G 模组 AG55 升级链路](#26-5g-模组-ag55-升级链路)
  - [2.7 TC387 子板 HEX 升级](#27-tc387-子板-hex-升级)
- [三、PREMIUM 设备与 TC387 子板通信](#三premium-设备与-tc387-子板通信)
  - [3.1 硬件架构与板型抽象](#31-硬件架构与板型抽象)
  - [3.2 通道映射表](#32-通道映射表)
  - [3.3 通信协议栈：数据面与控制面分离](#33-通信协议栈数据面与控制面分离)
  - [3.4 主控侧接收链路与性能优化](#34-主控侧接收链路与性能优化)
  - [3.5 本地/远程透明切换（接口抽象 + 工厂）](#35-本地远程透明切换接口抽象--工厂)
  - [3.6 双时钟域时间同步](#36-双时钟域时间同步)
- [四、简历内容](#四简历内容)
  - [4.1 项目描述（详细版）](#41-项目描述详细版)
  - [4.2 项目描述（精简版）](#42-项目描述精简版)
  - [4.3 技术栈关键词](#43-技术栈关键词)
  - [4.4 可量化指标](#44-可量化指标)
- [五、面试问答准备](#五面试问答准备)

---

## 一、项目整体概述

车规级数据采集终端（T-BOX），运行于 NXP S32G 平台（Linux），面向整车 CAN/CANFD/LIN/FlexRay/以太网数据采集、存储、上云场景。产品有多种板型（STANDARD / BASIC / PREMIUM 等），同一代码库通过板型识别（EEPROM/SN）适配不同硬件：

- **STANDARD 版**：CAN/LIN 直接挂在 S32G 本地（SocketCAN / LLCE 驱动）
- **PREMIUM 版**：S32G + TC387 双处理器架构，TC387 通过车载以太网扩展 8 路 CAN + 16 路 LIN + FlexRay
- **5G 模组**：移远 AG55，通过以太网与主控互联，负责蜂窝上行

系统采用**多进程模块化架构**：30+ 个独立进程（rawCAN、rawLIN、DFTsever 存储、ComAgent 上云、OTA、5GAgent 等），通过 Unix Domain Socket 的 IPC 消息总线（统一消息头 `OSAL_MSG_HEAD` + 模块 ID + 消息 ID）互联，元数据/配置由 MetaDataManager 集中管理。

---

## 二、OTA 升级系统

### 2.1 总体设计

OTA 系统的核心设计：**状态机驱动 + IPC 消息总线 + A/B 双分区 + 统一自解压升级包**。

- OTA Service 是独立进程（模块 ID `OSAL_MOD_ID_OTA = 10`），不直接连云端，通过 IPC 总线与 ComAgent（MQTT 客户端）通信，OTA 只关心状态机，网络细节全部交给 ComAgent
- 升级包是**自解压 Shell 脚本**：脚本 + `END_OF_SCRIPT_START_OF_INFO` 分隔符 + MD5/版本信息 + `END_OF_INFO_START_OF_PACKAGE` 分隔符 + tar 包，`sh` 执行即自动完成校验、解包、A/B 切换、重启
- 四条升级路径复用同一套包格式和分区机制：
  1. 云端 OTA（完整状态机流程）
  2. 本地 Web 升级（CGI 手动上传）
  3. 5G 模组 AG55 升级（跨处理器 TCP 流式刷写）
  4. TC387 子板升级（Intel HEX 块传输）

关键文件：

| 路径 | 说明 |
|---|---|
| `OTA/src/statemachine/OTAfsm.cpp` | 状态机实现（tinyfsm） |
| `OTA/src/actioner/` | 各状态动作：查询/下载/校验/确认/执行 |
| `OTA/UpdateTools/docapEdgeHeader.sh` | 自解压安装脚本 |
| `OTA/UpdateTools/scripts/create_image_link.sh` | A/B 分区符号链接切换 |
| `/usr/grce/OTA/upgrade/OTAImage` | 下载的升级包 |
| `/usr/grce/OTA/.status` | 升级状态文件（重启后回填） |

### 2.2 云端 OTA 主流程（状态机驱动）

状态机（基于 tinyfsm）状态迁移：

```
INIT → IDLE → DONWLOADING → VERIFY → CONFIRM → UPDATING → SUCCESS/FAILED/ERROR
```

**1. INIT —— 升级结果回填**

升级过程会重启系统，状态机内存态丢失，因此用 `/usr/grce/OTA/.status` 文件持久化升级进度。服务启动时读取：

- 读到 1（UPDATING）：上次升级中断 → 判定失败
- 读到 2（SUCCESS_A）：上次升级成功 → 上报成功

随后通过 `RpcUtils::registerTopic` 向 ComAgent 注册 telemetry topic（`v1/devices/me/telemetry`），注册成功 dispatch `InitSucess` 进入 IDLE。

**2. IDLE —— 查询新版本**

注册 MQTT 属性请求/响应 topic（ThingsBoard 风格）：

- 请求：`v1/devices/me/attributes/request/+`
- 响应：`v1/devices/me/attributes/response/+`
- 请求体：`{"sharedKeys":"sw_checksum,sw_update_now,sw_checksum_algorithm,sw_size,sw_title,sw_version"}`

请求带递增 session id（MQTT 异步响应对应回请求），10s 超时重查，重试 5 次失败进 ERROR。

云端响应解析（`ImageQuery::getVersionInfo`）：

- `sw_checksum`：SHA256
- `sw_size` / `sw_title` / `sw_version`
- `sw_update_now`：升级模式 —— `NOW`（用户确认）/ `SILENT`（静默升级）/ `FORCE`（强制，跳过版本检查）
- `sw_title` 含 `_5G_` 字样 → 识别为 5G 固件，版本比较和升级路径分流

版本号归一化比较：正则 `(\d{1,2}\.\d{1,2}\.\d{1,2})(-\d{1,3})?` 提取版本号，补丁号不足 3 位前补 0，Edge 固件比 `DeviceInfo.SoftwareVersion`，5G 固件比 `DeviceInfo.CommProjectRev`。

**3. DONWLOADING —— 分片下载**

- 分片大小 512KB（`OTA_CHUNK_FRAG_SIZE`）
- 下载 topic：`v2/sw/request/{sessionId}/chunk/+`（sessionId 继承自查询阶段）
- 每片 10s 超时，重试 3 次
- 支持断点续传（见 2.4）

**4. VERIFY —— SHA256 校验**

调系统 `sha256sum` 对整包计算哈希，与云端 `sw_checksum` 比对；失败删除升级包进 ERROR。

**5. CONFIRM —— 用户确认**

NOW 模式下校验通过后等待用户确认：状态写入 MetaData（`OTA/OTAState=CONFIRMING` + 新版本号），前端网页轮询到后弹确认框；用户点击确认 → `ConfirmUpdate.cgi` 发 IPC 消息 `OSAL_MSG_ID_INTER_OTA_CONFIRM_ACK` → dispatch `ConfirmEvent{true}` 进入 UPDATING。

**6. UPDATING —— 执行安装**

- FORCE 模式：`nohup sh OTAImage > ota_install.log 2>&1 &`
- SILENT 模式：前置 `EFFECT_AFTER_REBOOT=1` 环境变量，升级后重启生效
- 安装由升级包自解压脚本完成（见 2.3）

**7. 异常恢复**

- 订阅 ComAgent 的 MQTT 连接状态，重连成功 dispatch `MqttReconnectEvent` 重新查询
- ERROR 状态有 30s 兜底定时器自动重试，防止 topic 注册失败卡死

**状态同步**：每次状态迁移都做两路通知——

- 云端：telemetry 上报 `{current_sw_title, current_sw_version, sw_state, sw_timestamp, serial_number}`
- 本地：写 MetaData 的 `OTA/OTAState`，并映射为进度条数值（DOWNLOADING→20、VERIFYING→50、UPDATING→80、SUCCESS→100），前端 `Upgradefile.cgi` 轮询展示

### 2.3 A/B 双分区与自解压升级包

**自解压包结构**：

```
[Shell 安装脚本]
END_OF_SCRIPT_START_OF_INFO
Md5Value: <hex>
...版本信息...
END_OF_INFO_START_OF_PACKAGE
[UpdatePackage.tar 二进制]
```

**安装脚本执行流程**（`docapEdgeHeader.sh` 的 `do_upgrade`）：

1. 提取 tar 包并校验 MD5，写 `.status = 1`
2. 解压到 `/tmp/grce/tmpimg/`
3. 非重启生效模式：停掉 `docapedge`、`DICtrl` 服务，LED 闪烁提示
4. 读 `/usr/grce/.reserve/activeIndex`，0 → 装到 IMGB，1 → 装到 IMGA，实现 A/B 轮换
5. 调 `create_image_link.sh`：旧文件备份到旧分区，根目录文件创建指向新分区的**符号链接**——"切换分区"就是改链接指向 + 写 activeIndex，原子且可回滚
6. 顺带检测并升级 TC387 固件（`/etc/config/docap_tc3xx_develop.hex`）
7. 写 `.status = 2`，FORCE 模式直接 `reboot`；SILENT 模式只建链接，下次重启生效

**`.status` 状态码**：0=初始化，1=升级中，2=成功待确认，3=成功已确认（B），4=升级 A，5=失败 B。

**失败回滚**：A/B 双分区 + 符号链接切换，旧分区完整保留；升级中断重启后通过 `.status` 检测出失败并上报。

### 2.4 分片下载与断点续传

断点续传三要素（`ImageDownload::doDownload`）：

1. 下载文件保留在磁盘：`/usr/grce/OTA/upgrade/OTAImage`
2. checksum 文件记录该文件对应哪个包：`/usr/grce/OTA/upgrade/checksum`
3. 下载前比对：本地 checksum == 云端本次下发的 checksum → 用 `已下载文件大小 ÷ 512KB` 算出当前分片号，接着请求；不匹配 → 从头下载

断电/断网重启后，状态机重新走查询流程，拿到同样的包信息即可续传，不需要整包重下。

### 2.5 本地 Web 升级

云端 OTA 的"手动版"：用户从网页上传升级包（与云端下发同一格式），`LocalUpgrade.cgi` 存到 `/tmp/OTAUpgrade.bin`，`chmod +x` 后后台执行。后续解包、A/B 切换、`.status` 写入、进度条更新流程与云端完全一致。

配套 CGI：

- `Upgradefile.cgi`：返回 `OTA/ProcessBar` 进度条数值
- `ConfirmUpdate.cgi`：发送升级确认 IPC；`StatusClear` 参数重置 `.status`

### 2.6 5G 模组 AG55 升级链路

跨三个进程、两块处理器的升级路径：

1. **触发**：CGI `notify5GUpgrade` 发 IPC `OSAL_MSG_ID_INTER_5GAGENT_UPGRADE_AG55` 给 5GAgent
2. **主控侧 5GAgent**：
   - 启动 TCP 文件服务器 `ag55_ota_server -p 6666 -f /tmp/ag55.qwe &`（accept 后先发 `file_size + md5` 头，再持续发文件流）
   - 通过 TBOX-5G 私有协议（自定义帧头 magic + frameid + CRC32）向 5G 模组发 `CMDID_UPGRADE(0x60)` 命令，payload 为 `{serverIP, serverPort}`
3. **5G 模组侧**：收到命令后拉起 OTA Client，TCP 连上主控收文件
4. **流式刷写**：Client 用 `socketpair` 建管道，一边收 TCP 数据写管道，另一边线程调移远 `ql_stream_fwupdate_run(fd)` 流式写入非活动分区——数据不等收完就开始刷
5. **分区切换**：轮询 `ql_stream_fwupdate_getinfo`，`WRITEDONE` → `ql_absys_switch()` 切分区 + 重启；`FAILED` → `ql_absys_sync()` 同步回滚
6. **进度回传**：5GAgent 定时发 `CMDID_UPGRADE_STATUS(0x03)` 查询，模组回报百分比，映射到 `OTA/ProcessBar`，前端进度条可见；结束后 `killall ag55_ota_server` 清理

### 2.7 TC387 子板 HEX 升级

通过以太网对 TC387 子板刷 Intel HEX 固件（`EdgeCore/remoteUpgrade/remoteUpdate.cpp`），请求-响应式块传输协议（`upgrade_message.h`）：

1. `versionReq_t` 查子板当前版本，与 HEX 第一行（约定为版本号）比较，相同则跳过
2. `OTA_UPGRADE_REQ_t` 确认子板就绪（`isReady == 1`）
3. 阻塞等待子板回 `OTA_UPGRADE_ERASE_SUC_t`（Flash 擦除完成，60s 超时）
4. 逐行解析 Intel HEX：跳过首行版本信息、校验每行 checksum、type 0x04 更新扩展基址（高 8 位 0x80 替换为 0xA0）、type 0x00 数据记录计算 `fullAddr = baseAddr + addr + 0x600000`，封装 `OTA_TRANSFER_DATA_REQ_t{seqNum, address, data}` 逐块发送并等确认，同时累积 MD5
5. 发 `OTA_TRANSFER_END_REQ_t{totalSeqNum, md5}`，子板校验 MD5 匹配
6. 发 `OTA_RESET_REQ_t` 复位子板，等待网口恢复、重新建链
7. 再次查询版本，与升级包一致才判定成功

---

## 三、PREMIUM 设备与 TC387 子板通信

### 3.1 硬件架构与板型抽象

PREMIUM 版是双处理器架构：S32G 运行 Linux 应用，TC387 MCU 子板扩展 8 路 CAN + 16 路 LIN + FlexRay，两板通过车载以太网互联，子板固定 IP `192.168.1.133`。

板型在运行时通过 `getDeviceType()` 从 EEPROM/SN 识别（`device_type.h`），`DEVTYPE_PREMIUM_BIT = 0x04`。各模块通过 `getDevTypeUse()` 声明自己支持哪些板型，例如 timeSync、Flexray 仅 PREMIUM 启用。

### 3.2 通道映射表

核心抽象：**上层业务不关心某路 CAN 是本机的还是 TC387 的**。

`Common/common/src/logicCanMap.cpp` 按板型定义通道映射表，每项为 `{物理名, 逻辑名, 接口类型, 远端接口索引}`：

```cpp
// PREMIUM 板型（premiumLogicMapping）：
// CAN1~CAN14  → 本地 main_mcanX（INTERFACE_LOCAL）
// CAN15~CAN22 → TC387（INTERFACE_REMOTE, RemoteInterfaceIndex::CAN0~CAN7）
{0x0e, "remotecan0", "CAN15", ..., INTERFACE_REMOTE, RemoteInterfaceIndex::CAN0},
// LIN1~LIN16  → 全部走 TC387（RemoteInterfaceIndex::LIN0~LIN15）
// FlexRay1    → TC387（RemoteInterfaceIndex::FLEXRAY0）
```

`RemoteInterfaceIndex` 枚举统一编号远端通道：CAN0~CAN15、LIN0~LIN15、FLEXRAY0~4。

上层统一拿 `CanIf` 接口对象，`canBase::createInterface()` 工厂查表：LOCAL → `new CanLocal`（SocketCAN），REMOTE → `new CanRemote`（TC387 通道）。**同一套采集/存储/上传代码不改一行，同时跑在单板 SocketCAN 和双板远程 CAN 上。**

### 3.3 通信协议栈：数据面与控制面分离

#### 数据面（子板 → 主控，高频）：UDP 定长聚合帧

TC387 采集到总线报文后按聚合帧格式经 UDP 上报，端口按总线类型区分：

| 端口 | 业务 |
|---|---|
| 2345 | CAN |
| 2346 | LIN |
| 2347 | FlexRay |
| 2348 | 时间同步 |
| 2349 | OTA 升级 |
| 2350 | 风扇/温度 |

CAN 聚合帧（`rcp_message.h`，`__attribute__((packed))`）：

```c
typedef struct __attribute__((packed)) {
    uint8_t  flag;      // CAN_FRAME_DATA / CONTROL / CONFIG_RESPONSE / SLEEP
    uint8_t  bitmap;    // 哪些通道有数据
    uint32_t udpCount;  // UDP 序号，用于丢帧检测
    union {
        CAN_UDP_Frame_t frame[8];   // 8 通道 {ch, fd, dlc, messageId, data[64]}
        CanConfigUDP_t  ctl[8];
        Config_Response_t response[8];
    };
} CAN_UDP_t;
```

设计动机：CAN 满负载帧率极高，一报文一 UDP 包会压垮协议栈；**8 通道聚合成一个定长包，解析就是一次 memcpy，零序列化开销**。`bitmap` 标记有效通道，`udpCount` 递增序号用于丢帧统计（接收侧检测序号跳变，累计 `totalDropped_` 并记日志）。

LIN 聚合帧同理：`LIN_UDP_t` 支持 16 通道，帧结构含 `{channel, pid, dir, dl, data[8]}`，并支持调度表下发和 Master/Slave 配置。

#### 控制面（主控 ↔ 子板，低频）：nanopb/Protobuf 请求-响应

配置下发、版本查询、OTA、时间同步等走 `RcpMessage`（`rcp_message.proto`），用 **nanopb**（适配 MCU 资源受限环境），`oneof payload` 涵盖 35 种消息：CAN/LIN/FlexRay 配置、心跳、时间同步、OTA 全套、CAN 休眠、风扇控制。

C++ 侧用模板实现**编译期类型安全的 RPC 封装**：

```cpp
// 请求类型自动推导响应类型
template <typename Req> auto request(const Req &data)
    -> std::optional<response_of_t<Req>>;
// response_of<CanConfig_t>::type = CanConfigResp_t
// response_of<OTA_TRANSFER_DATA_REQ_t>::type = OTA_TRANSFER_DATA_RESP_t
```

`setPayload` 用 `if constexpr` 按类型自动打 oneof tag 和消息 flag，传错类型 `static_assert` 编译失败。

**为什么数据面用 packed 结构体、控制面用 protobuf？** 数据面固定格式、极高频率，序列化开销和包大小敏感，定长 packed 零成本解析；控制面消息类型多且持续演进，protobuf 的可扩展性更重要。**按频率和演进性选协议，不是一刀切。**

### 3.4 主控侧接收链路与性能优化

```
TC387 → UDP:2345 → MessageDispatcher（32MB 环形缓冲）
      → 3 个消费线程（SCHED_FIFO 99，绑 CPU6/7）批量解析
      → 按 ifIndex 分发 → RemoteConnection 阻塞队列
      → CanRemote::canRead → 采集线程（SCHED_FIFO 55，绑 CPU1）
      → canFrameQueue 无锁队列 → DFTsever 落盘 / ComAgent 上云 / 边缘计算
```

**MessageDispatcher（单例）**：

- UdpClient 收各端口数据写入 32MB `CircularBuffer` 环形缓冲；写满时按聚合帧整数倍丢弃最老数据，防止解析出半帧
- 3 个消费者线程：`SCHED_FIFO` 优先级 99、绑核 CPU6/7；批量按 `udpSize` 整数倍读出再解析，减少锁竞争和唤醒次数
- 解析：按包长判断类型，CAN 走 `processCanData`——拆聚合帧、按 bitmap 遍历有效通道、还原成 `RcpMessage`（区分标准帧/CANFD，DLC 长度换算）
- 分发：按 `ifIndex` 查 handler 表，响应进 `respQueue_`，数据进 `recvDataQueue_`

**RemoteConnection（每通道一个）**：

- `request()`：同步语义，写请求后阻塞 4s 等响应
- `read()` / `readRaw()`：阻塞带超时取数据帧，区分 `TIMEOUT` / `DISCONNECTED` / `INVALID_PAYLOAD`
- 丢帧检测：`detectUdpDropout` 按 `udpCount` 序号统计

**采集线程（canReadThreadFunc）**：

- 每路 CAN 一个线程，普通 CAN 设 `SCHED_FIFO(55)` 绑 CPU1，LLCE CAN 绑 CPU3（避开 LLCE 软中断核）
- 读到帧推入全局无锁队列 `canFrameQueue`，下游三类消费者：
  - **DFTsever**：落盘 BLF/MF4/ASC
  - **ComAgent**：实时上云
  - **边缘计算触发器**：触发前进历史环形队列，触发后把触发点前 N 秒数据打 `TRIGGER_FLAG_START` 一并落盘（触发前数据回溯）

### 3.5 本地/远程透明切换（接口抽象 + 工厂）

`CanIf` 抽象接口（`canRead/canWrite/up/down/isCanFd`），两个实现：

- `CanLocal`：SocketCAN，直接读内核 CAN 设备
- `CanRemote`：TC387 通道
  - `up()`：建连后从 MetaData 读配置（波特率、采样点、CANFD 开关），组装 `CanConfig_t` 发给子板，子板打开对应 CAN 控制器
  - `canRead()`：`readRaw()` 取一条 `RcpMessage`，转统一 `canFrameMessage`（时间戳、EFF/RTR/ERR 标志、BRS/ESI）
  - `canWrite()`：`CanFrame_t`/`CanfdFrame_t` 下发，子板代发到总线

`canBase` 用共享内存引用计数（`m_countShm`）：同一通道被多个线程使用时只真正 up/down 一次。

LIN 侧（`LinRemoteInterface`）额外支持：LDF 文件解析、按 PID 下发调度表（`LinConfigScheduleTable_t`）、Master/Slave 模式、经典/增强校验选择。

### 3.6 双时钟域时间同步

问题：TC387 和 S32G 是两个时钟域，CAN 帧时间戳由子板打，不同步会导致落盘数据时间轴错位。

方案（`timeSync` 模块，仅 PREMIUM）：每 20 秒——

1. 拉一个 **GPIO 硬件脉冲**给 TC387 做硬同步锚点
2. 同时发 `TimeSync_t{timestamp}` 应用层请求（端口 2348）

软硬结合校准，保证多板采集数据的时间戳在同一时间轴上可拼接。

---

## 四、简历内容

### 4.1 项目描述（详细版）

> **车规级数据采集终端（T-BOX）嵌入式软件开发**
> 基于 NXP S32G + 英飞凌 TC387 双处理器架构，支持 22 路 CAN/CANFD、16 路 LIN、FlexRay、车载以太网数据采集、存储与上云
>
> **板间通信框架（S32G ↔ TC387）**
> - 设计并实现双处理器以太网通信框架：上行数据面采用 UDP 定长聚合帧协议（8 通道 CAN 合一包、bitmap 通道标识、序号丢帧检测，memcpy 零序列化开销解析）；控制面采用 nanopb/Protobuf 请求-响应协议（oneof 涵盖 35 种消息），按"频率与演进性"分层选型
> - 基于 C++17 模板（`if constexpr` + 类型萃取）实现编译期类型安全的 RPC 封装，请求/响应类型自动绑定，错误传参编译期拦截
> - 通过接口抽象 + 工厂模式 + 板型通道映射表，实现 CAN/LIN 通道本地（SocketCAN/LLCE）与远程（TC387）透明切换，同一代码库支持 5 种板型
> - 高负载采集性能优化：32MB 无锁环形缓冲、消费线程 SCHED_FIFO(99) 实时调度 + CPU 绑核、批量解析、聚合帧对齐丢弃，支撑多路 CANFD 满负载采集
> - 实现主从双时钟域时间同步：GPIO 硬件脉冲 + 应用层时间戳校准，保证多板采集数据时间戳对齐
>
> **OTA 升级系统**
> - 设计并实现基于 tinyfsm 状态机的云端 OTA：MQTT 属性查询版本 → 512KB 分片下载（10s 超时 3 次重试）→ SHA256 校验 → 用户确认/静默/强制三种模式 → 安装重启，状态全链路双路上报（云端 telemetry + 本地进度条）
> - 设计自解压升级包格式（脚本 + MD5/版本信息 + tar 包），配合 A/B 双分区符号链接切换，升级原子完成、失败可回滚；`.status` 文件实现重启后升级结果回填
> - 实现下载断点续传：checksum 文件比对 + 按已下载大小定位分片号，断电断网后续传无需整包重下
> - 实现跨处理器固件升级：5G 模组（移远 AG55）私有协议命令 + TCP 文件服务 + `ql_stream_fwupdate_run` 流式刷写非活动分区；TC387 子板 Intel HEX 解析、逐块确认传输、MD5 完整性校验、复位后版本验证

### 4.2 项目描述（精简版）

> 负责车规级数据采集终端的通信中间件与 OTA 系统开发，覆盖 S32G（Linux）↔ TC387（MCU）板间通信、5G 模组升级、云端 OTA 三大链路。
>
> - 自研板间通信协议栈：UDP 聚合帧数据面 + nanopb 控制面，支撑 8 路 CANFD 满负载采集；C++17 模板化 RPC 框架，请求/响应类型编译期绑定
> - 接口抽象 + 工厂 + 通道映射表实现本地/远程总线透明访问，一套代码适配 5 种板型
> - 实时性优化：SCHED_FIFO 实时调度、CPU 绑核、32MB 无锁环形缓冲、批量消费，丢帧率可观测（序号检测）
> - 基于 tinyfsm 的云端 OTA 状态机 + A/B 双分区自解压升级包 + 512KB 分片断点续传 + SHA256 校验
> - 跨处理器固件升级：5G 模组流式刷写、TC387 HEX 逐块确认升级

### 4.3 技术栈关键词

```
语言/标准：   C++17（模板元编程、if constexpr、类型萃取）、C、Shell
网络编程：    Linux 用户态 UDP/TCP、Epoll、Unix Domain Socket IPC
协议：        Protobuf/nanopb、MQTT（ThingsBoard 风格 topic）、自定义二进制协议、TBOX-5G 私有协议
车载总线：    CAN / CANFD / LIN / FlexRay、SocketCAN、LLCE、UDS 诊断
实时性能：    SCHED_FIFO 实时调度、CPU 亲和性绑核、无锁环形缓冲、生产者-消费者模型、批量处理
OTA：         A/B 双分区、符号链接切换、断点续传、SHA256/MD5 校验、自解压升级包、A/B 分区回滚
状态机：      tinyfsm
固件相关：    Intel HEX 解析、双时钟域时间同步（GPIO 脉冲 + 应用层校准）、双处理器架构（S32G + TC387）
工具链：      CMake、交叉编译、CGI Web 配置界面、systemd 服务管理
```

### 4.4 可量化指标

> 按实际情况填写数字：

- 通道规模：22 路 CAN/CANFD + 16 路 LIN + 1 路 FlexRay（PREMIUM 板型），一套代码支持 5 种板型
- 单聚合帧 8 通道复用，解析零序列化开销；消费线程实时优先级 99 + 绑核，采集线程 FIFO(55)
- OTA 分片 512KB，10s 超时 3 次重试，断点续传按 checksum 对齐；A/B 双分区升级失败可回滚
- 板间通信 35 种 protobuf 消息类型，4s 同步请求超时；UDP 序号丢帧检测全程可观测

---

## 五、面试问答准备

**Q1：OTA 状态机为什么用状态机而不是顺序流程？**

云端 OTA 是异步事件驱动的：MQTT 响应、用户确认、超时、断连重连都是事件。tinyfsm 让每个状态只响应自己关心的事件，异常路径（下载中断连、用户拒绝升级）都有明确转移，而不是一堆 if-else。

**Q2：升级失败怎么回滚？**

两层保障：一是 A/B 双分区 + 符号链接切换（改链接指向 + 写 activeIndex 是原子的），旧分区完整保留，可从旧分区启动；二是 `.status` 状态文件，升级中断重启后能检测失败并上报云端。

**Q3：MQTT 断了怎么办？**

OTA 订阅 ComAgent 的 MQTT 连接状态，收到 `STATE_MQTT_CONNECTED` 就 dispatch `MqttReconnectEvent` 重新注册 topic 查询版本；ERROR 状态另有 30s 兜底定时器，防止 topic 注册失败永久卡死。

**Q4：断点续传的原理？**

三要素：下载文件保留磁盘、checksum 文件记录文件对应哪个包、下载前比对。checksum 匹配 → 文件大小 ÷ 512KB 算出当前分片号继续请求；不匹配 → 从头下载。

**Q5：云端怎么区分 Edge 固件和 5G 固件？**

`sw_title` 约定：带 `_5G_` 字样即 5G 固件。解析时标记到 `ImageInfo.title`，后续版本比较（比 `CommProjectRev` 而非 `SoftwareVersion`）和升级路径都按它分流。

**Q6：升级包为什么做成自解压脚本？**

原子性（一个文件 `sh` 执行即装，不依赖外部工具链）、自校验（内嵌 MD5 和版本信息）、模式差异只需一个环境变量（`EFFECT_AFTER_REBOOT`）传给同一脚本。

**Q7：数据面为什么用 packed 结构体而控制面用 protobuf？**

数据面固定格式、极高频率，序列化开销和包大小敏感，定长 packed 可 memcpy 零成本解析；控制面消息类型多（35 种）且持续演进，protobuf 可扩展性更重要。按频率和演进性选协议，不是一刀切。

**Q8：UDP 丢包怎么办？**

控制面有请求-响应超时重试；数据面采集场景丢一帧比重传更重要的是不阻塞，但每包带 `udpCount` 序号，接收侧统计丢帧数并记日志，问题可观测。环形缓冲溢出按整帧对齐丢弃，不会解析出半帧。

**Q9：双时钟域怎么处理？**

GPIO 硬件脉冲 + 应用层时间戳请求组合方案，20s 周期校准。采集线程使用子板打的时间戳（已同步），落盘时多板数据才能按时间轴对齐。

**Q10：怎么做到本地/远程 CAN 透明切换？**

三层：`CanIf` 接口抽象（canRead/canWrite/up/down）+ 工厂按板型映射表创建 `CanLocal`/`CanRemote` + 共享内存引用计数（同一通道多线程使用只 up/down 一次）。上层采集、存储、上云、诊断代码完全不感知差异。

**Q11：CAN 满负载时主控怎么扛住？**

四级优化：① 子板侧 8 通道聚合成一个定长 UDP 包，减少包数；② 32MB 环形缓冲削峰；③ 消费线程 SCHED_FIFO(99) + 绑核 CPU6/7，批量按帧对齐解析；④ 采集线程 FIFO(55) + 绑核 CPU1/3（避开 LLCE 软中断核），无锁队列交接给下游。

**Q12：TC387 升级过程中怎么保证刷写正确？**

逐块确认：每块 `OTA_TRANSFER_DATA_REQ_t{seqNum, address, data}` 都等子板响应；全程累积 MD5，结尾 `OTA_TRANSFER_END_REQ_t` 携带 MD5 由子板校验；复位重连后再查版本，与升级包第一行版本号一致才判定成功。
