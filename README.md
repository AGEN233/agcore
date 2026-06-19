# AGCORE

AGCORE 是 AGOS 工程中的核心组件，包含 BLE 数据通道、设备信息、版本信息、Pixel Driver、日志和通用工具模块。

## 目录

- `BLE/`: AGCORE BLE 广播、GATT、GAP 和数据收发。
- `DataChannel/`: AGCORE 数据队列和事件路由。
- `PixelDriver/`: 像素灯驱动配置和实现。
- `PrintfLog/`: AGCORE 与应用日志接口。
- `Utils/`: 校验等通用工具。
- `Version/`: 设备信息和 AGCORE 版本信息管理。

## 使用

在 ESP-IDF 工程中将本目录作为组件引入，并在 `menuconfig` 的 `AGCORE CONFIG` 下配置相关选项。
