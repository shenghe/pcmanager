

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define PRODUCT_PARTNER "h_home"                      				;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_${AUTO_FILE_VERSION}.exe"        ;生成安装包的名字
!define MACRO_CFG_PATH  "cfg"							;配置目录的名字
!define PRODUCT_WITH_KENG                           1
!define WRITE_TRAY_REG_RUN      1      ;是否写tray的启动项
;!define FORCE_SILENT_INST       1      ;是否强制静默安装

!include "leidian_install.nsh"

