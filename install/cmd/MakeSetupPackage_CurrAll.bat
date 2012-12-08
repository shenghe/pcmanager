@rem      CurrAll : 制作当前需要的安装包
@rem           %1 : 根目录一级
@rem           %2 : 制作安装包工具
set WorkDir=%1
set NsisTool=%2
set BatPath=%WorkDir%\install\cmd

call %BatPath%\MakeSetupPackage.bat %WorkDir% %NsisTool%
call %BatPath%\MakeSetupPackage_OEM.bat %WorkDir% %NsisTool%