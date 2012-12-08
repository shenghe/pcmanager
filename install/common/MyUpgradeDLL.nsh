!echo "Macro - Upgrade DLL File"

!ifndef ASS_VERBOSE
  !define ASS_VERBOSE 3
!endif

!verbose push
!verbose ${ASS_VERBOSE}


!ifndef UPGRADEDLL_INCLUDED

!define UPGRADEDLL_INCLUDED

; Macro - Upgrade DLL File
; Written by Joost Verburg
; ------------------------
;
; Parameters:
; LOCALFILE   - Location of the new DLL file (on the compiler system)
; DESTFILE    - Location of the DLL file that should be upgraded (on the user's system)
; TEMPBASEDIR - Directory on the user's system to store a temporary file when the system has
;               to be rebooted.
;               For Win9x support, this should be on the same volume as the DESTFILE!
;               The Windows temp directory could be located on any volume, so you cannot use
;               this directory.
;
; Define UPGRADEDLL_NOREGISTER if you want to upgrade a DLL that does not have to be registered.
;
; Note: If you want to support Win9x, you can only use short filenames (8.3).
;
; Example of usage:
; !insertmacro UpgradeDLL "dllname.dll" "$SYSDIR\dllname.dll" "$SYSDIR"
;

!macro UpgradeDLL LOCALFILE DESTFILE TEMPBASEDIR
  !verbose push
  !verbose ${ASS_VERBOSE}

  Push $R0
  Push $R1
  Push $R2
  Push $R3

  ;------------------------
  ;Unique number for labels

  !define UPGRADEDLL_UNIQUE ${__FILE__}${__LINE__}

  ;------------------------
  ;Check file and version

  IfFileExists "${DESTFILE}" 0 upgradedll.copy_${UPGRADEDLL_UNIQUE}

!ifdef UPGADFEDLL_FORCE
  Goto upgradedll.upgrade_${UPGRADEDLL_UNIQUE}
!else
  ClearErrors
  !ifdef UPGADFEDLL_FILETIME
    GetFileTimeLocal "${LOCALFILE}" $R0 $R1
    GetFileTime "${DESTFILE}" $R2 $R3
  !else
    GetDLLVersionLocal "${LOCALFILE}" $R0 $R1
    GetDLLVersion "${DESTFILE}" $R2 $R3
  !endif
!endif

  IfErrors upgradedll.upgrade_${UPGRADEDLL_UNIQUE}

;  IntCmpU $R0 $R2 0 upgradedll.done_${UPGRADEDLL_UNIQUE} upgradedll.upgrade_${UPGRADEDLL_UNIQUE}
;  IntCmpU $R1 $R3 upgradedll.noreboot_${UPGRADEDLL_UNIQUE} upgradedll.done_${UPGRADEDLL_UNIQUE} \
;    upgradedll.upgrade_${UPGRADEDLL_UNIQUE}

; 规则:如果源版本号大于目标文件版本号，更新目标文件，并注册
;      否则(源版本小于等于目标文件)，注册目标文件
  IntCmpU $R0 $R2 0 upgradedll.noreboot_${UPGRADEDLL_UNIQUE} upgradedll.upgrade_${UPGRADEDLL_UNIQUE}
  IntCmpU $R1 $R3 upgradedll.noreboot_${UPGRADEDLL_UNIQUE} upgradedll.noreboot_${UPGRADEDLL_UNIQUE} upgradedll.upgrade_${UPGRADEDLL_UNIQUE}

  ;------------------------
  ;Let's upgrade the DLL!

  SetOverwrite try

  upgradedll.upgrade_${UPGRADEDLL_UNIQUE}:
    !ifndef UPGRADEDLL_NOREGISTER
      ;Unregister the DLL
      ;UnRegDLL "${DESTFILE}"
    !endif

  ;------------------------
  ;Try to copy the DLL directly

  ClearErrors
    StrCpy $R0 "${DESTFILE}"
    Call :upgradedll.file_${UPGRADEDLL_UNIQUE}
  IfErrors 0 upgradedll.noreboot_${UPGRADEDLL_UNIQUE}

  ;------------------------
  ;DLL is in use. Copy it to a temp file and Rename it on reboot.

  GetTempFileName $R0 "${TEMPBASEDIR}"
    Call :upgradedll.file_${UPGRADEDLL_UNIQUE}
  Rename /REBOOTOK $R0 "${DESTFILE}"

  ;------------------------
  ;Register the DLL on reboot

  !ifndef UPGRADEDLL_NOREGISTER
    GetFullPathName /SHORT $R4 ${DESTFILE}
    ;WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "Register_$R4" "Rundll32.exe $R4,DllRegisterServer"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "Register_$R4" "$SYSDIR\regsvr32.exe /s $R4"
  !endif

  Goto upgradedll.done_${UPGRADEDLL_UNIQUE}

  ;------------------------
  ;DLL does not exist - just extract

  upgradedll.copy_${UPGRADEDLL_UNIQUE}:
    StrCpy $R0 "${DESTFILE}"
    Call :upgradedll.file_${UPGRADEDLL_UNIQUE}

  ;------------------------
  ;Register the DLL

  upgradedll.noreboot_${UPGRADEDLL_UNIQUE}:
    !ifndef UPGRADEDLL_NOREGISTER
      RegDLL "${DESTFILE}"
    !endif

  ;------------------------
  ;Done

  upgradedll.done_${UPGRADEDLL_UNIQUE}:

  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0

  ;------------------------
  ;End

  Goto upgradedll.end_${UPGRADEDLL_UNIQUE}

  ;------------------------
  ;Called to extract the DLL

  upgradedll.file_${UPGRADEDLL_UNIQUE}:
    File /oname=$R0 "${LOCALFILE}"
    Return

  upgradedll.end_${UPGRADEDLL_UNIQUE}:

 ;------------------------
 ;Restore settings

 SetOverwrite lastused
 
 !undef UPGRADEDLL_UNIQUE

 !verbose pop
!macroend

!endif
!verbose pop
