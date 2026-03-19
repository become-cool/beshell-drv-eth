import { W5500 } from "w5500"
import * as mg from "mg"

// W5500 配置
const SPI_HOST = 2
const PIN_CS = 5

// MQTT 配置
const MQTT_BROKER = "mqtt://broker.emqx.io:1883"
const MQTT_TOPIC = "beshell-eth/demo"

async function main() {
    console.log("\n=== Ethernet MQTT Client Example ===\n")
    
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
        
        // 连接 MQTT
        await connectMQTT()
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

async function connectMQTT() {
    console.log("Connecting to MQTT broker...")
    console.log(`Broker: ${MQTT_BROKER}`)
    console.log("")
    
    let client = null
    let messageCount = 0
    
    try {
        client = mg.connect(MQTT_BROKER, (event, data) => {
            console.log("MQTT Event:", event)
            
            if (event === "mqtt.open") {
                console.log("✓ Connected to MQTT broker!")
                console.log("")
                
                // 订阅主题
                console.log(`Subscribing to: ${MQTT_TOPIC}`)
                client.sub(MQTT_TOPIC)
                
                // 发布上线消息
                let onlineMsg = JSON.stringify({
                    device: "ESP32-W5500",
                    status: "online",
                    timestamp: Date.now()
                })
                client.push(MQTT_TOPIC, onlineMsg.toArrayBuffer())
                
                // 定期发布消息
                setInterval(() => {
                    messageCount++
                    let msg = JSON.stringify({
                        device: "ESP32-W5500",
                        message: `Hello #${messageCount}`,
                        timestamp: Date.now()
                    })
                    client.push(MQTT_TOPIC, msg.toArrayBuffer())
                    console.log(`Published message #${messageCount}`)
                }, 5000)
            }
            
            else if (event === "mqtt.msg") {
                let topic = data.topic()
                let payload = data.body()
                console.log(`Received on [${topic}]:`, payload.toString())
            }
            
            else if (event === "error") {
                console.log("✗ MQTT Error:", data)
            }
            
            else if (event === "close") {
                console.log("✗ MQTT connection closed")
            }
        })
        
        console.log("MQTT client is running...")
        console.log("Press Ctrl+C to stop\n")
        
    } catch (e) {
        console.log("Failed to connect to MQTT:", e.message)
    }
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}

main()
