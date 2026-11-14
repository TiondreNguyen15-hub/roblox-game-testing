@echo off
powershell.exe -STA -NoProfile -ExecutionPolicy Bypass -File "%~dp0Update-And-Enable-GPU.ps1" %*
set "gpuUpdateExit=%ERRORLEVEL%"
echo.
pause
exit /b %gpuUpdateExit%
