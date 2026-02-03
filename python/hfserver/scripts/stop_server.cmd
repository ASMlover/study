@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0stop_server.ps1" %*
