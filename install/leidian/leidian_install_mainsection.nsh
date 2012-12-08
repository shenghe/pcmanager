
Section "MainSection" SEC01

  ${if} ${silent}

	${else}
	  SetShellVarContext all
	  StrCmpS $INSTDIR $DESKTOP 0 label_not1
	  MessageBox MB_YESNO "您是否要安装在桌面目录上？" IDYES label_go_continue IDNO label_key_dir_exit
	label_not1:
	  SetShellVarContext current
	  StrCmpS $INSTDIR $DESKTOP 0 label_go_continue
	  MessageBox MB_YESNO "您是否要安装在桌面目录上？" IDYES label_go_continue IDNO label_key_dir_exit
		Goto label_go_continue
	label_key_dir_exit:
	  Abort
	label_go_continue:
	
	${endif}

  ;关掉当前运行的
  !insertmacro KillLeidianMain
  !insertmacro KillLeidianTray
; 卸载以前的服务
  push $0
  push $1
  Call GetInstalledLeidianPath
  ${if} $0 != "0"
       Strcpy $1 "$0\KSafeSvc.exe"
       IfFileExists $1 0 +2
        ;KillProcDLL::KillProc "KSafeSvc.exe"
        ExecWait "$1 /stop"
        ExecWait "$1 /uninstall"
  ${endif}
  pop $1
  pop $0
  
  ;KillProcDLL::KillProc "KSafeSvc.exe"
  !insertmacro KillLeidianMain
  KillProcDLL::KillProc "kprivacy.exe"
  KillProcDLL::KillProc "netmon.exe"
  KillProcDLL::KillProc "kmspeed.exe"
  KillProcDLL::KillProc "perfopt.exe"
  
  Delete "$INSTDIR\Appdata\ksoft_cache_2.dat"
  Delete "$INSTDIR\Appdata\ksoft_upcache.dat"
  Delete "$INSTDIR\Appdata\ksoft_ucache_4_flag.dat"
  Delete "$INSTDIR\Appdata\ksoft_ucache_4.dat"
  Delete "$INSTDIR\Appdata\ksoft_setuped.dat"
  Delete "$INSTDIR\Appdata\ksoft_info.dat"
  Delete "$INSTDIR\Appdata\ksoft_cache_2.dat"

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
!ifndef PRODUCT_UPDATE
  !insertmacro FileEx "${BIN_DIR}\KnInstAD.exe" "$INSTDIR\KnInstAD.exe" "$INSTDIR\"
