@echo off
powershell.exe -STA -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-GPU.ps1"
set "gpuInstallExit=%ERRORLEVEL%"
echo.
pause
exit /b %gpuInstallExit%
