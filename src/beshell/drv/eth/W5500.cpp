#include "W5500.hpp"
#include <beshell/platform.hpp>
#include <beshell/module/gpio/GPIO.hpp>
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_eth_mac_spi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"

using namespace std ;

namespace be::driver::comm {

    /**
     * 以太网（Ethernet）驱动模块
     *
     * 提供以太网芯片的驱动类，通过 SPI 等接口将 ESP32 接入有线网络。
     *
     * 目前提供的驱动类：
     * - [W5500](W5500.html) - WIZnet W5500 SPI 以太网芯片
     *
     * 该模块会持续增加其他以太网 IC 的驱动类。
     *
     * @module eth
     * @component beshell-drv-eth
     */

    /**
     * W5500 以太网驱动类
     * 
     * 用于通过 SPI 接口控制 W5500 以太网模块，实现有线网络连接。
     * 支持动态IP（DHCP）和静态IP配置，提供网络状态事件监听。
     * 
     * W5500 通过 SPI 接口与 ESP32 通信。在使用 W5500 之前，
     * 需要先通过 serial 模块初始化对应的 SPI 总线。
     * 
     * 示例：
     * ```javascript
     * import { W5500 } from "eth"
     * import * as serial from "serial"
     * 
     * // 第一步：初始化 SPI 总线
     * const spi = serial.spi2
     * spi.setup({
     *     miso: 19,  // MISO 引脚
     *     mosi: 23,  // MOSI 引脚
     *     sck: 18    // 时钟引脚
     * })
     * 
     * // 第二步：创建 W5500 实例
     * const eth = new W5500()
     * 
     * // 第三步：监听网络事件
     * eth.on("connected", () => {
     *     console.log("以太网已连接")
     * })
     * eth.on("ip.got", (info) => {
     *     console.log("获取到IP:", info.ip)
     *     console.log("子网掩码:", info.netmask)
     *     console.log("网关:", info.gw)
     * })
     * eth.on("disconnected", () => {
     *     console.log("以太网已断开")
     * })
     * 
     * // 第四步：初始化 W5500
     * eth.setup({
     *     spi: 2,        // SPI 总线号（对应 serial.spi2）
     *     cs: 5,         // CS 引脚
     *     rst: 17,       // 复位引脚（可选）
     *     intr: 16,      // 中断引脚（可选，不使用则采用轮询模式）
     *     clock_mhz: 16, // SPI 时钟频率（MHz）
     *     polling_ms: 0, // 轮询间隔（毫秒，0表示中断模式）
     *     phy_addr: 1    // PHY 地址
     * })
     * 
     * // 设置静态 IP（可选，默认使用 DHCP）
     * // eth.setIP("192.168.1.100", "255.255.255.0", "192.168.1.1")
     * ```
     * 
     * @component beshell-drv-eth
     *
     * @class W5500
     * @module eth
     * @extends EventEmitter
     */
    DEFINE_NCLASS_META(W5500, EventEmitter)

    typedef struct {
        uint8_t type ; // 1: Ethernet events, 2: IP events
        esp_event_base_t event_base ;
        int32_t event_id ;
        // union {
        //     esp_netif_ip_info_t ip_info ;
        // } ;
    } w5500_event_t ;

    std::vector<JSCFunctionListEntry> W5500::methods = {
        JS_CFUNC_DEF("setup", 0, W5500::setup),
        JS_CFUNC_DEF("setIP", 0, W5500::setIP),
        JS_CFUNC_DEF("getMAC", 0, W5500::getMAC),
    } ;

    W5500::W5500(JSContext * ctx, JSValue _jsobj)
        : EventEmitter(ctx,build(ctx,_jsobj))
    {
        memset(&ipinfo,0,sizeof(esp_netif_ip_info_t)) ;

        enableNativeEvent(ctx, sizeof(w5500_event_t), 5) ;
    }
    
