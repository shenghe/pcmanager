
Section "MainSection" SEC01

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
 
  !insertmacro FileEx "bin\ksafeave.dll" "$INSTDIR\ksafeave.dll" "$INSTDIR\"
  !insertmacro FileEx "bin\ksignup.dll"  "$INSTDIR\ksignup.dll"  "$INSTDIR\"
  !insertmacro FileEx "bin\ksgmerge.dll" "$INSTDIR\ksgmerge.dll" "$INSTDIR\"

  SetOverwrite try
  File "bin\signs.ini"
  File "bin\vinfo.ini"

  Push $1
  
  SetOverwrite try

  StrCpy  $1 "$INSTDIR\ksg"
  SetOutPath  $1
  File /r "bin\ksg\*.*"
  
  StrCpy  $1 "$INSTDIR\kae"
  SetOutPath  $1
  File /r "bin\kae\*.*"
  
  SetOverWrite ifnewer
  SetOutPath "$INSTDIR"
  

  IfSilent label_quit
    WriteRegDWORD HKLM ${PRODUCT_ENG_MARK} ${PRODUCT_ENG_MARK_VALUE} 1
    ;他娘的，先提权
    KillProcDLL::KillProc "??*****"
    FindProcDLL::FindProc "KSafeSvc.exe"
    ${if} $R0 == 1
        Call GetInstalledLeidianPath
        Exec '"$0\KSafeSvc.exe" -instkeng'
    ${endif}
  
label_quit:
   Pop $1
SectionEnd

