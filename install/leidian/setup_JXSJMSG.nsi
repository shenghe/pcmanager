

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

;剑侠情缘的每周弹框提醒包

!define PRODUCT_PARTNER "O_JXSJMSG"                      				;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_JXSJMSG.exe"        ;生成安装包的名字
!define MACRO_CFG_PATH  "cfg"							;配置目录的名字
!define PRODUCT_WITH_KENG                           1
!define WRITE_TRAY_REG_RUN      1      ;是否写tray的启动项
!define JXSJ_MSG_TIP_TXT        "剑侠情缘三  推荐您安装金山卫士$\r$\n$\r$\n游戏玩家首选安全软件，杀木马、防盗号、保护您的    $\r$\n游戏账号和财产安全，永久免费。确定安装吗？"

!include "leidian_install.nsh"

