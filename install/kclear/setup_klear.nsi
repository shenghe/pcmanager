
!system "if not exist output md output"

!include "auto_version.nsh"
!define PRODUCT_OUT_FILE      "..\output\setup_kclear.exe"          ;生成安装包的名字
!include "kclear_install.nsh"

