import { W5500 } from "w5500"
import * as mg from "mg"

// W5500 配置
const SPI_HOST = 2
const PIN_CS = 5

// TCP 服务器配置
const TCP_SERVER = "tcp://tcpbin.com:4242"

async function main() {
    console.log("\n=== Ethernet TCP Client Example ===\n")
    
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
        
        // 连接 TCP 服务器
        await connectTCP()
    })
    
    // 初始化 W5500
    console.log("Initializing W5500...")
    eth.setup({
        spi: SPI_HOST,
        cs: PIN_CS,
        clock_mhz: 16
    })
    
    console.log("Waiting for network connection...")
    while (!connected) {
        await sleep(100)
    }
}

async function connectTCP() {
    console.log("Connecting to TCP server...")
    console.log(`Server: ${TCP_SERVER}`)
    console.log("")
    
    let messageCount = 0
    
    try {
        let conn = mg.connect(TCP_SERVER, (event, data) => {
            console.log("TCP Event:", event)
            
            if (event === "connect") {
                console.log("✓ Connected to TCP server!")
                console.log("")
                
                // 发送欢迎消息
                let welcome = "Hello from ESP32-W5500!\n"
                conn.send(welcome)
                console.log("Sent:", welcome.trim())
                
                // 定期发送消息
                setInterval(() => {
                    messageCount++
                    let msg = `Message #${messageCount} from W5500 at ${Date.now()}\n`
                    try {
                        conn.send(msg)
                        console.log("Sent:", msg.trim())
                    } catch (e) {
                        console.log("Send failed:", e.message)
                    }
                }, 3000)
            }
            
            else if (event === "read") {
                let received = data.body()
                console.log("Received:", received.toString().trim())
            }
            
            else if (event === "error") {
                console.log("✗ TCP Error:", data)
            }
            
            else if (event === "close") {
                console.log("✗ TCP connection closed")
            }
        })
        
        console.log("TCP client is running...")
        console.log("Press Ctrl+C to stop\n")
        
    } catch (e) {
        console.log("Failed to connect:", e.message)
    }
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}

main()
