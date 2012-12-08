; 该脚本使用 HM VNISEdit 脚本编辑器向导产生

!include "LogicLib.nsh"
!include "FileFunc.nsh"

; 安装程序初始定义常量
!define PRODUCT_NAME "金山卫士V10引擎"
!define PRODUCT_PUBLISHER "金山卫士V10引擎"
!define PRODUCT_WEB_SITE "http://www.ijinshan.com"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_VERSION "1.1版"
!define PRODUCT_ENG_MARK  "SOFTWARE\KSafe\KEng"
!define PRODUCT_ENG_MARK_VALUE  "Install"       ; 安装标记名从 Enable 被修改为 Install

!define FILE_VER     		${AUTO_FILE_VERSION}

SetCompressor /SOLID lzma

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"


!define REGKEY_PRODUCT         "Software\KSafe"
!define REGVAL_INSTALL         "Install Path"         ; 记的修改InstallDirRegKey, by bbcallen

;//Our nsh
!addincludedir "..\common\"
!addplugindir "..\common\"

!include "defines.nsh"
!include "MyFileEx.nsh"
!include "MyUpgradeDLL.nsh"
!include "Http.nsh"
!include "commfunc.nsh"
!include "StrFunc.nsh"
!include "cmdLine.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "res\Install.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "res\InstallWelCome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "res\InstallWelCome.bmp"
!define MUI_UNICON "res\uninstall.ico"

; 安装程序属性设置
InstallDirRegKey HKLM "SOFTWARE\KSafe" "Install Path"
; 欢迎页面
!insertmacro MUI_PAGE_WELCOME

; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES

; 安装完成页面
!insertmacro MUI_PAGE_FINISH

; 卸载过程页面
!insertmacro MUI_UNPAGE_INSTFILES
; 卸载完成页面
!insertmacro MUI_UNPAGE_FINISH

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI 现代界面定义结束 ------

RequestExecutionLevel admin

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_OUT_FILE}"
InstallDir "$PROGRAMFILES\KSafe\KEng"
XPStyle on
ShowInstDetails nevershow
ShowUninstDetails nevershow
BrandingText "ijinshan.com"
Caption " ${PRODUCT_NAME} ${PRODUCT_VERSION} 安装"

!include "keng_install_mainsection.nsh"

Section -Post
  WriteUninstaller "$INSTDIR\uninstkeng.exe"   ; 卸载程序转到KEng子目录下, by bbcallen
  ; 不写卸载项
  ; WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  ; WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\KEng\uninstkeng.exe"  ; 同上
  ; WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  ; WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  ; WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

/******************************
 *  以下是安装程序的卸载部分  *
 ******************************/

Section Uninstall
  ;卸载统计
  ;push $0
  ;ReadRegStr $0 HKEY_LOCAL_MACHINE "${REGKEY_PRODUCT}\Coop" "PreOEM"
  ;!insertmacro InternetGetCookie "stat.ijinshan.com" "/uninstallkeng.htm?pid=$0&ver=${FILE_VER}"
  ;pop $0

  !insertmacro DeleteFileEx "$INSTDIR\ksgmerge.dll" "$INSTDIR"
  !insertmacro DeleteFileEx "$INSTDIR\ksignup.dll" "$INSTDIR"
  !insertmacro DeleteFileEx "$INSTDIR\ksafeave.dll" "$INSTDIR"

  Delete /REBOOTOK "$INSTDIR\uninstkeng.exe"

  SetOutPath "$TEMP"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "${PRODUCT_ENG_MARK}"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"

  SetAutoClose true
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

Function GetKSafePathByCMD

  Strcpy $0 ""
  !insertmacro GETPARAMETERVALUE "/D2="
  pop $0
  
  ;MessageBox MB_OK $0
  ${if} $0 != ""
      Strcpy $1 "$0ksafever.dll"
      IfFileExists $1 0 label_out
      Strcpy $0 $0
      goto label_out_x
  label_out:
      Strcpy $0 "0"
  ${else}
      Strcpy $0 "0"
  ${endif}

  label_out_x:

FunctionEnd

Function GetInstalledLeidianPath
  push $1

  Strcpy $0 ""
  ReadRegStr $0 HKLM "${REGKEY_PRODUCT}" "${REGVAL_INSTALL}"
  ${if} $0 != ""
      Strcpy $1 "$0\ksafever.dll"
      IfFileExists $1 0 label_out
      Strcpy $0 $0
      goto label_out_x
  label_out:
      Strcpy $0 "0"
  ${else}
      Strcpy $0 "0"
  ${endif}
  
  label_out_x:
  pop $1
FunctionEnd

Function .onInit
  push $0
  
  Call GetKSafePathByCMD
  ${if} $0 == "0"
      Call GetInstalledLeidianPath
      ${if} $0 == "0"
          MessageBox MB_OK "您机器上没有安装金山卫士，请先安装金山卫士!"
          quit
      ${endif}
  ${endif}
  StrCpy $INSTDIR "$0\KEng"
  pop $0
FunctionEnd

Function .onInstSuccess
FunctionEnd

Function un.onInit
FunctionEnd

Function un.onUninstSuccess
FunctionEnd

VIAddVersionKey  "ProductName" "金山卫士V10引擎"
VIAddVersionKey  "Comments" "金山卫士V10引擎"
VIAddVersionKey  "CompanyName" "金山卫士"
VIAddVersionKey  "LegalTrademarks" "金山卫士"
VIAddVersionKey  "LegalCopyright" "版权所有(C) 2010 金山卫士"
VIAddVersionKey  "FileDescription" "金山卫士V10引擎"
VIAddVersionKey  "FileVersion" "${FILE_VER}"
VIAddVersionKey  "ProductVersion" "${FILE_VER}"
VIProductVersion "${FILE_VER}"
