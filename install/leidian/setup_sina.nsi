

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define PRODUCT_PARTNER  "O_OSINA"                      ;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_sina.exe"          ;生成安装包的名字

!include "leidian_install.nsh"

