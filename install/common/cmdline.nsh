 ; load language from command line /L=1033
 ; foo.exe /S /L=1033 /D=C:\Program Files\Foo
 ; or:
 ; foo.exe /S "/L=1033" /D="C:\Program Files\Foo"
 ; gpv "/L=" "1033"
 
  Function StrStr
   Exch $R1 ; st=haystack,old$R1, $R1=needle
   Exch    ; st=old$R1,haystack
   Exch $R2 ; st=old$R1,old$R2, $R2=haystack
   Push $R3
   Push $R4
   Push $R5
   StrLen $R3 $R1
   StrCpy $R4 0
   ; $R1=needle
   ; $R2=haystack
   ; $R3=len(needle)
   ; $R4=cnt
   ; $R5=tmp
   loop:
     StrCpy $R5 $R2 $R3 $R4
     StrCmp $R5 $R1 done
     StrCmp $R5 "" done
     IntOp $R4 $R4 + 1
     Goto loop
 done:
   StrCpy $R1 $R2 "" $R4
   Pop $R5
   Pop $R4
   Pop $R3
   Pop $R2
   Exch $R1
 FunctionEnd

 
 !macro GETPARAMETERVALUE SWITCH
   Push $0
   Push $1
   Push $2
   Push $3
   Push $4

 ;$CMDLINE='"My Setup\Setup.exe" /L=1033 /S'
   Push "$CMDLINE"
   Push '${SWITCH}"'
   Call StrStr
   Pop $0

   ${if} "$0" != ""
       ;$0='/L="1033" /S'
        StrLen $2 "$0"
        Strlen $1 "${SWITCH}"
        IntOp $1 $1 + 1
        StrCpy $0 "$0" $2 $1
       ;$0='1033" /S'
        Push "$0"
        Push '"'
        Call StrStr
        Pop $1
        StrLen $2 "$0"
        StrLen $3 "$1"
        IntOp $4 $2 - $3
        StrCpy $0 $0 $4 0
   ${else}
        Push "$CMDLINE"
        Push "${SWITCH}"
        call StrStr
        Pop $0
   
        ${if} "$0" != ""
         ;$0='/L="1033" /S'
           StrLen $2 "$0"
           Strlen $1 "${SWITCH}"
           StrCpy $0 "$0" $2 $1
          ;$0=1033 /S'
           Push "$0"
           Push ' '
           Call StrStr
           Pop $1
           StrLen $2 "$0"
           StrLen $3 "$1"
           IntOp $4 $2 - $3
           StrCpy $0 $0 $4 0
        ${endif}
   ${endif}

   Pop $4
   Pop $3
   Pop $2
   Pop $1
   Exch $0
 !macroend
