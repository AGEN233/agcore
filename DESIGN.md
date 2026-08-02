# AGCORE 设计文档

> 本文档用于详细记录 AGCORE 的设计理念、模块划分、数据流与演进路径，便于后续复盘与继续设计。
> 标注 **✅ 已落地** 的为当前实现的现状；标注 **🟡 规划中** 的为已确认但尚未实现的设计方向。

---

## 1. 定位与设计理念

AGCORE 是个人 DIY 项目中沉淀出的 **ESP-IDF 适配抽离层**。位于**应用层**与 **ESP-IDF** 之间，职责是：

- 统一封装项目中反复出现的**能力**（通信链路、数据路由、事件分发、驱动、日志、工具），减少重复开发；
- 保持 **ESP-IDF 原生组件化**开发方式，不额外造一套框架；
- 为上层（application）提供 **统一、稳定、低耦合** 的基础能力。

### 1.1 架构决策流程（重要约定）

做任何功能时，必须按此优先级，**降低重复造轮子**：

1. **优先用 AGCORE 已封装的接口**（port / 各模块）；
2. AGCORE 没有 → 用 **ESP-IDF 标准接口**，并评估该功能**是否需要 AGCORE 统一封装**；
3. 再看**有没有可复用的现成接口**；
4. 实在没有再**自己做**；
5. 全程**降低重复造轮子**。

> 例：时间设置最终直接用 ESP-IDF 标准 `time()` / `settimeofday()`，**没有**在 port 层额外造时间宏封装，因为那是标准 ESP-IDF 已提供的系统能力。

---

## 2. 模块组成

```
agcore/
├─ agcore.h                 # 唯一应用层入口 + 版本宏(AGCORE_VERSION/ID)
├─ agcore_public.h          # 公共平铺聚合头(基础类型/port/日志/各模块)
├─ agcore_port.h            # 平台适配层(时间/内存/日志宏)
├─ data_channel/            # 统一数据通道(接收/发送/路由/封包)
│   ├─ agcore_datachannel.h # 统一数据结构 + 通道接口
│   ├─ agcore_data_queue.c  # 统一收发队列(接收+发送都在此处)
│   ├─ agcore_data_encode.c # 顶层协议 封包/解包
│   ├─ agcore_dataevent_route.c  # 统一数据路由分发
│   └─ agcore_datevent_handle.c  # CORE 命令分发
├─ BLE/                     # BLE 广播/GAP/GATT/收发
├─ pixel_driver/            # ARGB 可寻址 LED 驱动抽象
├─ display_driver/          # 显示驱动(7735/7789)
├─ log/                     # 统一日志输出
├─ Utils/                   # 工具(校验和 + 字节序 helper)
└─ Version/                 # 设备信息 + AGCORE 版本管理
```

---

## 3. 数据模型与两大总线

AGCORE 的核心是**两条正交的通道**：

| | 统一数据总线 | 统一事件总线 |
|---|---|---|
| 承载 | 外部**命令/数据帧**(发收) | 内部**系统状态事件**(广播) |
| 方向 | 请求→响应、多链路收发 | 状态变化→多模块响应 |
| 触发 | 链路收包 / 主动上报 | 模块内部状态改变 |
| 订阅 | 单一(应用层)+ CORE 截留 | **双订阅者**(agcore+应用层) |
| 现状 | ✅ 已落地 | 🟡 规划中 |

**判定原则**：
- **命令**（改动简单、单执行者、无需协调）→ 走**数据总线**，同步处理。例：`SET_DEVICETIME`。
- **状态广播 / 需多模块跨层响应** → 走**事件总线**。例：联网、复位、恢复出厂。

---

## 4. 统一数据总线（✅ 已落地）

### 4.1 统一数据结构

