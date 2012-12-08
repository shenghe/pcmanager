@rem           Down : 官方版安装包
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

%tooldir%\2.45\makensis.exe /V1 %installdir%\leidian\Setup.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto dubaweb
   ) else   (
   @goto failed
   )
   
:dubaweb
%tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\Setup_dubaweb.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto zhuansha
   ) else   (
   @goto failed
   )
   
:zhuansha
%tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\Setup_zhuansha.nsi
@if %ERRORLEVEL% EQU 0 (
   @goto update
   ) else   (
   @goto failed
   )
   
:update
%tooldir%\2.45\makensis.exe  /V1 %installdir%\leidian\update_setup.nsi
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