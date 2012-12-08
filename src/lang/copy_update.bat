@echo off
setlocal
set "batpath=%~dp0"
set "srcpath=%~f1"
set "svnpath=%~f2"
set "current_dir=%cd%"
set "cfgpath=%~f3"
set updatecount=0

if not defined srcpath goto Error
if not defined svnpath goto Error

svn up %svnpath%
if not errorlevel 0 echo ------------更新代码库出错----------------------- & goto Error

for /F "tokens=1,2 delims=, " %%i in (update.txt) do if exist %srcpath%\%%i copy %srcpath%\%%i %svnpath%\%%j /y & echo --更新文件%%i-- & set /a updatecount=updatecount+1

svn ci %svnpath% -m "更新升级数据"
if not errorlevel 0 echo ------------提交数据出错----------------------- & goto Error

goto Exit

:Error
echo ---------------------something error--------------------------------
goto Finish

:Exit
echo --------------------------------------------------------------------------
echo 更新完成，更新文件%updatecount%个
echo --------------------------------------------------------------------------

:Finish