```c
typedef struct {
    agcore_data_link_et link;       // 数据来源链路(BLE/WIFI/UART/...)
    uint8_t sn;                     // 顶层协议序号
    uint16_t cmd;                   // 业务命令字
    uint16_t payload_len;           // 业务载荷长度
    uint8_t payload[AGCORE_DATA_CHANNEL_PAYLOAD_MAX]; // 业务载荷
} agcore_data_st;
```

应用层不关心数据来自哪条链路，只按统一数据头解析命令与载荷。

### 4.2 数据流（接收）

```
多链路 RX
   → agcore_data_push(&data)   入统一接收队列
   → agcore_data_task()        出队
   → agcore_data_route_handler()  统一路由
        ├─ CORE 命令: agcore_data_handle_core()  截留并处理(返回 true)
        └─ 其余: 推应用层回调(agcore_data_cb)
```

- 应用层通过 `agcore_data_handler_register(cb)` 注册统一回调；
- 链路层通过 `agcore_data_push()` 推数据。

### 4.3 数据流（发送）

```
调用方 → agcore_data_send(&data)    入统一发送队列
   → agcore_data_send_task()        出队
   → agcore_data_encode(&data, buf) 统一封包(大端 + SN + 校验)
   → 按 data->link 分发到链路回调    如 agcore_ble_notify(buf, len)
```

- 各链路通过 `agcore_data_send_register(link, cb)` 注册发送回调（BLE 已接）；
- **封包统一在发送队列内完成**，各链路只收已封好的帧，不重复封包；
- **从哪来回哪去**：回包时 `rsp.link = data->link`（目标链路）。

### 4.4 顶层协议帧（Encode/Decode）

`agcore_data_encode()` 生成帧：

```text
[Header 4B][Ver 1B][SN 1B][PayloadLen 2B][Cmd 2B][Payload][CheckSum 1B]
```

- `PayloadLen` / `Cmd` 均为**大端**；
- 帧尾为 8 位累加校验和；
- `agcore_data_decode()` 反向解析，含 SN 去重、版本、校验检查。

### 4.5 CORE 命令分发（✅ 已落地）

`agcore_data_handle_core()` 按 `cmd` 分发，每个命令一个 `agcore_handle_xxx_handle()` 静态函数：

| 命令 | 值 | 状态 | 说明 |
|---|---|---|---|
| `CORE_CMD_GET_VERINFO` | 0xAC00 | ✅ | 回包 device_type/id、fw/hw、agcore_version、gitHash(17B) |
| `CORE_CMD_GET_DEVICETIME` | 0xAC01 | ✅ | 回包 4 字节 Unix 秒(标准 `time(NULL)`) |
| `CORE_CMD_SET_DEVICETIME` | 0xAC02 | ✅ | 解析 u32 → `settimeofday`，判返回值回包 0/1 |

---

## 5. 统一事件总线（🟡 规划中）

### 5.1 设计目标

事件总线用于承载"状态变化需要**同时通知 agcore 与应用层**"的场景，与数据总线解耦。

> 核心场景（例）：**联网成功**
> - agcore 内部：触发配网记录、**时间同步机制**、**MQTT 连接**（跑 AIOT 协议）
> - 应用层：控制 **LAN 指示灯**、刷新 UI 等

### 5.2 设计形态（已确认）

**双订阅者 + 不筛选 + 全广播**：

```
事件 → agcore_evt_publish(evt_id, arg)
   ├─ 订阅者A: agcore 内部回调 (switch → 编排配网/时间/MQTT)
   └─ 订阅者B: 应用层回调     (switch → LED/UI 等)
```

- **不在 AGCORE 内做按事件筛选取舍**，而是全部一起推，订阅者自行 `switch(evt_id)`；
- agcore 内部采用**单一内部订阅者**，其回调内负责编排各机制（配网/时间/MQTT），避免内部多份订阅。

### 5.3 事件 ID 枚举（草拟，待补充）

