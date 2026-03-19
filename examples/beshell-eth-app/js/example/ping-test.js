import { W5500 } from "w5500"
import * as mg from "mg"

// W5500 配置
const SPI_HOST = 2
const PIN_CS = 5

// 测试目标
const PING_TARGET = "8.8.8.8"  // Google DNS

async function main() {
    console.log("\n=== Ethernet Network Test ===\n")
    
    let eth = new W5500()
    let ipAddress = null
    
    // 监听事件
    eth.on("connected", () => {
        console.log("✓ Ethernet link connected")
    })
    
    eth.on("ip.got", async (info) => {
        ipAddress = info.ip
        console.log("✓ Got IP address:", ipAddress)
        console.log(`  Netmask: ${info.netmask}`)
        console.log(`  Gateway: ${info.gw}`)
        console.log("")
        
        // 运行网络测试
        await runNetworkTests()
    })
    
    // 初始化 W5500
    console.log("Initializing W5500...")
    console.log(`  SPI Host: ${SPI_HOST}`)
    console.log(`  CS Pin: ${PIN_CS}`)
    console.log("")
    
    eth.setup({
        spi: SPI_HOST,
        cs: PIN_CS,
        clock_mhz: 16
    })
    
    console.log("Waiting for network connection...")
}

async function runNetworkTests() {
    console.log("=== Running Network Tests ===\n")
    
    // Test 1: DNS Resolution
    console.log("Test 1: DNS Resolution")
    console.log("Current DNS server:", mg.getDNS())
    
    try {
        let urlInfo = mg.parseUrl("http://www.google.com")
        console.log("Parsed www.google.com:", JSON.stringify(urlInfo))
    } catch (e) {
        console.log("DNS test failed:", e.message)
    }
    console.log("")
    
    // Test 2: HTTP GET
    console.log("Test 2: HTTP GET via Ethernet")
    try {
        let startTime = Date.now()
        let body = await mg.get("http://httpbin.org/ip")
        let duration = Date.now() - startTime
        
        console.log(`✓ HTTP request completed in ${duration}ms`)
        console.log("Response:", body.toString())
    } catch (e) {
        console.log("✗ HTTP test failed:", e.message)
    }
    console.log("")
    
    // Test 3: SNTP Time Sync
    console.log("Test 3: SNTP Time Synchronization")
    try {
        let timestamp = await new Promise((resolve, reject) => {
            mg.sntpRequest("udp://pool.ntp.org:123", (err, time) => {
                if (err) reject(err)
                else resolve(time)
            })
        })
        
        let date = new Date(timestamp)
        console.log("✓ SNTP sync successful")
        console.log("Current time:", date.toISOString())
    } catch (e) {
        console.log("✗ SNTP test failed:", e.message)
    }
    console.log("")
    
    console.log("=== All tests completed ===")
}

main()