    W5500::~W5500() {
        // ds("~W5500")
        if(handlerEth) {
            esp_event_handler_instance_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, handlerEth);
            handlerEth = NULL ;
        }
        if(handlerIp) {
            esp_event_handler_instance_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, handlerIp);
            handlerIp = NULL ;
        }
    }

    /**
     * 构造函数
     * 
     * 创建一个新的 W5500 实例。
     * 
     * @component beshell-drv-eth
     *
     * @module eth
     * @class W5500
     * @method constructor
     * @return W5500 返回 W5500 实例
     */
    JSValue W5500::constructor(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        auto obj = new W5500(ctx, this_val) ;
        obj->shared() ;
        return obj->jsobj ;
    }

    /** Event handler for Ethernet events */
    void W5500::ethEventHandler(W5500 * that, esp_event_base_t event_base, int32_t event_id, void *event_data) {

        if(!that) {
            printf("W5500 instance is null\n") ;
            return ;
        }
        if( that->eth_handle != *(esp_eth_handle_t *)event_data ){
            printf("Event from unknown eth handle\n") ;
        }

        switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(that->eth_handle, ETH_CMD_G_MAC_ADDR, that->mac_addr);
            // printf("Ethernet Link Up\n");
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            // printf("Ethernet Link Down\n");
            break;
        case ETHERNET_EVENT_START:
            // printf("Ethernet Started\n");
            break;
        case ETHERNET_EVENT_STOP:
            // printf("Ethernet Stopped\n");
            break;
        default:
            // printf("event_id=%d\n",(int) event_id) ;
            break;
        }
        
        w5500_event_t msg = {
            .type = 1,
            .event_base = event_base,
            .event_id = event_id,
        } ;
        that->emitNativeEvent(&msg) ;
    }

    /** Event handler for IP_EVENT_ETH_GOT_IP */
    void W5500::gotIpEventHandler(W5500 * that, esp_event_base_t event_base, int32_t event_id, void * event_data) {

        if(!that) {
            printf("W5500 instance is null\n") ;
            return ;
        }

        w5500_event_t msg = {
            .type = 2,
            .event_base = event_base,
            .event_id = event_id,
        } ;

        if(event_id==IP_EVENT_ETH_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
            memcpy(&that->ipinfo, &event->ip_info, sizeof(esp_netif_ip_info_t));
        }
        else if ( event_id == IP_EVENT_ETH_LOST_IP ) {
            memset(&that->ipinfo,0,sizeof(esp_netif_ip_info_t)) ;
        }

        that->emitNativeEvent(&msg) ;
    }

    
    void W5500::onNativeEvent(JSContext *ctx, void * param) {
        w5500_event_t * event_wrapper = (w5500_event_t *) param ;
        char const * name = nullptr ;
        if(event_wrapper->type==1) {
            switch (event_wrapper->event_id) {
            case ETHERNET_EVENT_CONNECTED:
                name = (char*)"connected" ;
                break;
            case ETHERNET_EVENT_DISCONNECTED:
                name = (char*)"disconnected" ;
                break;
            case ETHERNET_EVENT_START:
                name = (char*)"start" ;
                break;
            case ETHERNET_EVENT_STOP:
                name = (char*)"stop" ;
                break;
            default:
                break;
            }
        }

        else if(event_wrapper->type==2) {
            switch (event_wrapper->event_id) {
            case IP_EVENT_ETH_GOT_IP: {
                char ip_str[16];
                JSValue jsipinfo = JS_NewObject(ctx) ;
                
                sprintf(ip_str, IPSTR, IP2STR(&ipinfo.ip));
                JS_SetPropertyStr(ctx, jsipinfo, "ip", JS_NewString(ctx, ip_str)) ;

                sprintf(ip_str, IPSTR, IP2STR(&ipinfo.netmask));
                JS_SetPropertyStr(ctx, jsipinfo, "netmask", JS_NewString(ctx, ip_str)) ;

                sprintf(ip_str, IPSTR, IP2STR(&ipinfo.gw));
                JS_SetPropertyStr(ctx, jsipinfo, "gw", JS_NewString(ctx, ip_str)) ;

                emitSyncFree("ip.got", {jsipinfo}) ;
                return ;
            }
            case IP_EVENT_ETH_LOST_IP:
                name = "ip.lost" ;
                break;
            default:
                break;
            }
        }

        if(!name){
            printf("unknow event: %d\n", event_wrapper->event_id) ;
            return ;
        }
        
        emitSyncFree(name, {}) ;
    }

    /**
     * 初始化 W5500 以太网模块
     * 
     * 配置并启动 W5500 以太网模块，建立网络连接。
     * 
     * **注意**：在调用此方法前，必须先通过 serial 模块初始化对应的 SPI 总线。
     * 
     * 配置参数说明：
     * - spi: SPI 总线号（如 2 表示 SPI2，对应 serial.spi2）
     * - cs: CS（片选）引脚号
     * - rst: 复位引脚号（可选，默认 -1 表示不使用）
     * - intr: 中断引脚号（可选，默认 -1 表示不使用轮询模式）
     * - clock_mhz: SPI 时钟频率（MHz，默认 16）
     * - polling_ms: 轮询间隔（毫秒，默认 0 表示使用中断模式）
     * - phy_addr: PHY 地址（默认 1）
     * 
     * 示例：
     * ```javascript
     * import { W5500 } from "eth"
     * import * as serial from "serial"
     * 
     * // 先初始化 SPI 总线
     * const spi = serial.spi2
     * spi.setup({
     *     miso: 19,
     *     mosi: 23,
     *     sck: 18
     * })
     * 
     * // 然后初始化 W5500
     * const eth = new W5500()
     * 
     * // 基本配置
     * eth.setup({
     *     spi: 2,   // 使用 SPI2（对应 serial.spi2）
     *     cs: 5     // CS 引脚
     * })
     * 
     * // 完整配置（使用中断模式）
     * eth.setup({
     *     spi: 2,         // SPI 总线号（对应 serial.spi2）
     *     cs: 5,          // CS 引脚
     *     rst: 17,        // 复位引脚
     *     intr: 16,       // 中断引脚（使用中断模式时设置）
     *     clock_mhz: 16,  // SPI 时钟频率
     *     polling_ms: 0,  // 0 表示使用中断模式
     *     phy_addr: 1     // PHY 地址
     * })
     * 
     * // 使用轮询模式（不设置 intr 或设置 polling_ms > 0）
     * eth.setup({
     *     spi: 2,
     *     cs: 5,
     *     rst: 17,
     *     polling_ms: 100  // 每 100ms 轮询一次
     * })
     * ```
     *
     * @component beshell-drv-eth
     *
     * @module eth
     * @class W5500
     * @method setup
     * @param config:object 配置对象
     *     {
     *         spi: number,           // SPI 总线号
     *         cs: number,            // CS 引脚号
     *         rst?: number,          // 复位引脚号，默认 -1
     *         intr?: number,         // 中断引脚号，默认 -1
     *         clock_mhz?: number,    // SPI 时钟频率（MHz），默认 16
     *         polling_ms?: number,   // 轮询间隔（毫秒），默认 0，0 表示中断模式
     *         phy_addr?: number      // PHY 地址，默认 1
     *     }
     * @return undefined
     * @throws SPI 以太网驱动未启用
     * @throws 安装 GPIO 中断失败
     * @throws SPI 以太网驱动安装失败
     * @throws SPI 以太网驱动连接到网络接口失败
     * @throws SPI 以太网驱动启动失败
     */
    JSValue W5500::setup(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
#if CONFIG_ETH_SPI_ETHERNET_W5500
        ASSERT_ARGC(1)
        THIS_NCLASS(W5500, that)

        GET_INT_PROP(argv[0], "spi", that->spi_num, spi_host_device_t, )
        GET_GPIO_PROP_OPT(argv[0], "cs", that->gpio_cs, GPIO_NUM_NC)
        GET_GPIO_PROP_OPT(argv[0], "rst", that->gpio_rst, GPIO_NUM_NC)
        GET_GPIO_PROP_OPT(argv[0], "intr", that->gpio_intr, GPIO_NUM_NC)
        GET_INT32_PROP_OPT(argv[0], "clock_mhz", that->clock_mhz, 16)
        GET_UINT32_PROP_OPT(argv[0], "polling_ms", that->polling_ms, 0)
        GET_UINT8_PROP_OPT(argv[0], "phy_addr", that->phy_addr, 1)

        // Init common MAC and PHY configs to default
        eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
        eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

        be::platform::networkInit() ;

        if(!GPIO::installISR(0)){
            JSTHROW("install gpio isr failed")
        }

        // Configure SPI interface for specific SPI module
        spi_device_interface_config_t spi_devcfg = {
            .mode = 0,
            .clock_speed_hz = that->clock_mhz * 1000 * 1000,
            .spics_io_num = that->gpio_cs,
            .queue_size = 20,
        };

        eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(that->spi_num, &spi_devcfg);
        w5500_config.int_gpio_num = that->gpio_intr;
        w5500_config.poll_period_ms = that->polling_ms;
        esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
        esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
    
        // Init Ethernet driver to default and install it
        esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac, phy);

        // cc:1b:e0:e3:c4:58
        // uint8_t custom_mac2[6] = {0xcc,0x1b,0xe0,0xe3,0xc4,0x58};
        // uint8_t custom_mac2[6] = {0xcc,0x1b,0xe0,0xe3,0xc3,0xb8};
        // mac->set_addr(mac, custom_mac2);

        if(esp_eth_driver_install(&eth_config_spi, &that->eth_handle) != ESP_OK){
            JSTHROW("SPI Ethernet driver install failed")
        }

        // // 设置 mac 地址
        // uint8_t custom_mac[7] = {0xcc,0x1b,0xe0,0xe3,0xc3,0xb8,0};
        // JSValue jsmac = JS_GetPropertyStr(ctx, argv[0], "mac") ;
        // if( !JS_IsUndefined(jsmac) ) {
        //     if( JS_IsString(jsmac) ) {
        //         size_t c_str_mac_len = 0 ;
        //         const char * c_str_mac = JS_ToCStringLen(ctx, &c_str_mac_len, jsmac) ;
        //         if(c_str_mac && c_str_mac_len==17) {
        //             if (sscanf(c_str_mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
        //                 &custom_mac[0], &custom_mac[1], &custom_mac[2], 
        //                 &custom_mac[3], &custom_mac[4], &custom_mac[5]) == 6)
        //             {
        //                 // printf("input: %02x:%02x:%02x:%02x:%02x:%02x\n",custom_mac[0],custom_mac[1],custom_mac[2],custom_mac[3],custom_mac[4],custom_mac[5]) ;
        //                 mac->set_addr(mac, custom_mac);
        //                 // mac->set_addr(mac, custom_mac2);

        //                 uint8_t custom_mac_read[6] = {0};
        //                 mac->get_addr(mac, custom_mac_read);
        //                 printf("%02x:%02x:%02x:%02x:%02x:%02x\n", custom_mac_read[0], custom_mac_read[1], custom_mac_read[2], custom_mac_read[3], custom_mac_read[4], custom_mac_read[5]) ;
        //             }
        //             JS_FreeCString(ctx, c_str_mac) ;
        //         }
        //     }
        // }
        // else {
        //     uint8_t base_mac_addr[ETH_ADDR_LEN];
        //     esp_efuse_mac_get_default(base_mac_addr) ;
        //     esp_derive_local_mac(that->mac_addr, base_mac_addr);
        //     // printf("Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x\n", that->mac_addr[0], that->mac_addr[1], that->mac_addr[2], that->mac_addr[3], that->mac_addr[4], that->mac_addr[5]);
        //     if(esp_eth_ioctl(that->eth_handle, ETH_CMD_S_MAC_ADDR, that->mac_addr) != ESP_OK) {
        //         printf("SPI Ethernet driver set MAC address failed\n") ;
        //     }
        // }
        // JS_FreeValue(ctx, jsmac) ;

        
            uint8_t base_mac_addr[ETH_ADDR_LEN];
            esp_efuse_mac_get_default(base_mac_addr) ;
            esp_derive_local_mac(that->mac_addr, base_mac_addr);
            // printf("Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x\n", that->mac_addr[0], that->mac_addr[1], that->mac_addr[2], that->mac_addr[3], that->mac_addr[4], that->mac_addr[5]);
            if(esp_eth_ioctl(that->eth_handle, ETH_CMD_S_MAC_ADDR, that->mac_addr) != ESP_OK) {
                printf("SPI Ethernet driver set MAC address failed\n") ;
            }


        uint8_t custom_mac_read[6] = {0};
        mac->get_addr(mac, custom_mac_read);
        printf("w5500 mac: %02x:%02x:%02x:%02x:%02x:%02x\n", custom_mac_read[0], custom_mac_read[1], custom_mac_read[2], custom_mac_read[3], custom_mac_read[4], custom_mac_read[5]) ;

        // 注册网络接口
        // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
        // default esp-netif configuration parameters.
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
        that->eth_netif = esp_netif_new(&cfg);
        that->eth_netif_glue = esp_eth_new_netif_glue(that->eth_handle);
        // Attach Ethernet driver to TCP/IP stack
        if(esp_netif_attach(that->eth_netif, that->eth_netif_glue)!=ESP_OK) {
            JSTHROW("SPI Ethernet driver attach to netif failed")
        }
    
        // Register user defined event handers
        esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, (esp_event_handler_t)&ethEventHandler, (void*)that, &that->handlerEth);
        esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, (esp_event_handler_t)&gotIpEventHandler, (void*)that, &that->handlerIp);

        // 需要在这里增加一个引用表示 esp native 事件监听，否则 w5500 js 对象被释放以后，esp event handle 里会传入 that 的野指针
        // unsetup 以后在 free 这个引用， 同时 unregister esp handler
        JS_DupValue(ctx, that->jsobj) ;
    
        // Start Ethernet driver state machine
        if(esp_eth_start(that->eth_handle)!=ESP_OK){
            JSTHROW("SPI Ethernet driver start failed")
        }

        return JS_UNDEFINED ;