!endif
  !insertmacro FileEx "${BIN_DIR}\KSafeTray.exe" "$INSTDIR\KSafeTray.exe" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\KSafe.exe" "$INSTDIR\KSafe.exe" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kscan.dll" "$INSTDIR\kscan.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksshield.dll" "$INSTDIR\ksshield.dll" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\ksafeave.dll" "$INSTDIR\ksafeave.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksafedb.dll" "$INSTDIR\ksafedb.dll" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\ksignup.dll" "$INSTDIR\ksignup.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\KSafeSvc.exe" "$INSTDIR\KSafeSvc.exe" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksfmon.dll" "$INSTDIR\ksfmon.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksfmonsys64.sys" "$INSTDIR\ksfmonsys64.sys" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksafeexam.dll" "$INSTDIR\ksafeexam.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksafevul.dll" "$INSTDIR\ksafevul.dll" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\kfiled.dll" "$INSTDIR\kfiled.dll" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\kplugext.dll" "$INSTDIR\kplugext.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kplugeng.dll" "$INSTDIR\kplugeng.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksafeup.dll" "$INSTDIR\ksafeup.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksafever.dll" "$INSTDIR\ksafever.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kiefixeng.dll" "$INSTDIR\kiefixeng.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\krunopt.dll" "$INSTDIR\krunopt.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\zlib1.dll" "$INSTDIR\zlib1.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcm80.dll" "$INSTDIR\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcp80.dll" "$INSTDIR\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcr80.dll" "$INSTDIR\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\knescan.dll" "$INSTDIR\knescan.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kexectrl.dll" "$INSTDIR\kexectrl.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kavmgr.dll" "$INSTDIR\kavmgr.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kproclib.dll" "$INSTDIR\kproclib.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\perfopt.exe" "$INSTDIR\perfopt.exe" "$INSTDIR\"

  !insertmacro FileEx "${BIN_DIR}\kwsctrl.dll" "$INSTDIR\kwsctrl.dll" "$INSTDIR\"

  !insertmacro FileEx "${BIN_DIR}\kswbc.dll" "$INSTDIR\kswbc.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kwssp.dll" "$INSTDIR\kwssp.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kwsui.dll" "$INSTDIR\kwsui.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kxebase.dll" "$INSTDIR\kxebase.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\scom.dll" "$INSTDIR\scom.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kswebshield.dll" "$INSTDIR\kswebshield.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kdump.dll" "$INSTDIR\kdump.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kdumprep.exe" "$INSTDIR\kdumprep.exe" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kdumpfix.exe" "$INSTDIR\kdumpfix.exe" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kse\sqlite.dll" "$INSTDIR\sqlite.dll" "$INSTDIR\"
  
  
  ;!insertmacro FileEx "${BIN_DIR}\kprivacy.exe" "$INSTDIR\kprivacy.exe" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\KAV_ONLINE_95_11.exe" "$INSTDIR\KAV_ONLINE_95_11.exe" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\fwproxy.dll" "$INSTDIR\fwproxy.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\netstat.dll" "$INSTDIR\netstat.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kmodurl.sys" "$INSTDIR\kmodurl.sys" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kmodurl64.sys" "$INSTDIR\kmodurl64.sys" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\kmspeed.exe" "$INSTDIR\kmspeed.exe" "$INSTDIR\"

  File "${BIN_DIR}\sqlite3.dll"
  File "${BIN_DIR}\json.dll"

  # 安装清理模块
  SetOutPath "$INSTDIR"
  !insertmacro FileEx "${BIN_DIR}\kclear.dll" "$INSTDIR\kclear.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\fcache.dll" "$INSTDIR\fcache.dll" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\trashcleaner.dll" "$INSTDIR\trashcleaner.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\trackcleaner.dll" "$INSTDIR\trackcleaner.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\regcleaner.dll" "$INSTDIR\regcleaner.dll" "$INSTDIR\"
  SetOverwrite on
  SetOutPath "$INSTDIR\cfg"
  File "${BIN_DIR}\cfg\bigfilerule.dat"       ; 大文件管理文件规则
  File "${BIN_DIR}\cfg\trashfilerule.dat"     ; 垃圾清理过滤规则
  File "${BIN_DIR}\cfg\defaultoclean.xml"     ; 一键清理默认清理条目
  SetOutPath "$INSTDIR\data"
  File "${BIN_DIR}\data\clearsoft.dat"           ; 痕迹清理配置文件
  File "${BIN_DIR}\data\trashconf.dat"           ; 垃圾清理默认配置
  File /r /x .svn "${BIN_DIR}\data\icon"         ; 图标文件

  # 安装网盾的文件
  SetOutPath "$INSTDIR"
  !insertmacro FileEx "${BIN_DIR}\BC.sys" "$INSTDIR\BC.sys" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kavquara.dll" "$INSTDIR\kavquara.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kcldrep.dll" "$INSTDIR\kcldrep.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\khandler.dll" "$INSTDIR\khandler.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kscanner.dll" "$INSTDIR\kscanner.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksreng3.dll" "$INSTDIR\ksreng3.dll" "$INSTDIR\"
  ;!insertmacro FileEx "${BIN_DIR}\kssdet.dll" "$INSTDIR\kssdet.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\ksscore.dll" "$INSTDIR\ksscore.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kxebase.dll" "$INSTDIR\kxebase.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kxestat.dll" "$INSTDIR\kxestat.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\scom.dll" "$INSTDIR\scom.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\pedownui.dll" "$INSTDIR\pedownui.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\kwsplugin.dll" "$INSTDIR\kwsplugin.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\netmon.exe" "$INSTDIR\netmon.exe" "$INSTDIR\"

  SetOverwrite on
  File "${BIN_DIR}\kscantyp.xml"
  File "${BIN_DIR}\kxecomm.dat"
  File "${BIN_DIR}\productidinfo.ini"
  File "${BIN_DIR}\ksrengcfg.ini"
  File "${BIN_DIR}\ksscfgx.ini"
  File "${BIN_DIR}\vf1.dat"
  File "${BIN_DIR}\kwsu.dat"
  File "${BIN_DIR}\kws.ini"
  File "${BIN_DIR}\kdh.dat"
  File "${BIN_DIR}\bro.cfg"
  File "${BIN_DIR}\ksais.dat"
  File "${BIN_DIR}\kdumpcfg.dat"
  File "${BIN_DIR}\kqsccfg.xml"
  File "${BIN_DIR}\plugtrust.ini"
  File "${BIN_DIR}\kwsprio.ini"
  File "${BIN_DIR}\oem.ini"
  
 !insertmacro UpVirusDat4 "config3.dat" "2011.4.2" "1"
 !insertmacro UpVirusDat4 "WhiteList.dat" "2010.9.8.1" "1"
 !insertmacro UpVirusDat4 "sp3.nlb" "2011.4.2" "1"
 !insertmacro UpVirusDat4 "kwsu.dat" "2010.12.16" "1"
 !insertmacro UpVirusDat4 "vf1.dat" "2011.1.19" "1"

  SetOutPath "$INSTDIR\kse"
  !insertmacro FileEx "${BIN_DIR}\kse\ksbwdet2.dll" "$INSTDIR\kse\ksbwdet2.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\bkrescan.dll" "$INSTDIR\kse\bkrescan.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\ksbcommsp.dll" "$INSTDIR\kse\ksbcommsp.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\ksecorex.dll" "$INSTDIR\kse\ksecorex.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\sqlite.dll" "$INSTDIR\kse\sqlite.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\wfs.dll" "$INSTDIR\kse\wfs.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\kavquara.dll" "$INSTDIR\kse\kavquara.dll" "$INSTDIR\kse\"
  !insertmacro FileEx "${BIN_DIR}\kse\ksecansp.dll" "$INSTDIR\kse\ksecansp.dll" "$INSTDIR\kse\"
  SetOverwrite on
  File "${BIN_DIR}\kse\data.fsg"
  File "${BIN_DIR}\kse\ksbwdt.ini"
  File "${BIN_DIR}\kse\unknown.fsg"
  File "${BIN_DIR}\kse\quarantine.ini"
  File "${BIN_DIR}\kse\ksecansp.xml"
  File "${BIN_DIR}\kse\chcfg.ini"
  File "${BIN_DIR}\kse\kse.sysbinfile.dat"

  SetOutPath "$INSTDIR\ksrcfg"
  SetOverwrite on
  File "${BIN_DIR}\ksrcfg\config3.dat"
  File "${BIN_DIR}\ksrcfg\sysrep3.dat"
  
  SetOutPath "$INSTDIR\kse\config"
  File "${BIN_DIR}\kse\config\kse.stat_fac_cfg.xml"
  File "${BIN_DIR}\kse\config\ksecore.addon.xml"
  File "${BIN_DIR}\kse\config\ksecore.netdetcfg.xml"
  File "${BIN_DIR}\kse\config\ksecore.sln.xml"
  File "${BIN_DIR}\kse\config\kspfeng.filemonfilter.xml"
  File "${BIN_DIR}\kse\config\kspfeng.polman.xml"
  File "${BIN_DIR}\kse\config\kspfeng.pwlcfg.xml"
  
  SetOutPath "$INSTDIR\kxecore"
  !insertmacro FileEx "${BIN_DIR}\kxecore\kxecore.dll" "$INSTDIR\kxecore\kxecore.dll" "$INSTDIR\kxecore\"
  
  # 写入N实例的支持模块 --- 四个实例的支持 拉拉呼啦啦
  SetOutPath "$INSTDIR\cp1"
  !insertmacro FileEx "${BIN_DIR}\cp\KSafeSvc.exe" "$INSTDIR\cp1\KSafeSvc.exe" "$INSTDIR\cp1"
  File "/oname=$INSTDIR\cp1\kxecomm.dat" "${BIN_DIR}\cp\kxecomm1.dat"
  File "/oname=$INSTDIR\cp1\productidinfo.ini" "${BIN_DIR}\cp\productidinfo1.ini"
  File "/oname=$INSTDIR\cp1\ksscfgx.ini" "${BIN_DIR}\cp\ksscfgx1.ini"
  File "/oname=$INSTDIR\cp1\Microsoft.VC80.CRT.manifest" "${BIN_DIR}\Microsoft.VC80.CRT.manifest"
  !insertmacro FileEx "${BIN_DIR}\msvcm80.dll" "$INSTDIR\cp1\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcp80.dll" "$INSTDIR\cp1\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcr80.dll" "$INSTDIR\cp1\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\knescan.dll" "$INSTDIR\cp1\knescan.dll" "$INSTDIR\"

  SetOutPath "$INSTDIR\cp2"
  !insertmacro FileEx "${BIN_DIR}\cp\KSafeSvc.exe" "$INSTDIR\cp2\KSafeSvc.exe" "$INSTDIR\cp2"
  File "/oname=$INSTDIR\cp2\kxecomm.dat" "${BIN_DIR}\cp\kxecomm2.dat"
  File "/oname=$INSTDIR\cp2\productidinfo.ini" "${BIN_DIR}\cp\productidinfo2.ini"
  File "/oname=$INSTDIR\cp2\Microsoft.VC80.CRT.manifest" "${BIN_DIR}\Microsoft.VC80.CRT.manifest"
  !insertmacro FileEx "${BIN_DIR}\msvcm80.dll" "$INSTDIR\cp2\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcp80.dll" "$INSTDIR\cp2\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcr80.dll" "$INSTDIR\cp2\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\knescan.dll" "$INSTDIR\cp2\knescan.dll" "$INSTDIR\"
  
  SetOutPath "$INSTDIR\cp3"
  !insertmacro FileEx "${BIN_DIR}\cp\KSafeSvc.exe" "$INSTDIR\cp3\KSafeSvc.exe" "$INSTDIR\cp3"
  File "/oname=$INSTDIR\cp3\kxecomm.dat" "${BIN_DIR}\cp\kxecomm3.dat"
  File "/oname=$INSTDIR\cp3\productidinfo.ini" "${BIN_DIR}\cp\productidinfo3.ini"
  File "/oname=$INSTDIR\cp3\Microsoft.VC80.CRT.manifest" "${BIN_DIR}\Microsoft.VC80.CRT.manifest"
  !insertmacro FileEx "${BIN_DIR}\msvcm80.dll" "$INSTDIR\cp3\msvcm80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcp80.dll" "$INSTDIR\cp3\msvcp80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\msvcr80.dll" "$INSTDIR\cp3\msvcr80.dll" "$INSTDIR\"
  !insertmacro FileEx "${BIN_DIR}\knescan.dll" "$INSTDIR\cp3\knescan.dll" "$INSTDIR\"

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
  
  
!ifndef BIND_WITH_OTHER_SOFTMGR
  ;软件管家的安装...................
  StrCpy  $1 "$INSTDIR\KSoft"
  SetOutPath  $1
  !insertmacro FileEx "${BIN_DIR}\ksoft\softmgr.dll" "$INSTDIR\KSoft\softmgr.dll" "$INSTDIR\KSoft\"
  !insertmacro FileEx "${BIN_DIR}\ksoft\kauinsc.dll" "$INSTDIR\KSoft\kauinsc.dll" "$INSTDIR\KSoft\"
  !insertmacro FileEx "${BIN_DIR}\ksoft\kauins.exe" "$INSTDIR\KSoft\kauins.exe" "$INSTDIR\KSoft\"
  !insertmacro FileEx "${BIN_DIR}\kpcfileopen.exe" "$INSTDIR\kpcfileopen.exe" "$INSTDIR\KSoft\"

  SetOverwrite on
  File /oname=$INSTDIR\KSoft\softmgr.ico "${BIN_DIR}\KSoft\softmgr.ico"
  !ifndef PRODUCT_UPDATE
		SetOverwrite on
  !else
		SetOverwrite off
  !endif
  StrCpy  $1 "$INSTDIR\KSoft\data"
  SetOutPath  $1
  File /r /x .svn "${BIN_DIR}\KSoft\data\*.*"
  SetOverwrite on
  StrCpy  $1 "$INSTDIR\KSoft\html"
  SetOutPath  $1
  File /r /x .svn "${BIN_DIR}\KSoft\html\*.*"
  StrCpy  $1 "$INSTDIR\KSoft\icon"
  SetOutPath  $1
  File /r /x .svn "${BIN_DIR}\KSoft\icon\*.*"
  ;软件管家安装结束..................
