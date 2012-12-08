

!system "if exist version.inc.nsi del version.inc.nsi"
!system "getver.exe ..\..\product\win32\ksafever.dll"
!include "version.inc.nsi"

!define PRODUCT_PARTNER  "O_JXSJ"                      ;合作厂商
!define PRODUCT_OUT_FILE "..\..\product\package\setup_ojxsj_jinshanweishi.exe"          ;生成安装包的名字
!define MACRO_CFG_PATH  "cfg_enable_mon_skylark"
!define WRITE_TRAY_REG_RUN 1
!define PRODUCT_JXSJ       1

!define STR_JXSJ_INI_NAME                       "resinstall.ini"
!define STR_JXSJ_INI_SECTION_PRE                ""
!define STR_JXSJ_INI_INST_KEY                   "install"

!define STR_JXSJ_FMARK_1                        ".exe"
!define STR_JXSJ_FMARK_2                        "setup_ojxsj_"


!include "leidian_install.nsh"

