@echo off
setlocal
cd /d "%~dp0"

set ACTION=%1
if "%ACTION%"=="" (
  echo Usage: run.bat ^<start^|stop^|restart^|status^|logs^>
  exit /b 1
)
shift

if /I "%ACTION%"=="start" (
  powershell -ExecutionPolicy Bypass -File "scripts\\start_server.ps1" %*
  exit /b %ERRORLEVEL%
)
if /I "%ACTION%"=="stop" (
  powershell -ExecutionPolicy Bypass -File "scripts\\stop_server.ps1" %*
  exit /b %ERRORLEVEL%
)
if /I "%ACTION%"=="restart" (
  powershell -ExecutionPolicy Bypass -File "scripts\\restart_server.ps1" %*
  exit /b %ERRORLEVEL%
)
if /I "%ACTION%"=="status" (
  powershell -ExecutionPolicy Bypass -File "scripts\\status_server.ps1" %*
  exit /b %ERRORLEVEL%
)
if /I "%ACTION%"=="logs" (
  powershell -ExecutionPolicy Bypass -File "scripts\\logs_server.ps1" %*
  exit /b %ERRORLEVEL%
)

echo Unknown action: %ACTION%
echo Usage: run.bat ^<start^|stop^|restart^|status^|logs^>
exit /b 1
