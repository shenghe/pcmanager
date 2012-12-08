@rem           Down : OEM版本安装包
@rem        curpath : 根目录一级
@rem        tooldir : 制作安装包工具目录,默认使用2.30版本,如果是其它版本调用时加上版本号作为目录如：%tooldir%\2.37\makensis.exe
@rem     installdir : 制作安装包信息的目录
@rem         script : 安装包脚本文件
@rem   　 　　 file : 打包所需文件的存放目录
@rem        package : 安装包输出目录

@set curpath=%1
@set tooldir=%2
@set installdir=%curpath%\install

if {%curpath%} == { } goto nopath
if {%tooldir%} == { } goto notool

%tooldir%\2.45\makensis.exe /V1 %installdir%\leidian\setup_kisfree.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto kisguanwang
   ) else   (
   @goto failed
   )

:kisguanwang
%tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_kisguanwang.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto alading
   ) else   (
   @goto failed
   )

:alading
%tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_alading.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto osok
   ) else   (
   @goto failed
   )
      
:osok
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_osok.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto smart_slient
   ) else   (
   @goto failed
   )
   
:smart_slient
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_smart_slient.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto wd_web
   ) else   (
   @goto failed
   )
   
:wd_web
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_wd_web.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto smart
   ) else   (
   @goto failed
   )
   
:smart
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_smart.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto kis
   ) else   (
   @goto failed
   )
   
:kis
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_kis.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto wangdun
   ) else   (
   @goto failed
   )
:wangdun
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_wangdun.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto maxthon
   ) else   (
   @goto failed
   )
   
:maxthon
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_maxthon.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto beike
   ) else   (
   @goto failed
   )

:beike
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_beike.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto webmidpage
   ) else   (
   @goto failed
   )
   
:webmidpage
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_webmidpage.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto bsmidpage
   ) else   (
   @goto failed
   )
   
:bsmidpage
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_bsmidpage.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto ocer
   ) else   (
   @goto failed
   )
   
:ocer
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_ocer.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto founder
   ) else   (
   @goto failed
   )
   
:founder
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_founder.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto thunder_web
   ) else   (
   @goto failed
   )
   
:thunder_web
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_thunder_web.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto maxthonui
   ) else   (
   @goto failed
   )
   
:maxthonui
@rem %tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\setup_maxthonui.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto exit
   ) else   (
   @goto failed
   )

:nopath
echo WorkCopy path no Set
if Failed

:notool
echo MakeNsis Tool no set
if Failed
  
:failed
echo MakeSetupPackage Exec Failed
if Failed

:exit