!endif

	SetOverwrite on
  StrCpy  $1 "$INSTDIR\"
  SetOutPath  $1
  File /r /x .svn "${BIN_DIR}\Microsoft.VC80.CRT.manifest"

  !ifndef PRODUCT_UPDATE
		SetOverwrite on
  !else
		SetOverwrite off
  !endif
  
  StrCpy  $1 "$INSTDIR\data"
  SetOutPath  $1
  File /r /x .svn /x "sfm2" "${BIN_DIR}\data\*.*"

  !insertmacro UpVirusDat2 "script.db" "1.0.0.1" "0"
  
  ;StrCpy  $1 "$INSTDIR\data\sfm3kb"
  ;SetOutPath  $1
  ;!insertmacro UpVirusDat3 "sfm3kb.dat" "2010.7.13.1" "0"
  
  ;StrCpy  $1 "$INSTDIR\data\sfm2"
  ;SetOutPath  $1
  
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;;; 打包隐私保护器
  ;SetOverWrite on
  ;StrCpy  $1 "$INSTDIR\kprivacy"
  ;SetOutPath  $1

	; 先把之前的exe删掉
  ;!insertmacro DeleteFileEx "$INSTDIR\kprivacy.exe" "$INSTDIR"
  ; 装新的
  ;!insertmacro FileEx "${BIN_DIR}\kprivacy\kprivacy.exe" "$1\kprivacy.exe" "$1\"
  ;!insertmacro FileEx "${BIN_DIR}\kprivacy\kclear.dll" "$1\kclear.dll" "$1\"
  ;!insertmacro FileEx "${BIN_DIR}\trackcleaner.dll" "$1\trackcleaner.dll" "$1\"

  ;File "${BIN_DIR}\kprivacy\kclear.kui"
  ;File "${BIN_DIR}\kprivacy\rule.dat"
  ;File "${BIN_DIR}\kprivacy\custom_rule.dat"

	; 复制ICON
  ;StrCpy  $1 "$INSTDIR\kprivacy\data\icon"
  ;SetOutPath  $1
  ;File /r "${BIN_DIR}\data\icon\*.*"

	; 复制CFG文件
	;SetOverwrite on
  ;SetOutPath "$INSTDIR\kprivacy\cfg"
  ;File "${BIN_DIR}\cfg\bigfilerule.dat"       ; 大文件管理文件规则

	;;; 隐私保护器结束
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


  SetOverWrite off
  StrCpy  $1 "$INSTDIR\cfg"
  SetOutPath  $1

