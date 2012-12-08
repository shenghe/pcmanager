

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define PRODUCT_PARTNER  "h_update"                      ;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_update.exe"       ;生成安装包的名字
!define PRODUCT_UPDATE   "1"                           ;升级包
!define PRODUCT_WITH_KENG "1"
!define WRITE_TRAY_REG_RUN      1      ;是否写tray的启动项
!include "leidian_install.nsh"

