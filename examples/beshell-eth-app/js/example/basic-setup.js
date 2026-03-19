import { W5500 } from "w5500"

// W5500 配置
const SPI_HOST = 2        // SPI2 (HSPI)
const PIN_CS = 5          // CS 引脚
const PIN_RST = -1        // RST 引脚 (-1 表示不使用)
const PIN_INTR = -1       // INT 引脚 (-1 表示不使用)

async function main() {
    console.log("\n=== W5500 Ethernet Basic Setup ===\n")
    
    // 创建 W5500 对象
    let eth = new W5500()
    
    // 监听事件
    eth.on("start", () => {
        console.log("✓ Ethernet started")
    })
    
    eth.on("connected", () => {
        console.log("✓ Ethernet link connected")
    })
    
    eth.on("disconnected", () => {
        console.log("✗ Ethernet link disconnected")
    })
    
    eth.on("ip.got", (info) => {
        console.log("✓ Got IP address:")
        console.log(`  IP:      ${info.ip}`)
        console.log(`  Netmask: ${info.netmask}`)
        console.log(`  Gateway: ${info.gw}`)
        console.log("")
        console.log("Ethernet is ready!")
    })
    
    eth.on("ip.lost", () => {
        console.log("✗ IP address lost")
    })
    
    // 初始化 W5500
    console.log("Initializing W5500...")
    console.log(`  SPI Host: ${SPI_HOST}`)
    console.log(`  CS Pin: ${PIN_CS}`)
    console.log("")
    
    try {
        eth.setup({
            spi: SPI_HOST,
            cs: PIN_CS,
            rst: PIN_RST,
            intr: PIN_INTR,
            clock_mhz: 16,      // SPI 时钟频率 (MHz)
            polling_ms: 0,      // 0 表示使用中断模式
            phy_addr: 1         // PHY 地址
        })
        
        console.log("W5500 initialization command sent.")
        console.log("Waiting for network connection...")
        console.log("")
        console.log("Press Ctrl+C to stop\n")
        
    } catch (e) {
        console.log("Failed to initialize W5500:", e.message)
    }
}

main()
