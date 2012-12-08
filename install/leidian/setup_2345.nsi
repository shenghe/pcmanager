

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define PRODUCT_PARTNER  "O_2345"                                      ;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_2345.exe"          ;生成安装包的名字
!define MACRO_CFG_PATH  "cfg_enable_mon_skylark"
!define WRITE_TRAY_REG_RUN 1
!define LOCK_HOMEPAGE_TEXT "设定安全导航站2345.com为主页，轻松上网"
!define LOCK_HOMEPAGE_URL "http://www.2345.com/?7686"


!include "leidian_install.nsh"

