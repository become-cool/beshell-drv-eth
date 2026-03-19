import { W5500 } from "w5500"
import * as mg from "mg"

// W5500 配置
const SPI_HOST = 2
const PIN_CS = 5

// 测试 URL
const TEST_URL = "http://httpbin.org/get"

async function main() {
    console.log("\n=== Ethernet HTTP Client Example ===\n")
    
    let eth = new W5500()
    let connected = false
    
    // 监听事件
    eth.on("connected", () => {
        console.log("✓ Ethernet link connected")
    })
    
    eth.on("ip.got", async (info) => {
        console.log("✓ Got IP address:", info.ip)
        console.log("")
        connected = true
        
        // 开始 HTTP 测试
        await runHttpTests()
    })
    
    // 初始化 W5500
    console.log("Initializing W5500...")
    eth.setup({
        spi: SPI_HOST,
        cs: PIN_CS,
        clock_mhz: 16
    })
    
    // 等待连接
    console.log("Waiting for network connection...")
    while (!connected) {
        await sleep(100)
    }
}

async function runHttpTests() {
    console.log("=== HTTP Tests via Ethernet ===\n")
    
    // Test 1: Simple GET
    console.log("Test 1: Simple GET request")
    console.log(`URL: ${TEST_URL}`)
    console.log("")
    
    try {
        let startTime = Date.now()
        let body = await mg.get(TEST_URL)
        let duration = Date.now() - startTime
        
        console.log(`✓ Request completed in ${duration}ms`)
        console.log(`Response length: ${body.byteLength} bytes`)
        console.log("")
        console.log("Response preview:")
        console.log(body.toString().substring(0, 500))
        console.log("")
        
    } catch (e) {
        console.log("✗ Request failed:", e.message)
    }
    
    // Test 2: Parse URL
    console.log("Test 2: URL parsing")
    let urlInfo = mg.parseUrl("http://example.com:8080/path/to/resource")
    console.log("Parsed URL:", JSON.stringify(urlInfo, null, 2))
    console.log("")
    
    // Test 3: DNS
    console.log("Test 3: DNS configuration")
    console.log("Current DNS:", mg.getDNS())
    console.log("DNS Timeout:", mg.getDNSTimeout(), "ms")
    console.log("")
    
    console.log("=== All tests completed ===")
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}

main()
