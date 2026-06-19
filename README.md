# AGCORE

AGCORE 是个人 DIY 项目中沉淀出来的一套 ESP-IDF 适配抽离层。

它不是完整应用框架，而是位于应用层与 ESP-IDF 之间的基础组件集合，用于统一封装项目中反复使用的能力，减少重复开发，提高不同项目之间的代码复用率和维护性。

## 设计目标

- 拥抱 ESP-IDF 生态，保持 ESP-IDF 原生组件化开发方式。
- 为应用层提供统一、稳定、低耦合的基础能力。
- 抽离常用硬件驱动、通信通道、日志和工具模块。
- 方便在多个 ESP-IDF 项目之间复用。

## 模块组成

- `BLE/`

  统一 BLE 广播、GAP、GATT 和数据收发能力。

- `DataChannel/`

  统一数据路由入口，负责把来自 BLE、Wi-Fi、UART 等链路的数据整理成同一种数据结构，再交给应用层处理。

- `PixelDriver/`

  ARGB 可寻址 LED 驱动抽象层，目前包含 WS2812B 相关实现。

- `PrintfLog/`

  统一日志输出封装。

- `Utils/`

  校验和通用工具函数。

- `Version/`

  设备信息和 AGCORE 版本信息管理。

## 统一蓝牙广播

AGCORE BLE 使用统一的广播格式，对外暴露稳定的设备识别信息。

广播名称格式：

```text
AGIOT + MAC
```

扫描响应中的厂商数据格式：

```text
AGCORE + device_type + device_id + fw_version + hw_version
```

字段说明：

- `AGCORE`: 固定头，6 字节。
- `device_type`: 设备类型，2 字节。
- `device_id`: 设备 ID，2 字节。
- `fw_version`: 固件版本，2 字节。
- `hw_version`: 硬件版本，2 字节。

如果应用层还没有调用 `agcore_set_device_info()` 设置设备信息，AGCORE 会为广播设备字段填充默认字节：

```text
00 FF 00 FF 00 FF 00 FF
```

默认值只用于广播填充，不会覆盖应用层保存的设备信息。

## 统一数据路由

AGCORE 将不同链路的数据统一抽象为 `agcore_data_st`，应用层只需要注册一个统一的数据处理回调。

当前支持的数据来源：

- `AGCORE_DATA_LINK_BLE`
- `AGCORE_DATA_LINK_WIFI`
- `AGCORE_DATA_LINK_UART`

应用层通过以下接口注册数据回调：

```c
void agcore_data_handler_register(agcore_data_cb cb);
```

链路层通过以下接口把数据推入 AGCORE 数据队列：

```c
void agcore_data_push(agcore_data_link_et link, uint16_t cmd, const uint8_t *payload, uint16_t payload_len);
```

## 统一数据头

AGCORE 内部统一数据结构定义如下：

```c
typedef struct {
    agcore_data_link_et link;
    uint16_t cmd;
    uint16_t payload_len;
    uint8_t payload[AGCORE_DATA_CHANNEL_PAYLOAD_MAX];
} agcore_data_st;
```

字段说明：

- `link`: 数据来源链路。
- `cmd`: 应用命令字。
- `payload_len`: 载荷长度。
- `payload`: 载荷数据。

这种结构让应用层不用关心数据来自 BLE、Wi-Fi 还是 UART，只需要按照统一的数据头解析命令和载荷。

## 使用方式

将 AGCORE 作为 ESP-IDF component 引入工程，并在 `menuconfig -> AGCORE CONFIG` 下配置相关功能。
