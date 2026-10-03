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
esp_err_t agcore_data_push(agcore_link link, uint8_t *raw, uint16_t raw_len);
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

链路层提交完整帧的堆缓冲区；`agcore_data_push()` 成功时转移所有权，失败时调用方负责释放。接收任务解码后同步调用所有回调，全部回调返回后释放帧缓冲区。回调只借用 payload，不得释放、保存指针或修改共享内容；应答使用独立发送数据。发送接口在当前任务中封包并提交链路。

### BLE 分包

现有写入特征 `0x4301` 和 Notify 特征 `0x4302` 均使用独立分包头，每片格式为 `total_len[2] | offset[2] | data[N]`，两个字段均为大端。`total_len` 包含整个顶层帧（含顶层头和校验），`offset` 是本片数据在完整帧中的字节偏移。单片消息也必须携带分包头，偏移为 0。

接收端只允许一个正在重组的消息：偏移为 0 开始或替换当前消息，后续片总长度必须一致且偏移连续；格式错误会丢弃当前重组。完整消息才进入统一队列，其他链路可直接提交完整帧。分包编解码在 `pipe/data/protocol/agcore_data_protocol.c`，不解释顶层协议。

`CONFIG_AGCORE_BLE_MESSAGE_MAX_LEN` 默认 4096 字节；`CONFIG_AGCORE_BLE_FRAGMENT_TIMEOUT_MS` 默认 5000 毫秒，按最后一片接收时间计时。超时、断连和重连均清理未完成重组。Notify 按实际 MTU 切片，每片数据上限为 `MTU - 7`。分包层不额外添加序号，使用连续偏移检查顺序；顶层协议序号保持独立。

## 使用方式

将 AGCORE 作为 ESP-IDF component 引入工程，并在 `menuconfig -> AGCORE CONFIG` 下配置相关功能。
