!include "FileFunc.nsh"

!insertmacro GetParameters

OutFile "getver.exe"
SilentInstall silent
 
RequestExecutionLevel user
 
Section

    ${GetParameters} $R6
 
    ## Get file version
    GetDllVersion "$R6" $R0 $R1
    IntOp $R2 $R0 / 0x00010000
    IntOp $R3 $R0 & 0x0000FFFF
    IntOp $R4 $R1 / 0x00010000
    IntOp $R5 $R1 & 0x0000FFFF
    StrCpy $R1 "$R2.$R3.$R4.$R5"
 
    ## Write it to a !define for use in main script
    FileOpen  $R0 '$EXEDIR\version.inc.nsi' w
    FileWrite $R0 '!define AUTO_FILE_VERSION "$R1"'
    FileClose $R0
 
SectionEnd
