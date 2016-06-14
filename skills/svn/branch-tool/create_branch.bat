@echo off
echo [project-name] 创建分支工具

set branch_name=%1

rem 输入分支名称
:input
if "%branch_name%"=="" (
	set /p branch_name=请输入分支名：
	goto input
)

rem 再次确认
choice /M "将创建分支 %branch_name%，是否确定？" /c YN
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

echo 正在创建分支 %branch_name% ...
set MESSAGE="#0 [project-name] weekly branch"

set BRANCH_ROOT=https://[svn-project-path]/svn/virtual/branches

rem 策划分支
set designer_root=%BRANCH_ROOT%/designer/%branch_name%
svn copy --ignore-externals --parents https://[svn-project-path]/svn/[res-path] %designer_root%/[res-path] -m %MESSAGE%
svn copy --ignore-externals --parents https://[svn-project-path]/svn/[engine-path] %designer_root%/[engine-path] -m %MESSAGE%

rem 程序分支
set programer_root=%BRANCH_ROOT%/programer/%branch_name%
svn copy --ignore-externals --parents https://[svn-project-path]/svn/src/[src-path] %programer_root%/[src-path] -m %MESSAGE%

svn checkout --depth=empty %programer_root% tmp2
echo ../../designer/%branch_name%/[res-path] [res-path] > ext2.txt
echo ../../designer/%branch_name%/[engine-path] [engine-path] >> ext2.txt
svn propset svn:externals tmp2 -F ext2.txt
svn commit tmp2 -m %MESSAGE%
del ext2.txt
rd /s /q tmp2

echo 完成.

:end
pause
