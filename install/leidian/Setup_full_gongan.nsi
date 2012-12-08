

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define PRODUCT_PARTNER "O_TJSJ"                      ;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_ksafe_full.exe"          ;
!define PRODUCT_WITH_KENG                           1生成安装包的名字
!define MACRO_CFG_PATH  "cfg_enable_mon_skylark"


!include "leidian_install.nsh"