!ifdef MACRO_CFG_PATH
  ;StrCpy $1 "${BIN_DIR}\{$MACRO_CFG_PATH}\*.*"
  File /r /x .svn "${BIN_DIR}\${MACRO_CFG_PATH}\*.*"
!else
  File /r /x .svn "${BIN_DIR}\cfg\*.*"
!endif

; 如果有自定义皮肤，则复制整个目录
!ifdef RES_SKIN_PATH
	SetOverwrite on
  StrCpy  $1 "$INSTDIR\res"
  SetOutPath  $1
  File /r /x .svn "${RES_SKIN_PATH}\*.*"
!endif

  SetOverwrite on
  SetOutPath  "$INSTDIR\webui"
  File /r /x .svn "${BIN_DIR}\webui\*.*"
	
  SetOverWrite ifnewer

  SetOutPath "$INSTDIR"
  Pop  $1
  Pop $0
  
  
  ;写入安装路径
  WriteRegStr HKLM "${REGKEY_PRODUCT}" "${REGVAL_INSTALL}" "$INSTDIR"
  
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; 直接解压毒霸引擎
  SetOutPath "$INSTDIR\KEng"
  SetOverwrite ifnewer
  !insertmacro FileEx "${BIN_DIR}\KEng\ksafeave.dll" "$INSTDIR\KEng\ksafeave.dll" "$INSTDIR\KEng"
  !insertmacro FileEx "${BIN_DIR}\KEng\ksignup.dll"  "$INSTDIR\KEng\ksignup.dll"  "$INSTDIR\KEng"
  !insertmacro FileEx "${BIN_DIR}\KEng\ksgmerge.dll" "$INSTDIR\KEng\ksgmerge.dll" "$INSTDIR\KEng"

  SetOverwrite try
  File "${BIN_DIR}\KEng\signs.ini"
  File "${BIN_DIR}\KEng\vinfo.ini"
  Push $1
  SetOverwrite try
  StrCpy  $1 "$INSTDIR\KEng\ksg"
  SetOutPath  $1
  File /r "${BIN_DIR}\KEng\ksg\*.*"
      
  StrCpy  $1 "$INSTDIR\KEng\kae"
  SetOutPath  $1
  File /r "${BIN_DIR}\KEng\kae\*.*"

  Pop $1
      WriteRegDWORD HKLM ${PRODUCT_ENG_MARK} ${PRODUCT_ENG_MARK_VALUE} 1
  ; 解压毒霸引擎完毕
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  
  ;自动开机启动
  ;!ifdef WRITE_TRAY_REG_RUN
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "${LEIDIAN_TRAY_AUTORUN}" '"$INSTDIR\KSafeTray.exe" -autorun'
  ;!endif
  ExecWait "$INSTDIR\KSafeSvc.exe /uninstall"
  ExecWait "$INSTDIR\KSafeSvc.exe /install"
  
  ${if} ${silent}
  ${else}
	  SetDetailsPrint none
	  ExecWait '"$INSTDIR\KSafe.exe" -setupcache:$HWNDPARENT"'
	  ;ExecWait "$INSTDIR\KSafeSvc.exe /stop"
	  SetDetailsPrint both
  ${endif}
  
  !ifdef WRITE_PID_SPECAIL_VALUE
    WriteRegDWORD HKLM "SOFTWARE\KSafe\Coop" "${SPECAIL_KEY_VALUE}" "${SPECAIL_KEY_VALUE_DATA}"
  !endif
!ifndef PRODUCT_UPDATE
	${if} ${silent}
  ${else}
	  SetDetailsPrint none
	  ExecWait '"$INSTDIR\KnInstAD.exe" -h:$HWNDPARENT -t:金山卫士 -i:0'
	  SetDetailsPrint both
  ${endif}
!endif
	
SectionEnd

Section -AdditionalIcons
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\金山卫士"
  CreateShortCut "$SMPROGRAMS\金山卫士\金山卫士.lnk" "$INSTDIR\KSafe.exe"
  CreateShortCut "$DESKTOP\金山卫士.lnk" "$INSTDIR\KSafe.exe"
  CreateShortCut "$SMPROGRAMS\金山卫士\卸载金山卫士.lnk" "$INSTDIR\uninst.exe"
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\金山卫士\金山卫士首页.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
SectionEnd
