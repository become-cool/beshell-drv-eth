#include <stdio.h>
#include <beshell/BeShell.hpp>
#include <beshell-drv-eth/W5500.hpp>

using namespace std ;
using namespace be ;
using namespace be::driver::comm ;


#ifdef __cplusplus
extern "C" {
#endif

void app_main(void)
{
    BeShell beshell;

    // 启用 BeShell 模块
    beshell.use<FS>() ;
    beshell.use<Serial>() ;
    beshell.use<NVS>() ;
    
    // 启用 W5500 以太网驱动模块
    beshell.use<W5500>() ;

    // 挂载 js 分区到文件的根目录
    FS::mount("/", new LittleFS("js", true)) ;

    // 启动 BeShell
    beshell.main("/main.js");
}

#ifdef __cplusplus
}
#endif
