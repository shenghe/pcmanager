;-------------------------------------------------------------------------------------------------------
; ¹«ÓÃº¯Êý
;-------------------------------------------------------------------------------------------------------
!macro MyGetTempFileName
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4

  GetTempFileName $1
  GetTempFileName $2
  Delete $1
  Delete $2

  StrCpy $3 $1 "" -7
  StrCpy $1 $3 3

  StrCpy $4 $2 "" -8
  StrCpy $2 $4 3

;  StrCpy $0 "$WINDIR\$1$2.exe"
  StrCpy $0 "$TEMP\$1$2.dll"

  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
!macroend
;-------------------------------------------------------------------------------------------------------
!macro ExitProg WndClass WndName wParam lParam
  Push $0
  FindWindow $0 ${WndClass} ${WndName}
  
  ${if} $0 != 0
    SendMessage $0 ${WM_CLOSE} ${wParam} ${lParam}
    Sleep 1000
  ${endif}
  
  Pop $0
!macroend

;---------------------------------------------------------------------------------------------------
!macro DeleteFileEx FileName TmpPath
  ClearErrors
  Delete "${FileName}"
  ${if} ${Errors}
    Push $0
    GetTempFileName $0
    Delete $0
    
    ClearErrors
    Rename "${FileName}" "$0"
    
    ${if} ${FileExists} ${FileName}
      GetTempFileName $0 ${TmpPath}
      Delete $0
      Rename "${FileName}" "$0"
    ${endif}
    
    Delete /REBOOTOK $0
    Pop $0
  ${endif}
!macroend
; -----------------------------------------------------------------------------------------------------
Var N_OEM_NAME

Function Install_Stat
   Push $0
   Push $1

   ${if} $N_OEM_NAME == ""
      StrCpy $N_OEM_NAME "h_home"
   ${endif}

   ReadRegStr $0 HKLM ${REGKEY_COOP} "PreOEM"
   ReadRegStr $1 HKLM ${REGKEY_COOP} "OEMName"

   DeleteRegValue  HKLM ${REGKEY_COOP} "OEM"
   DeleteRegValue  HKLM ${REGKEY_COOP} "PreOEM"
   DeleteRegValue  HKLM ${REGKEY_COOP} "OEMName"

   ${if} $0 == ""
      WriteRegStr HKLM ${REGKEY_COOP} "PreOEM" $N_OEM_NAME
   ${else}
      WriteRegStr   HKLM ${REGKEY_COOP} "PreOEM" "$N_OEM_NAME_Installed_$1"
   ${endif}

   WriteRegStr HKLM ${REGKEY_COOP} "OEMName"  $N_OEM_NAME
!ifndef PRODUCT_UPDATE
   WriteRegStr HKLM ${REGKEY_COOP} "OrgOEM"   $N_OEM_NAME
!endif

   Pop $1
   Pop $0
FunctionEnd

Function GetWindowsVersion
   Push $R0
   Push $R1
   ClearErrors
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
   IfErrors 0 lbl_winnt
   ; we are not NT
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber

   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error

   StrCpy $R1 $R0 3

   StrCmp $R1 '4.0' lbl_win32_95
   StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98

   lbl_win32_95:
     StrCpy $R0 '95'
   Goto lbl_done

   lbl_win32_98:
     StrCpy $R0 '98'
   Goto lbl_done

   lbl_win32_ME:
     StrCpy $R0 'ME'
   Goto lbl_done

   lbl_winnt:

   StrCpy $R1 $R0 1

   StrCmp $R1 '3' lbl_winnt_x
   StrCmp $R1 '4' lbl_winnt_x

   StrCpy $R1 $R0 3

   StrCmp $R1 '5.0' lbl_winnt_2000
   StrCmp $R1 '5.1' lbl_winnt_XP
   StrCmp $R1 '5.2' lbl_winnt_2003
   StrCmp $R1 '6.0' lbl_winnt_vista lbl_error

   lbl_winnt_x:
     StrCpy $R0 "NT $R0" 6
   Goto lbl_done

   lbl_winnt_2000:
     Strcpy $R0 '2000'
   Goto lbl_done

   lbl_winnt_XP:
     Strcpy $R0 'XP'
   Goto lbl_done

   lbl_winnt_2003:
     Strcpy $R0 '2003'
   Goto lbl_done

   lbl_winnt_vista:
     Strcpy $R0 'Vista'
   Goto lbl_done

   lbl_error:
     Strcpy $R0 ''
   lbl_done:

   Pop $R1
   Exch $R0
 FunctionEnd

 Function un.GetWindowsVersion
   Push $R0
   Push $R1
   ClearErrors
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
   IfErrors 0 lbl_winnt
   ; we are not NT
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber

   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error

   StrCpy $R1 $R0 3

   StrCmp $R1 '4.0' lbl_win32_95
   StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98

   lbl_win32_95:
     StrCpy $R0 '95'
   Goto lbl_done

   lbl_win32_98:
     StrCpy $R0 '98'
   Goto lbl_done

   lbl_win32_ME:
     StrCpy $R0 'ME'
   Goto lbl_done

   lbl_winnt:

   StrCpy $R1 $R0 1

   StrCmp $R1 '3' lbl_winnt_x
   StrCmp $R1 '4' lbl_winnt_x

   StrCpy $R1 $R0 3

   StrCmp $R1 '5.0' lbl_winnt_2000
   StrCmp $R1 '5.1' lbl_winnt_XP
   StrCmp $R1 '5.2' lbl_winnt_2003
   StrCmp $R1 '6.0' lbl_winnt_vista lbl_error

   lbl_winnt_x:
     StrCpy $R0 "NT $R0" 6
   Goto lbl_done

   lbl_winnt_2000:
     Strcpy $R0 '2000'
   Goto lbl_done

   lbl_winnt_XP:
     Strcpy $R0 'XP'
   Goto lbl_done

   lbl_winnt_2003:
     Strcpy $R0 '2003'
   Goto lbl_done

   lbl_winnt_vista:
     Strcpy $R0 'Vista'
   Goto lbl_done

   lbl_error:
     Strcpy $R0 ''
   lbl_done:

   Pop $R1
   Exch $R0
 FunctionEnd
