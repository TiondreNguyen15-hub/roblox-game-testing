@echo off
powershell.exe -STA -NoProfile -ExecutionPolicy Bypass -File "%~dp0Update-Game-ESP.ps1"
set "gameEspUpdateExit=%ERRORLEVEL%"
echo.
pause
exit /b %gameEspUpdateExit%
