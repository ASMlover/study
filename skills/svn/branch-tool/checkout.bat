@echo off
echo [project-name] 签出切换分支代码工具

set branch_name=%1
set BRANCH_ROOT=https://[svn-project-path]/svn/virtual/branches

rem 输入分支名称
:input
if "%branch_name%"=="" (
	set /p branch_name=请输入分支名：
	goto input
)

rem 输入角色类型
choice /M "您是一个开发者(P)还是一个策划(D)？" /c PD
if errorlevel 255 (
	echo 错误的输入!!
	goto end
) else if errorlevel 2 (
	rem (D)esigner
	set branch_path=%BRANCH_ROOT%/designer/%branch_name%
) else if errorlevel 1 (
	rem (P)rogramer
	set branch_path=%BRANCH_ROOT%/programer/%branch_name%
) else if errorlevel 0 (
	rem Ctrl+C interruptted
	goto end
)

set SUBDIR=[project-name]_branch
if exist %SUBDIR% (
	rem 切换分支
	svn switch --ignore-ancestry %branch_path% %SUBDIR%
) else (
	rem 检出分支
	svn co %branch_path% %SUBDIR%
)

:end
pause
