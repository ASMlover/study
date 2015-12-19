@echo off

@if not exist "%HOME%" @set HOME=%HOMEDRIVE%%HOMEPATH%
@if not exist "%HOME%" @set HOME=%USERPROFILE%

call copy /Y gitconfig "%HOME%\.gitconfig"
call copy /Y mercurial.ini "%HOME%\mercurial.ini"

echo installing VCS-Conf successfully ...
pause
@echo on
