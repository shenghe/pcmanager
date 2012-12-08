
Section "MainSection" SEC01

  ;关掉当前运行的
  KillProcDLL::KillProc "KSafe.exe"
  !insertmacro KillLeidianTray
; 卸载以前的服务
  push $0
  push $1
  Call GetInstalledLeidianPath
  ${if} $0 != "0"
       Strcpy $1 "$0\KSafeSvc.exe"
       IfFileExists $1 0 +2
        KillProcDLL::KillProc "KSafeSvc.exe"
        ExecWait "$1 /uninstall"
  ${endif}
  pop $1
  pop $0

  KillProcDLL::KillProc "KSafeSvc.exe"
  KillProcDLL::KillProc "KSafe.exe"

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer

  !insertmacro FileEx "bin\KSafeTray.exe" "$INSTDIR\KSafeTray.exe" "$INSTDIR\"
  !insertmacro FileEx "bin\KSafe.exe" "$INSTDIR\KSafe.exe" "$INSTDIR\"
  ;!insertmacro FileEx "bin\ksafeave.dll" "$INSTDIR\ksafeave.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafebak.dll" "$INSTDIR\ksafebak.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kcache.dll" "$INSTDIR\kcache.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafedb.dll" "$INSTDIR\ksafedb.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafeeng.dll" "$INSTDIR\ksafeeng.dll" "$INSTDIR\"
  ;!insertmacro FileEx "bin\ksignup.dll" "$INSTDIR\ksignup.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\KSafeSvc.exe" "$INSTDIR\KSafeSvc.exe" "$INSTDIR\"
  !insertmacro FileEx "bin\ksfmon.dll" "$INSTDIR\ksfmon.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafeexam.dll" "$INSTDIR\ksafeexam.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafevul.dll" "$INSTDIR\ksafevul.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kplugext.dll" "$INSTDIR\kplugext.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kplugeng.dll" "$INSTDIR\kplugeng.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafeup.dll" "$INSTDIR\ksafeup.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksafever.dll" "$INSTDIR\ksafever.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\katrun.dll" "$INSTDIR\katrun.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kiefixeng.dll" "$INSTDIR\kiefixeng.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\krunopt.dll" "$INSTDIR\krunopt.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\zlib1.dll" "$INSTDIR\zlib1.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcm80.dll" "$INSTDIR\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcp80.dll" "$INSTDIR\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcr80.dll" "$INSTDIR\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\knescan.dll" "$INSTDIR\knescan.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kexectrl.dll" "$INSTDIR\kexectrl.dll" "$INSTDIR\"

  !insertmacro FileEx "bin\KAV100602_ONLINE_10_10.exe" "$INSTDIR\KAV100602_ONLINE_10_10.exe" "$INSTDIR\"
  !insertmacro FileEx "bin\kwsctrl.dll" "$INSTDIR\kwsctrl.dll" "$INSTDIR\"

  !insertmacro FileEx "bin\kswbc.dll" "$INSTDIR\kswbc.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kwssp.dll" "$INSTDIR\kwssp.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kwsui.dll" "$INSTDIR\kwsui.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kxebase.dll" "$INSTDIR\kxebase.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\scom.dll" "$INSTDIR\scom.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kswebshield.dll" "$INSTDIR\kswebshield.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kdump.dll" "$INSTDIR\kdump.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kdumprep.exe" "$INSTDIR\kdumprep.exe" "$INSTDIR\"
  !insertmacro FileEx "bin\kse\sqlite.dll" "$INSTDIR\sqlite.dll" "$INSTDIR\"

  File "bin\sqlite3.dll"
  File "bin\json.dll"

  # 安装清理模块
  SetOutPath "$INSTDIR"
  !insertmacro FileEx "bin\kclear.dll" "$INSTDIR\kclear.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\fcache.dll" "$INSTDIR\fcache.dll" "$INSTDIR\"
  ;!insertmacro FileEx "bin\trashcleaner.dll" "$INSTDIR\trashcleaner.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\trackcleaner.dll" "$INSTDIR\trackcleaner.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\regcleaner.dll" "$INSTDIR\regcleaner.dll" "$INSTDIR\"
  SetOverwrite on
  SetOutPath "$INSTDIR\cfg"
  File "bin\cfg\onekeyclean.xml"       ; 一键清理配置文件
  File "bin\cfg\bigfilerule.dat"       ; 大文件管理文件规则
  File "bin\cfg\trashfilerule.dat"     ; 垃圾清理过滤规则
  File "bin\cfg\defaultoclean.xml"     ; 一键清理默认清理条目
  File "bin\cfg\trashfilerule.dat"     ; 垃圾清理过滤列表
  SetOutPath "$INSTDIR\data"
  File "bin\data\clearsoft.dat"           ; 痕迹清理配置文件
  File "bin\data\trashconf.dat"           ; 垃圾清理默认配置
  File /r /x .svn "bin\data\icon"         ; 图标文件

  # 安装网盾的文件
  SetOutPath "$INSTDIR"
  !insertmacro FileEx "bin\BC.sys" "$INSTDIR\BC.sys" "$INSTDIR\"
  !insertmacro FileEx "bin\bootsafe.sys" "$INSTDIR\bootsafe.sys" "$INSTDIR\"
  !insertmacro FileEx "bin\kavquara.dll" "$INSTDIR\kavquara.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kcldrep.dll" "$INSTDIR\kcldrep.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\khandler.dll" "$INSTDIR\khandler.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kscanner.dll" "$INSTDIR\kscanner.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksreng3.dll" "$INSTDIR\ksreng3.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksscore.dll" "$INSTDIR\ksscore.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kxebase.dll" "$INSTDIR\kxebase.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\kxestat.dll" "$INSTDIR\kxestat.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\scom.dll" "$INSTDIR\scom.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\pedownui.dll" "$INSTDIR\pedownui.dll" "$INSTDIR\"
  SetOverwrite on
  File "bin\kscantyp.xml"
  File "bin\kxecomm.dat"
  File "bin\productidinfo.ini"
  File "bin\ksrengcfg.ini"
  File "bin\vf1.dat"
  File "bin\kwsu.dat"
  File "bin\kws.ini"
  File "bin\kdh.dat"


 !insertmacro UpVirusDat4 "config3.dat" "2010.9.14.1" "1"
 !insertmacro UpVirusDat4 "WhiteList.datb" "2010.9.8.1" "1"
 !insertmacro UpVirusDat4 "sp3.nlb" "2010.9.14.1" "1"


  SetOutPath "$INSTDIR\kse"
  !insertmacro FileEx "bin\kse\ksbwdet2.dll" "$INSTDIR\kse\ksbwdet2.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "bin\kse\ksecorex.dll" "$INSTDIR\kse\ksecorex.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "bin\kse\sqlite.dll" "$INSTDIR\kse\sqlite.dll" "$INSTDIR\kse\"
  SetOverwrite on
  File "bin\kse\data.fsg"
  File "bin\kse\ksbwdt.ini"
  File "bin\kse\unknown.fsg"

  SetOutPath "$INSTDIR\kse\config"
  File "bin\kse\config\kse.stat_fac_cfg.xml"
  File "bin\kse\config\ksecore.addon.xml"
  File "bin\kse\config\ksecore.netdetcfg.xml"
  File "bin\kse\config\ksecore.sln.xml"
  File "bin\kse\config\kspfeng.filemonfilter.xml"
  File "bin\kse\config\kspfeng.polman.xml"
  File "bin\kse\config\kspfeng.pwlcfg.xml"

  SetOutPath "$INSTDIR\kxecore"
  !insertmacro FileEx "bin\kxecore\kxecore.dll" "$INSTDIR\kxecore\kxecore.dll" "$INSTDIR\kxecore\"

  # 写入三实例的支持模块
  SetOutPath "$INSTDIR\cp1"
  !insertmacro FileEx "bin\cp\KSafeSvc.exe" "$INSTDIR\cp1\KSafeSvc.exe" "$INSTDIR\cp1"
  File "/oname=$INSTDIR\cp1\kxecomm.dat" "bin\cp\kxecomm1.dat"
  File "/oname=$INSTDIR\cp1\Microsoft.VC80.CRT.manifest" "bin\Microsoft.VC80.CRT.manifest"
  !insertmacro FileEx "bin\msvcm80.dll" "$INSTDIR\cp1\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcp80.dll" "$INSTDIR\cp1\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcr80.dll" "$INSTDIR\cp1\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\knescan.dll" "$INSTDIR\cp1\knescan.dll" "$INSTDIR\"

  SetOutPath "$INSTDIR\cp2"
  !insertmacro FileEx "bin\cp\KSafeSvc.exe" "$INSTDIR\cp2\KSafeSvc.exe" "$INSTDIR\cp2"
  File "/oname=$INSTDIR\cp2\kxecomm.dat" "bin\cp\kxecomm2.dat"
  File "/oname=$INSTDIR\cp2\Microsoft.VC80.CRT.manifest" "bin\Microsoft.VC80.CRT.manifest"
  !insertmacro FileEx "bin\msvcm80.dll" "$INSTDIR\cp2\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcp80.dll" "$INSTDIR\cp2\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\msvcr80.dll" "$INSTDIR\cp2\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\knescan.dll" "$INSTDIR\cp2\knescan.dll" "$INSTDIR\"

  SetOutPath "$INSTDIR"
  # 写入网盾查杀的注册信息
  WriteRegStr HKLM "SOFTWARE\KSafe\KXEngine\KxEScanSystem\ksecore.config.top\appconfig\kse.kspfeng.kspolfile" "cfgval" "..\KEng\signs.ini"
  WriteRegStr HKLM "SOFTWARE\KSafe\KXEngine\KxEScanSystem\ksecore.config.top\appconfig\kse.kspfeng.ksksgpath" "cfgval" "..\KEng\ksg"
  WriteRegStr HKLM "SOFTWARE\KSafe\KXEngine\KxEScanSystem\ksecore.config.top\appconfig\kse.kspfeng.virinfo_cfgpath" "cfgval" "..\KEng\vinfo.ini"
  WriteRegStr HKLM "SOFTWARE\KSafe\KXEngine\KxEScanSystem\ksecore.config.top\appconfig\kse.kspfeng.virinfo_libpath" "cfgval" "..\KEng\ksg"
  WriteRegStr HKLM "SOFTWARE\KSafe\KXEngine\KxEScanSystem\ksecore.config.top\appconfig\kse.kspfeng.ksinifile" "cfgval" "..\KEng\kae\kaecore.ini"
  WriteRegStr HKLM "SOFTWARE\KSafe\KXEngine\KxEScanSystem\ksecore.config.top\appconfig\kse.kspfeng.ksbwmpath" "cfgval" "..\KEng\kae"

  # 写入网盾融合版本信息
  WriteRegStr HKLM "SOFTWARE\KSafe\kws" "i" "$INSTDIR\kwsctrl.dll"


  ;设置PID
  Call GetParternName
  pop $0
  StrCpy $N_OEM_NAME "$0"
  Call Install_Stat


  Push $0
  Push $1
  
  ;软件管家的安装...................
  StrCpy  $1 "$INSTDIR\KSoft"
  SetOutPath  $1
  !insertmacro FileEx "bin\KSoft\softmgr.dll" "$INSTDIR\KSoft\softmgr.dll" "$INSTDIR\KSoft\"

  StrCpy  $1 "$INSTDIR\KSoft\data"
  SetOutPath  $1
  File /r /x .svn "bin2\KSoft\data\*.*"
  StrCpy  $1 "$INSTDIR\KSoft\html"
  SetOutPath  $1
  File /r /x .svn "bin2\KSoft\html\*.*"
  StrCpy  $1 "$INSTDIR\KSoft\icon"
  SetOutPath  $1
  File /r /x .svn "bin2\KSoft\icon\*.*"
  ;软件管家安装结束..................
  
  StrCpy  $1 "$INSTDIR\"
  SetOutPath  $1
  File /r /x .svn "bin\Microsoft.VC80.CRT.manifest"

  StrCpy  $1 "$INSTDIR\data"
  SetOutPath  $1
  !insertmacro UpVirusDat2 "script.db" "1.0.0.1" "0"

  
  StrCpy  $1 "$INSTDIR\data"
  SetOutPath  $1
  File /r /x .svn /x "sfm2" "bin2\data\*.*"
  
  StrCpy  $1 "$INSTDIR\data\sfm3kb"
  SetOutPath  $1
  !insertmacro UpVirusDat3 "sfm3kb.dat" "2010.7.13.1" "0"

  
  ;!insertmacro UpVirusDat "0501.0000.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0000.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0100.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0100.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0200.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0200.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0300.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0501.0300.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0502.0000.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0502.0000.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0502.0100.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0502.0100.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0502.0200.1033sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0502.0200.2052sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0600.0000.0000sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0600.0100.0000sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0600.0200.0000sysfix.idx" "1.0.0.1"
  ;!insertmacro UpVirusDat "0601.0000.0000sysfix.idx" "1.0.0.1"

  SetOverwrite on
  SetOutPath  "$INSTDIR\webui"
  File /r /x .svn "bin\webui\*.*"

  SetOverWrite ifnewer

  SetOutPath "$INSTDIR"
  Pop  $1
  Pop $0
  
  ;写入安装路径
  ;WriteRegStr HKLM "${REGKEY_PRODUCT}" "${REGVAL_INSTALL}" "$INSTDIR"
  ExecWait "$INSTDIR\KSafeSvc.exe /stop"
  ExecWait "$INSTDIR\KSafeSvc.exe /start"

SectionEnd

