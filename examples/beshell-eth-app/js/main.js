import * as fs from "fs"

let examples = fs.listDirSync("/example")
  .sort((a,b)=>a.localeCompare(b))
  .reduce((lst,filename)=>{
      return lst + `    run /example/${filename}\n`
  },'')


console.log(`
  ============================================
   BeShell-Ethernet (W5500) Example App
  ============================================

  This is a demo for W5500 Ethernet features:
  
  * Basic Setup     - Initialize W5500 with DHCP
  * Static IP       - Configure static IP address
  * HTTP Client     - Make HTTP requests via Ethernet
  * HTTP Server     - Create HTTP server on Ethernet
  * MQTT Client     - Connect to MQTT via Ethernet
  * TCP Client      - Raw TCP socket communication

  Available Examples:
  ${examples}
  
  Commands:
  * Enter \`ls /example\` to list all examples
  * Enter \`run <full example path>\` to run example
  * Enter \`reboot\` to restart
  * Enter \`help\` or \`?\` to list all commands
  * Enter JavaScript code to run in interactive mode
`)

console.log(`
  ============================================
   BeShell-Ethernet (W5500) 示例程序
  ============================================

  本示例演示 W5500 以太网功能：
  
  * 基础设置    - 使用 DHCP 初始化 W5500
  * 静态 IP     - 配置静态 IP 地址
  * HTTP 客户端 - 通过以太网发送 HTTP 请求
  * HTTP 服务器 - 在以太网上创建 HTTP 服务器
  * MQTT 客户端 - 通过以太网连接 MQTT
  * TCP 客户端  - 原始 TCP 套接字通信

  可用示例：
  ${examples}
  
  命令：
  * 输入 \`ls /example\` 列出所有示例
  * 输入 \`run <完整路径>\` 运行示例
  * 输入 \`reboot\` 重启
  * 输入 \`help\` 或 \`?\` 列出所有命令
  * 输入 JavaScript 代码进入交互模式
`)
