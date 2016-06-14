@echo off
echo [project-name] 删除分支工具

set branch_name=%1

rem 输入分支名称
:input
if "%branch_name%"=="" (
	set /p branch_name=请输入分支名：
	goto input
)

rem 确认
color fc
choice /M "将删除分支 %branch_name%，是否确定？" /c YN
if errorlevel 255 (
	echo 错误的输入!!
	goto end
) else if errorlevel 2 (
	rem no
	goto end
) else if errorlevel 1 (
	rem yes
) else if errorlevel 0 (
	rem Ctrl+C interruptted
	goto end
)

rem 再次确认
color ce
choice /M "慎重提醒!!!!!!!，将删除分支 %branch_name%，是否继续？" /c YN
if errorlevel 255 (
	echo 错误的输入!!
	goto end
) else if errorlevel 2 (
	rem no
	goto end
) else if errorlevel 1 (
	rem yes
) else if errorlevel 0 (
	rem Ctrl+C interruptted
	goto end
)
color 07

echo 正在删除分支 %branch_name% ...
set MESSAGE="#0 [project-name] delete branch"

set BRANCH_ROOT=https://[svn-project-path]/svn/virtual/branches

svn del %BRANCH_ROOT%/programer/%branch_name% -m %MESSAGE%
svn del %BRANCH_ROOT%/designer/%branch_name% -m %MESSAGE%

echo 完成.

:end
color 07
pause
