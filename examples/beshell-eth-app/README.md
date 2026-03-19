# BeShell-Ethernet (W5500) Example App

这是一个演示 BeShell-Ethernet (W5500 SPI 以太网驱动) 用法的示例工程。

## 功能特性

本示例工程演示了以下功能：

### 以太网功能

| 示例文件 | 说明 |
|---------|------|
| `basic-setup.js` | W5500 基础初始化，DHCP 获取 IP |
| `static-ip.js` | 配置静态 IP 地址 |
| `http-client.js` | 通过以太网发送 HTTP 请求 |
| `http-server.js` | 在以太网上创建 HTTP 服务器 |
| `mqtt-client.js` | 通过以太网连接 MQTT Broker |
| `tcp-client.js` | 原始 TCP 套接字通信 |
| `ping-test.js` | 网络连通性测试 |

## 项目结构

```
beshell-eth-app/
├── CMakeLists.txt          # 项目 CMake 配置
├── sdkconfig.defaults      # 默认 SDK 配置
├── README.md               # 本文件
├── main/
│   ├── main.cpp            # C++ 入口文件
│   ├── CMakeLists.txt      # main 组件 CMake 配置
│   └── idf_component.yml   # 组件依赖配置
├── img/
│   ├── partitions-4MB.csv  # 4MB Flash 分区表
│   ├── partitions-8MB.csv  # 8MB Flash 分区表
│   └── partitions-16MB.csv # 16MB Flash 分区表
└── js/
    ├── main.js             # JS 入口文件
    └── example/            # 示例脚本目录
        ├── basic-setup.js
        ├── static-ip.js
        ├── http-client.js
        ├── http-server.js
        ├── mqtt-client.js
        ├── tcp-client.js
        └── ping-test.js
```

## 硬件要求

- ESP32 / ESP32-S3 / ESP32-C3 等芯片
- W5500 SPI 以太网模块
- 至少 4MB Flash
- USB 转串口用于烧录和调试

### 硬件连接 (W5500)

```
ESP32          W5500
-----          -----
GPIO 5    -->  CS
GPIO 18   -->  SCLK
GPIO 19   -->  MISO
GPIO 23   -->  MOSI
GPIO 4    -->  RST (可选)
GPIO 34   -->  INT (可选)
3.3V      -->  VCC
GND       -->  GND
```

**注意**：SPI 引脚可能因 ESP32 型号而异，请根据实际硬件调整。

## 快速开始

### 1. 修改 SPI 配置

根据你的硬件连接修改示例中的配置：

```javascript
const SPI_HOST = 2    // SPI2 (HSPI)
const PIN_CS = 5      // CS 引脚
```

### 2. 构建项目

```bash
idf.py build
```

### 3. 烧录固件

```bash
idf.py flash
```

### 4. 查看串口输出

```bash
idf.py monitor
```

### 5. 运行示例

在串口终端中，输入以下命令运行示例：

```
run /example/basic-setup.js
```

## 示例详解

### 基础设置 (DHCP)

```javascript
import { W5500 } from "w5500"

let eth = new W5500()

// 监听网络事件
eth.on("connected", () => {
    console.log("Ethernet link connected")
})

eth.on("ip.got", (info) => {
    console.log("IP:", info.ip)
    console.log("Netmask:", info.netmask)
    console.log("Gateway:", info.gw)
})

// 初始化 W5500
eth.setup({
    spi: 2,           // SPI 主机
    cs: 5,            // CS 引脚
    rst: -1,          // RST 引脚 (-1 表示不使用)
    intr: -1,         // INT 引脚 (-1 表示不使用)
    clock_mhz: 16,    // SPI 时钟频率
    polling_ms: 0,    // 0 表示中断模式
    phy_addr: 1       // PHY 地址
})
```

### 静态 IP 配置

```javascript
// 等待连接建立
await new Promise((resolve) => {
    eth.once("connected", resolve)
})

// 设置静态 IP
eth.setIP("192.168.1.100", "255.255.255.0", "192.168.1.1")
```

### HTTP 客户端

```javascript
import * as mg from "mg"

// 等待以太网就绪后
let body = await mg.get("http://api.example.com/data")
console.log(body.toString())
```

### HTTP 服务器

```javascript
mg.listenHttp("0.0.0.0:8080", (event, req, rspn) => {
    if (event === "http.msg") {
        rspn.setStatus(200)
        rspn.setHeader("Content-Type", "text/html")
        rspn.send("<h1>Hello from W5500!</h1>")
    }
})
```

## API 参考

### W5500 类

#### constructor()
创建 W5500 对象。

#### setup(config)
初始化 W5500 模块。
- `config.spi`: SPI 主机号 (0-2)
- `config.cs`: CS 引脚号
- `config.rst`: RST 引脚号 (-1 表示不使用)
- `config.intr`: INT 引脚号 (-1 表示不使用)
- `config.clock_mhz`: SPI 时钟频率 (MHz)
- `config.polling_ms`: 轮询间隔 (0 表示中断模式)
- `config.phy_addr`: PHY 地址

#### setIP(ip, netmask, gateway)
设置静态 IP 地址。

#### getMAC()
获取 MAC 地址。

### 事件

- `start`: 以太网启动
- `connected`: 网线连接
- `disconnected`: 网线断开
- `stop`: 以太网停止
- `ip.got`: 获取到 IP 地址
- `ip.lost`: IP 地址丢失

## 依赖组件

- [beshell](https://github.com/become-cool/beshell) - BeShell 核心框架
- [beshell-drv-eth](https://github.com/become-cool/beshell-drv-eth) - W5500 以太网驱动
- [beshell-mg](https://github.com/become-cool/beshell-mg) - 网络组件

## 许可证

LGPL

## 相关链接

- [BeShell 文档](https://beshell.become.cool)
- [W5500 数据手册](https://www.wiznet.io/wp-content/uploads/wiznethome/Chip/W5500/Documents/W5500_Datasheet_v1.0.2_1.pdf)
- [ESP-IDF 以太网文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_eth.html)
