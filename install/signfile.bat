set sign_tool="d:\Program Files\Microsoft SDKs\Windows\v6.1\Bin\signtool.exe"
set cer_path="E:\sign\ks.cert.cer"
%sign_tool% sign /v /ac %cer_path% /s MY /n "Kingsoft Security Co.,Ltd" /d "Ω…ΩŒ¿ ø" /t "http://timestamp.verisign.com/scripts/timstamp.dll" %1