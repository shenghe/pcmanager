
Section "MainSection" SEC01

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
 
  KillProcDLL::KillProc "KClear.exe"
  !insertmacro FileEx "bin\KClear.exe" "$INSTDIR\KClear.exe" "$INSTDIR\"

  SetOverwrite ifnewer

  Push $1
  
  IfSilent label_quit
    WriteRegDWORD HKLM ${PRODUCT_ENG_MARK} ${PRODUCT_ENG_MARK_VALUE} 1
label_quit:

   Pop $1
SectionEnd

