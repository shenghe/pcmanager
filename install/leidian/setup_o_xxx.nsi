

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define USE_NAME_AS_PARTER          1
!define PRODUCT_OUT_FILE "..\..\product\package\setup_o_pid.exe"          ;生成安装包的名字
!include "leidian_install.nsh"

