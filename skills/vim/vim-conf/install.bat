@echo off

REM Copyright (c) 2014 ASMlover. All rights reserved.
REM
REM Redistribution and use in source and binary forms, with or without
REM modification, are permitted provided that the following conditions
REM are met:
REM
REM  * Redistributions of source code must retain the above copyright
REM    notice, this list ofconditions and the following disclaimer.
REM
REM  * Redistributions in binary form must reproduce the above copyright
REM    notice, this list of conditions and the following disclaimer in
REM    the documentation and/or other materialsprovided with the
REM    distribution.
REM
REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
REM "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
REM LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
REM FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
REM COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
REM INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
REM BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
REM LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
REM CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
REM LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
REM ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
REM POSSIBILITY OF SUCH DAMAGE.

@if not exist "%HOME%" @set HOME=%HOMEDRIVE%%HOMEPATH%
@if not exist "%HOME%" @set HOME=%USERPROFILE%

@set APP_DIR=%cd%
if not exist "%APP_DIR%" @set APP_DIR=%HOME%\vim-conf
if not exist "%APP_DIR%" (
  call git clone https://github.com/ASMlover/vim-conf.git "%APP_DIR%" 
) else (
  @set ORIGINAL_DIR=%cd%
  echo updating vim-conf
  chdir /d "%APP_DIR%"
  call git pull
  chdir /d "%ORIGINAL_DIR%"
  call cd "%APP_DIR%"
)

call copy /Y vimrc "%VIMPATH%\_vimrc"
call copy /Y vimrc.bundles "%VIMPATH%"
call copy /Y vimrc.bundles.local "%VIMPATH%"
call copy /Y vimrc.local "%VIMPATH%"

if not exist "%VIMPATH%\vimfiles\bundle" (
  call mkdir "%VIMPATH\vimfiles\bundle%"
)

if not exist "%VIMPATH\vimfiles\bundle\Vundle.vim%" (
  call git clone https://github.com/gmarik/Vundle.vim.git "%VIMPATH\vimfiles\bundle\Vundle.vim%"
) else (
  call cd "%VIMPATH\vimfiles\bundle\Vundle.vim%"
  call git pull
  call cd %VIMPATH%
)

call gvim +BundleInstall! +BundleClean +qall

@echo on
