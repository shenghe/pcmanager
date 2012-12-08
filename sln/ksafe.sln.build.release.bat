@echo off

cd /d %~dp0
set BUILD_TARGET=%CD%\ksafe.sln

echo.
if ""=="%VS80COMNTOOLS%" (
   echo environment variable 'VS80COMNTOOLS%' has not been set
   goto Exit_Fail
)

set DEVENV=%VS80COMNTOOLS%\..\IDE\devenv.com
if not exist "%DEVENV%" (
   echo file "%DEVENV%" can not be found
   goto Exit_Fail
)

@echo on

"%DEVENV%" "%BUILD_TARGET%" /Build Release

:Exit_Fail

pause