#else
        JSTHROW("SPI Ethernet driver not enabled, please set \"CONFIG_ETH_USE_SPI_ETHERNET=y\" in sdkconfig")
#endif
    }
    
    /**
     * 获取 MAC 地址
     * 
     * 获取 W5500 以太网模块的 MAC 地址。
     * 
     * @component beshell-drv-eth
     *
     * @module eth
     * @class W5500
     * @method getMAC
     * @return string 返回 MAC 地址字符串（格式：xx:xx:xx:xx:xx:xx）
     */
    JSValue W5500::getMAC(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        return JS_UNDEFINED ;
    }

    /**
     * 设置静态 IP 地址
     * 
     * 配置静态 IP 地址、子网掩码和网关。调用此方法会停止 DHCP 客户端。
     * 
     * 示例：
     * ```javascript
     * // 设置静态 IP
     * eth.setIP("192.168.1.100", "255.255.255.0", "192.168.1.1")
     * ```
     *
     * @component beshell-drv-eth
     *
     * @module eth
     * @class W5500
     * @method setIP
     * @param ip:string IP 地址（如 "192.168.1.100"）
     * @param netmask:string 子网掩码（如 "255.255.255.0"）
     * @param gw:string 网关地址（如 "192.168.1.1"）
     * @return undefined
     * @throws esp_netif_set_ip_info() 失败
     */
    JSValue W5500::setIP(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        ASSERT_ARGC(3)
        THIS_NCLASS(W5500, that)

        std::string ARGV_TO_STRING(0, ip)
        std::string ARGV_TO_STRING(1, netmask)
        std::string ARGV_TO_STRING(2, gw)

        esp_netif_ip_info_t ip_info;
        memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
        ip_info.ip.addr = ipaddr_addr(ip.c_str());
        ip_info.netmask.addr = ipaddr_addr(netmask.c_str());
        ip_info.gw.addr = ipaddr_addr(gw.c_str());

        esp_netif_dhcpc_stop(that->eth_netif); // 必须先停止DHCP客户端
        esp_err_t err = esp_netif_set_ip_info(that->eth_netif, &ip_info) ;
        if(err != ESP_OK) {
            JSTHROW("esp_netif_set_ip_info() failed: %d", err)
        }

        return JS_UNDEFINED ;
    }
}