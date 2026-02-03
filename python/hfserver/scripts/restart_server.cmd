@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0restart_server.ps1" %*