```c
typedef enum {
    AGCORE_EVT_NONE = 0,
    AGCORE_EVT_NET_CONNECTED,   /* 网络连接成功 */
    AGCORE_EVT_TIME_UPDATED,    /* 时间已更新 */
    AGCORE_EVT_MQTT_CONNECTED,  /* MQTT 已连接 */
    AGCORE_EVT_RESET,           /* 复位 */
    AGCORE_EVT_FACTORY_RESET,   /* 恢复出厂 */
    AGCORE_EVT_MAX
} agcore_event_id_et;
```

### 5.4 落地清单（做事件总线时）

- [ ] 新增 `agcore_event.c/.h`（`agcore_evt_publish` / 内部+应用层注册）；
- [ ] 定义事件 ID 枚举，并从 `agcore_public.h` 导出；
- [ ] 在 `agcore_handle_set_devicetime` 等命令处理完成后，可选发 `TIME_UPDATED`；
- [ ] 待"联网/配网/MQTT"真实需求出现时，接入 `NET_CONNECTED` 的发布与双订阅。

---

## 6. 平台适配层（agcore_port.h）

集中封装硬件/平台相关能力，后续跨平台或换芯片只需改此处：

```c
#define agcore_get_timems()  ((uint32_t)(esp_timer_get_time() / 1000ULL)) // 相对运行毫秒
#define agcore_get_timeus()  ((uint64_t)esp_timer_get_time())             // 相对运行微秒
#define agcore_malloc / agcore_free
#define AGCORE_LOG_PORT(...)  printf(...)
```

**注意**：Unix 时间戳**不在 port 层**加封装——直接采用 ESP-IDF 标准接口 `time()`/`settimeofday()`（见 §1.1 决策流程图，避免为标准能力重复造轮子）。port 层只保留相对运行时间（`get_timems`/`get_timeus`）等平台能力。

---

## 7. 字节序工具（✅ 已落地）

`Utils/agcore_utils_bytes.c` 提供大端字节流与数值互转，避免各处手写位移：

```c
void     agcore_put_bytes16(uint8_t *buf, uint16_t val);  // u16 → 2B 大端
void     agcore_put_bytes32(uint8_t *buf, uint32_t val);  // u32 → 4B 大端
uint16_t agcore_get_bytes16(const uint8_t *buf);          // 2B 大端 → u16
uint32_t agcore_get_bytes32(const uint8_t *buf);          // 4B 大端 → u32
```

已在 `data_channel` / `Version` 等模块统一替换手动拼接。

---

## 8. 头文件组织约定

- 模块统一走 **`agcore_public.h` 平铺聚合**（基础类型 + port + 日志 + 各模块），不逐个 include；
- `agcore.h` 是**唯一应用层入口**，聚合 `agcore_public.h` 并定义版本宏；
- 业务宏（如 CORE 命令号）留在**各自所属模块**头（`agcore_datevent_handle.h`），不塞进 public。

---

## 9. 更待办 / 复盘要点

- [ ] 统一事件总线落地（§5），首个触发点是"联网"场景；
- [ ] 补充 **UART / WIFI** 链路（接收 push + 发送回调注册），当前仅 BLE；
- [ ] 时间同步机制、MQTT/AIOT 协议（依赖网络事件，见 §5.1）；
- [ ] 部分显示驱动 / WS2812B 存在**历史遗留问题**，改造另立专项，未纳入 §7 字节序收敛范围。

---

## 10. 历史决策记录（反「我为什么这么做」）

- **时间设置不搞系统事件**：它是"单执行者、无跨模块协作"的命令，走数据总线同步处理即可，不值得为它造事件；
- **事件总线双订阅不筛选**：联网要同时通知 agcore（配网/时间/MQTT）与应用层（LED）两边，故广播 > 筛选；
- **发送队列统一封包**：把 `agcore_data_encode` 收进发送队列，各链路只收帧，避免重复封包（BLE 不再自己 encode）；
- **字节 helper 命名**：`agcore_put_bytes16` / `agcore_get_bytes32`（动词后置、带位数），统一大端语义。
