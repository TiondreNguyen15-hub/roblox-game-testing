@echo off
powershell.exe -STA -NoProfile -ExecutionPolicy Bypass -File "%~dp0Update-HUD.ps1"
set "hudUpdateExit=%ERRORLEVEL%"
echo.
pause
exit /b %hudUpdateExit%
