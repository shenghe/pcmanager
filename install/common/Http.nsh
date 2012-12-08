!macro InternetGetCookie HOST URI
  Internet::GetUrlCode ${HOST} ${URI} ${VAR_3}
!macroend

 !macro InternetGetCookie1 URL CookieName
 Push $0
  Push $1
  Push $2
  Push $3
  
  SetPluginUnload manual
  StrCpy $0 ""
  StrCpy $1 ""
  StrCpy $2 1020 # variables are limited to 1024 characters
  StrCpy $3 0
  
  System::Call "Wininet::InternetGetCookieEx(t, t, t, *i, i, i) i ('${URL}', '${CookieName}', r1r1, r2r2, 0, 0) .r3 ? u"
  System::Free 0
  
  ${if} $3 == 1        # return 1
    StrLen $2 ${CookieName}
    IntOp $2 $2 + 1
    StrCpy $0 $1 "" $2
  ${endif}

  Pop $3
  Pop $2
  Pop $1
  Exch $0
!macroend
