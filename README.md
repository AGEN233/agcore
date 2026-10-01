# AGCORE

AGCORE 是个人 DIY 项目中沉淀出来的一套 ESP-IDF 适配抽离层。

它不是完整应用框架，而是位于应用层与 ESP-IDF 之间的基础组件集合，用于统一封装项目中反复使用的能力，减少重复开发，提高不同项目之间的代码复用率和维护性。

## 设计目标

- 拥抱 ESP-IDF 生态，保持 ESP-IDF 原生组件化开发方式。
- 为应用层提供统一、稳定、低耦合的基础能力。
- 抽离常用硬件驱动、通信通道、日志和工具模块。
- 方便在多个 ESP-IDF 项目之间复用。

## 模块组成

- `ble/`

  统一 BLE 广播、GAP、GATT 和数据收发能力。

- `data_channel/`

  统一数据路由入口，负责把来自 BLE、Wi-Fi、UART 等链路的数据整理成同一种数据结构，再交给应用层处理。

- `pixel_driver/`

  ARGB 可寻址 LED 驱动抽象层，目前包含 WS2812B 相关实现。

- `log/`

  统一日志输出封装。

- `utils/`

  校验和通用工具函数。

- `version/`

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

如果应用层还没有调用 `agcore_device_info_set()` 设置设备信息，AGCORE 会为广播设备字段填充默认字节：

```text
00 FF 00 FF 00 FF 00 FF
```

默认值只用于广播填充，不会覆盖应用层保存的设备信息。

## 统一数据路由

AGCORE 将不同链路的数据统一抽象为 `agcore_data_t`。多个接收回调可以分别注册，所有回调同步收到完整的、不透明的 payload。

当前支持的数据来源：

- `LINK_BLE`
- `LINK_WIFI`
- `LINK_UART`

应用层通过以下接口注册数据回调：

```c
void agcore_data_handler_register(agcore_data_cb cb);
```

链路层通过以下接口把数据推入 AGCORE 数据队列：

```c
void agcore_data_push(agcore_link link, const uint8_t *raw, uint16_t raw_len);
```

## 统一数据头

AGCORE 内部统一数据结构定义如下：

```c
typedef struct {
    agcore_link link;
    uint8_t sn;
    uint16_t payload_len;
    uint8_t *payload;
} agcore_data_t;
```

字段说明：

- `link`: 数据来源链路。
- `sn`: 顶层协议序号。
- `payload_len`: 顶层协议 payload 的字节数。
- `payload`: 不透明字节串。AGCORE 的队列、编解码和通用路由不解释其中的业务格式。

链路层入队前会复制原始帧；接收任务解码后同步调用所有回调，回调返回后释放帧缓冲区。需要命令的 CORE 或应用回调自行解析 payload，不能保留接收 payload 指针。发送接口同样将 payload 作为不透明字节串，并在入队前复制。

## 使用方式

将 AGCORE 作为 ESP-IDF component 引入工程，并在 `menuconfig -> AGCORE CONFIG` 下配置相关功能。
