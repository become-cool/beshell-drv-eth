import { W5500 } from "w5500"
import * as mg from "mg"

// W5500 配置
const SPI_HOST = 2
const PIN_CS = 5

// 服务器配置
const SERVER_PORT = 8080

async function main() {
    console.log("\n=== Ethernet HTTP Server Example ===\n")
    
    let eth = new W5500()
    let ipAddress = null
    
    // 监听事件
    eth.on("connected", () => {
        console.log("✓ Ethernet link connected")
    })
    
    eth.on("ip.got", async (info) => {
        ipAddress = info.ip
        console.log("✓ Got IP address:", ipAddress)
        console.log("")
        
        // 启动 HTTP 服务器
        startHttpServer(ipAddress)
    })
    
    // 初始化 W5500
    console.log("Initializing W5500...")
    eth.setup({
        spi: SPI_HOST,
        cs: PIN_CS,
        clock_mhz: 16
    })
    
    console.log("Waiting for network connection...")
}

function startHttpServer(ip) {
    console.log("Starting HTTP server...")
    console.log(`Server will run at http://${ip}:${SERVER_PORT}`)
    console.log("")
    
    // 请求计数器
    let requestCount = 0
    
    mg.listenHttp(`0.0.0.0:${SERVER_PORT}`, (event, req, rspn) => {
        if (event !== "http.msg") return
        
        requestCount++
        let url = req.url()
        let method = req.method()
        
        console.log(`[${requestCount}] ${method} ${url}`)
        
        // 路由处理
        if (url === "/") {
            rspn.setStatus(200)
            rspn.setHeader("Content-Type", "text/html")
            rspn.send(`
                <!DOCTYPE html>
                <html>
                <head>
                    <title>W5500 Ethernet Server</title>
                    <style>
                        body { font-family: Arial, sans-serif; margin: 40px; }
                        h1 { color: #333; }
                        .info { background: #f0f0f0; padding: 20px; border-radius: 5px; }
                    </style>
                </head>
                <body>
                    <h1>Hello from W5500 Ethernet!</h1>
                    <div class="info">
                        <p><strong>Server IP:</strong> ${ip}</p>
                        <p><strong>Port:</strong> ${SERVER_PORT}</p>
                        <p><strong>Requests handled:</strong> ${requestCount}</p>
                    </div>
                    <h2>Available endpoints:</h2>
                    <ul>
                        <li><a href="/api/status">/api/status</a> - System status</li>
                        <li><a href="/api/time">/api/time</a> - Current time</li>
                        <li><a href="/api/network">/api/network</a> - Network info</li>
                    </ul>
                </body>
                </html>
            `)
        }
        
        else if (url === "/api/status") {
            rspn.setStatus(200)
            rspn.setHeader("Content-Type", "application/json")
            rspn.send(JSON.stringify({
                status: "ok",
                uptime: Date.now(),
                requests: requestCount,
                platform: "ESP32 + W5500"
            }, null, 2))
        }
        
        else if (url === "/api/time") {
            rspn.setStatus(200)
            rspn.setHeader("Content-Type", "application/json")
            rspn.send(JSON.stringify({
                timestamp: Date.now(),
                iso: new Date().toISOString(),
                local: new Date().toLocaleString()
            }, null, 2))
        }
        
        else if (url === "/api/network") {
            rspn.setStatus(200)
            rspn.setHeader("Content-Type", "application/json")
            rspn.send(JSON.stringify({
                ip: ip,
                port: SERVER_PORT,
                dns: mg.getDNS(),
                dns_timeout: mg.getDNSTimeout()
            }, null, 2))
        }
        
        else {
            rspn.setStatus(404)
            rspn.setHeader("Content-Type", "application/json")
            rspn.send(JSON.stringify({
                error: "Not found",
                path: url
            }))
        }
    })
    
    console.log("✓ HTTP server started!")
    console.log("")
    console.log("Try accessing:")
    console.log(`  http://${ip}:${SERVER_PORT}/`)
    console.log(`  http://${ip}:${SERVER_PORT}/api/status`)
    console.log("")
    console.log("Press Ctrl+C to stop")
}

main()
