# BeShell-Ethernet (W5500) Example App

This is an example project demonstrating the usage of BeShell-Ethernet (W5500 SPI Ethernet driver).

## Features

This example project demonstrates the following features:

### Ethernet Features

| Example File | Description |
|---------|------|
| `basic-setup.js` | W5500 basic initialization, DHCP IP acquisition |
| `static-ip.js` | Configure static IP address |
| `http-client.js` | Send HTTP requests via Ethernet |
| `http-server.js` | Create HTTP server on Ethernet |
| `mqtt-client.js` | Connect to MQTT Broker via Ethernet |
| `tcp-client.js` | Raw TCP socket communication |
| `ping-test.js` | Network connectivity test |

## Project Structure

```
beshell-eth-app/
├── CMakeLists.txt          # Project CMake configuration
├── sdkconfig.defaults      # Default SDK configuration
├── README.md               # This file
├── main/
│   ├── main.cpp            # C++ entry file
│   ├── CMakeLists.txt      # main component CMake config
│   └── idf_component.yml   # Component dependencies
├── img/
│   ├── partitions-4MB.csv  # 4MB Flash partition table
│   ├── partitions-8MB.csv  # 8MB Flash partition table
│   └── partitions-16MB.csv # 16MB Flash partition table
└── js/
    ├── main.js             # JS entry file
    └── example/            # Example scripts directory
        ├── basic-setup.js
        ├── static-ip.js
        ├── http-client.js
        ├── http-server.js
        ├── mqtt-client.js
        ├── tcp-client.js
        └── ping-test.js
```

## Hardware Requirements

- ESP32 / ESP32-S3 / ESP32-C3 or other chips
- W5500 SPI Ethernet module
- At least 4MB Flash
- USB-to-Serial for flashing and debugging

### Hardware Connection (W5500)

```
ESP32          W5500
-----          -----
GPIO 5    -->  CS
GPIO 18   -->  SCLK
GPIO 19   -->  MISO
GPIO 23   -->  MOSI
GPIO 4    -->  RST (optional)
GPIO 34   -->  INT (optional)
3.3V      -->  VCC
GND       -->  GND
```

**Note**: SPI pins may vary depending on ESP32 model, adjust according to your hardware.

## Quick Start

### 1. Modify SPI Configuration

Modify the configuration in examples according to your hardware:

```javascript
const SPI_HOST = 2    // SPI2 (HSPI)
const PIN_CS = 5      // CS pin
```

### 2. Build the Project

```bash
idf.py build
```

### 3. Flash the Firmware

```bash
idf.py flash
```

### 4. View Serial Output

```bash
idf.py monitor
```

### 5. Run Examples

In the serial terminal, enter the following command to run an example:

```
run /example/basic-setup.js
```

## Example Details

### Basic Setup (DHCP)

```javascript
import { W5500 } from "w5500"

let eth = new W5500()

// Listen for network events
eth.on("connected", () => {
    console.log("Ethernet link connected")
})

eth.on("ip.got", (info) => {
    console.log("IP:", info.ip)
    console.log("Netmask:", info.netmask)
    console.log("Gateway:", info.gw)
})

// Initialize W5500
eth.setup({
    spi: 2,           // SPI host
    cs: 5,            // CS pin
    rst: -1,          // RST pin (-1 for not used)
    intr: -1,         // INT pin (-1 for not used)
    clock_mhz: 16,    // SPI clock frequency
    polling_ms: 0,    // 0 for interrupt mode
    phy_addr: 1       // PHY address
})
```

### Static IP Configuration

```javascript
// Wait for connection
await new Promise((resolve) => {
    eth.once("connected", resolve)
})

// Set static IP
eth.setIP("192.168.1.100", "255.255.255.0", "192.168.1.1")
```

### HTTP Client

```javascript
import * as mg from "mg"

// After Ethernet is ready
let body = await mg.get("http://api.example.com/data")
console.log(body.toString())
```

### HTTP Server

```javascript
mg.listenHttp("0.0.0.0:8080", (event, req, rspn) => {
    if (event === "http.msg") {
        rspn.setStatus(200)
        rspn.setHeader("Content-Type", "text/html")
        rspn.send("<h1>Hello from W5500!</h1>")
    }
})
```

## API Reference

### W5500 Class

#### constructor()
Create a W5500 object.

#### setup(config)
Initialize the W5500 module.
- `config.spi`: SPI host number (0-2)
- `config.cs`: CS pin number
- `config.rst`: RST pin number (-1 for not used)
- `config.intr`: INT pin number (-1 for not used)
- `config.clock_mhz`: SPI clock frequency (MHz)
- `config.polling_ms`: Polling interval (0 for interrupt mode)
- `config.phy_addr`: PHY address

#### setIP(ip, netmask, gateway)
Set static IP address.

#### getMAC()
Get MAC address.

### Events

- `start`: Ethernet started
- `connected`: Ethernet link connected
- `disconnected`: Ethernet link disconnected
- `stop`: Ethernet stopped
- `ip.got`: IP address obtained
- `ip.lost`: IP address lost

## Dependencies

- [beshell](https://github.com/become-cool/beshell) - BeShell core framework
- [beshell-drv-eth](https://github.com/become-cool/beshell-drv-eth) - W5500 Ethernet driver
- [beshell-mg](https://github.com/become-cool/beshell-mg) - Network component

## License

LGPL

## Links

- [BeShell Documentation](https://beshell.become.cool)
- [W5500 Datasheet](https://www.wiznet.io/wp-content/uploads/wiznethome/Chip/W5500/Documents/W5500_Datasheet_v1.0.2_1.pdf)
- [ESP-IDF Ethernet Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_eth.html)
