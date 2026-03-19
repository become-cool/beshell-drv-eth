import { W5500 } from "w5500"

// W5500 配置
const SPI_HOST = 2
const PIN_CS = 5

// 静态 IP 配置
const STATIC_IP = "192.168.1.100"
const NETMASK = "255.255.255.0"
const GATEWAY = "192.168.1.1"

async function main() {
    console.log("\n=== W5500 Static IP Configuration ===\n")
    
    let eth = new W5500()
    
    // 监听事件
    eth.on("connected", () => {
        console.log("✓ Ethernet link connected")
    })
    
    eth.on("ip.got", (info) => {
        console.log("✓ IP configured:")
        console.log(`  IP:      ${info.ip}`)
        console.log(`  Netmask: ${info.netmask}`)
        console.log(`  Gateway: ${info.gw}`)
        console.log("")
        console.log("Note: This is the DHCP-assigned IP before we set static IP")
    })
    
    // 初始化 W5500
    console.log("Initializing W5500...")
    eth.setup({
        spi: SPI_HOST,
        cs: PIN_CS,
        clock_mhz: 16
    })
    
    console.log("Waiting for link connection...")
    
    // 等待连接建立
    await new Promise((resolve) => {
        eth.once("connected", resolve)
    })
    
    // 等待一下让 DHCP 完成（如果有）
    await sleep(2000)
    
    // 设置静态 IP
    console.log("")
    console.log("Setting static IP configuration...")
    console.log(`  IP:      ${STATIC_IP}`)
    console.log(`  Netmask: ${NETMASK}`)
    console.log(`  Gateway: ${GATEWAY}`)
    
    try {
        eth.setIP(STATIC_IP, NETMASK, GATEWAY)
        console.log("✓ Static IP configured successfully!")
        console.log("")
        console.log("The device now uses static IP address.")
        console.log("You can access it at:", STATIC_IP)
        
    } catch (e) {
        console.log("✗ Failed to set static IP:", e.message)
    }
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}

main()
