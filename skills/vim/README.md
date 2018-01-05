# **README for vim**
***

## **说明**
    主要记录vim的相关操作技巧以及环境等...
    vim-demo是一个终端下的vim配置例子
    win-vim主要记录了windows下面gvim的配置信息

## **备份**
  * Windows => (kuaipan)
  * Linux(Ubuntu) => vim.tar.gz

## **将vim中关键字设置成非粗体模式**
    需要修改colorscheme对应的主题模式中的代码;
    比如修改desert.vim
    在文件的最后添加上:
    hi Type gui=none
    hi Statement gui=none

## **Vim退格建失效的解决方案**
    具体请参见VIM帮助 :h bs

    backspace的取值如下:
    indent  允许在自动缩进上退格
    eol     允许在换行符上退格(连接行)
    start   允许在插入开始的位置上退格;C-W/C-U到达插入开始的位置时停留一次
    如果backspace的取值为空, 表示和vi兼容

    为了和5.4以及更早的版本兼容
    0 => :set backspace=
    1 => :set backspace=indent,eol
    2 => :set backspace=indent,eol,start

    所以解决方法就是在.vimrc文件中加入如下代码:
    set backspace=indent,eol,start

## **Vim文件加入BOM头**
    1. 查看是否有BOM头
       : set bomb?
       显示nobomb表示无BOM头,
       现实bomb表示有BOM头
    2. 设置BOM头
       : set bomb

## **Vim获取文件名字及时间**
    获取当前的文件名字:
      expand("%:t")
    获取当前的时间:
      strftime("%y/%m:%d %H:%M:%S")

## **Vim使用"alt + 左右键"来移动标签**
    nn <silent> <M-left> :if tabpagenr() == 1\|exe "tabm ".tabpagenr("$")\|el\|exe "tabm ".(tabpagenr() - 2)\|en<CR>
    nn <silent> <M-right> :if tabpagenr() == tabpagenr("$")\|tabm 0\|el\|exe "tabm ".tabpagenr()\|en<CR>

## **Windows下将文件拖入vim的新标签页**
> ### **1. 新建立一个bat**
        @echo off
        start /B gvim -p --remote-tab-silent "%~f1"
        exit
> ### **2. Linux下面**
    在.bashrc文件中添加
    alias vim='gvim --remote-tab-silent'
    alias gvim='gvim --remote-tab-silent'
> ### **3. 修改注册表**
    1) 双击时打开标签页
        修改/HKEY_CLASSES_ROOT/Applications/gvim.exe/shell/edit/command的值
        D:\Tools\gvim\vim73\gvim.exe -p --remote-tab-silent "%1"
    2) 鼠标右键时打开标签页
        添加注册表
        [HKEY_CLASSES_ROOT\*\]下添加Shell项
        [HKEY_CLASSES_ROOT\*\Shell]添加Edit with &Vim项
        [HKEY_CLASSES_ROOT\*\Shell\Edit with &Vim]添加Command项
        [HKEY_CLASSES_ROOT\*\Shell\Edit with &Vim\Command]值修改为:
        "D:\Tools\gvim\vim73\gvim.exe" -p --remote-tab-silent "%1"

## **VIM恢复文件关闭之前的光标位置**
    在.vimrc文件中添加
    if has("autocmd")
      au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif
    endif

## **Ubuntu Install YouCompleteMe**
    $ sudo apt-get install build-essential cmake
    $ sudo apt-get install python-dev
    $ cd ~/.vim/bundle/YouCompleteMe/
    $ ./install.sh --clang-completer

## **YouCompleteMe in Windows**
  * Python2.7+
  * x64
  * Vim74 (support python 2.x)
  * Demo `Vim74.zip`

## **Vim 代码折叠**
    启用缩进折叠, 所有文本将按照缩进层次自动折叠:
        :set foldmethod=indent
    使用zm可手动折叠缩进, zr打开折叠缩进;
    指定缩进折叠的级别:
        :set foldlevel=0
    折叠命令:
      * zc 关闭当前打开的折叠
      * zo 打开当前的折叠
      * zm 关闭所有折叠
      * zM 关闭所有折叠及其嵌套折叠
      * zr 打开所有折叠
      * zR 打开所有折叠及其嵌套折叠
      * zd 删除当前折叠
      * zE 删除所有折叠
      * zj 移动至下一个折叠
      * zk 移动值上一个折叠
      * zn 禁用折叠
      * zN 启用折叠

## **针对NERDTree和Ag插件映射快捷键**
    " keyboard shortcuts
    let mapleader=','
    nnoremap <leader>a :Ag<space>
    nnoremap <silent> <leader>d :NERDTreeToggle<CR>
    nnoremap <silent> <leader>f :NERDTreeFind<CR>
    nnoremap <silent> <leader>] :TagbarToggle<CR>

## **常用设置**
 * `set autochdir` 自动转换到打开文件所在目录
 * `set autoread` 自动读取修改了的文件
 * `set autowrite` 自动保存文件(编译或切换文件)

## **配置vim-marching**
> ### **For Windows**
 * 下载[MinGW](http://cznic.dl.sourceforge.net/project/mingw/Installer/mingw-get-setup.exe)
 * 安装MinGW(gcc, g++, pthread-w32, make ...)
 * 到[LLVM](https://llvm.org/builds)下载CLANG Windows Installer
 * 安装LLVM
 * 然后根据[vim-marching](https://github.com/osyo-manga/vim-marching)来配置
 * vim-marching的运行需要[vimproc.vim](https://github.com/Shougo/vimproc.vim)
 * 编译vimproc.vim

## **远程Terminal下Powerline Font**
> ### **Font的安装**
    $ git clone https://github.com/eugeneching/consolas-powerline-vim.git
    $ cd consolas-powerline-vim
    $ sudo cp *.ttf /usr/share/fonts/fonts
    $ sudo fc-cache -f -v
    * 在windows下将这些字体安装到系统Fonts中
    * 在远程Putty中将字体设置为Powerline Consolas，编码设置为UTF-8
> ### **Vim配置**
    let g:airline_powerline_fonts=1
    set guifont=PowerlineSymbols\ for\ Powerline
    let g:Powerline_symbols='fancy'

## **Windows下编译vim**
  * 下载最新的vim源代码(这里以vim74为例子)
  * 进入vim目录(vim\src)
    - 首先修改bigvim.bat(32位平台)或bigvim64.bat(64位平台)
    - 这里我的例子(32-bit/64-bit), 然后使用`VS2015 x64 本机工具命令提示符`工具进入vim\src目录:
```shell
    :: 下面的环境设置假设了编译器安装地址(请根据自己环境进行设置)

    :: 设置VC2015 x86编译环境
    \> D:\Tools\MSVC2015\VC\vcvarsall.bat x86
    :: 设置VC2017 x86编译环境
    \> D:\Tools\VS2017\VC\Auxiliary\Build\vcvarsall.bat x86

    :: 设置VC2015 x64编译环境
    \> D:\Tools\MSVC2015\VC\vcvarsall.bat amd64
    :: 设置VC2017 x64编译环境
    \> D:\Tools\VS2017\VC\Auxiliary\Build\vcvarsall.bat amd64
```
```shell
:: for x86
:: command to build big Vim with OLE, Perl, Python, Ruby and Tcl
SET TOOLDIR=D:\Tools\
nmake -f Make_mvc.mak SDK_INCLUDE_DIR="C:\Program Files\Microsoft SDKs\Windows\v7.1A\Include" GUI=yes OLE=yes PYTHON3=%TOOLDIR%python35 DYNAMIC_PYTHON3=yes PYTHON3_VER=35 %1 IME=yes CSCOPE=yes

:: build for vim.exe
:: nmake -f Make_mvc.mak SDK_INCLUDE_DIR="C:\Program Files\Microsoft SDKs\Windows\v7.1A\Include" OLE=yes PYTHON3=%TOOLDIR%python35 DYNAMIC_PYTHON3=yes PYTHON3_VER=35 %1 IME=yes CSCOPE=yes
```

```shell
:: for x64
:: command to build big Vim with OLE, Perl, Python, Ruby and Tcl
SET TOOLDIR=D:\Tools\
nmake -f Make_mvc.mak SDK_INCLUDE_DIR="C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include" CPU=AMD64 GUI=yes OLE=yes PYTHON3=%TOOLDIR%python35 DYNAMIC_PYTHON3=yes PYTHON3_VER=35 %1 IME=yes CSCOPE=yes

:: build for vim.exe
:: nmake -f Make_mvc.mak SDK_INCLUDE_DIR="C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include" CPU=AMD64 OLE=yes PYTHON3=%TOOLDIR%python35 DYNAMIC_PYTHON3=yes PYTHON3_VER=35 %1 IME=yes CSCOPE=yes
```

        \> cd vim\src
        \> bigvim.bat

  * 在自己的目标安装目录创建[targetdir]\vim，[targetdir]\vim\vim74
  * 将vim源码目录下的runtime下的所有东西拷贝到[targetdir]\vim\vim74
  * 将vim\src中编译出来的exe以及dll拷贝到[targetdir]\vim\vim74

        \> copy src\*.exe [targetdir]\vim\vim74
        \> copy src\GvimExt\gvimext.dll [targetdir]\vim\vim74
        \> copy src\xxd\xxd.exe [targetdir]\vim\vim74

  * 在[targetdir]\vim目录下创建默认的vimfiles目录和`_vimrc`


## **Windows下更新编译YouCompleteMe**
  * 更新YouCompleteMe以及submodules
  * 更新Vim的Plug（PlugUpdate）
  * 重新编译`ycm_core`
```shell
  \> cd ~/.vim/plugged/YouCompleteMe
  \> git pull
  \> git submodule update --init --recursive
  \> call gvim -u "%HOME%\.vim\evil-vimrc\evil-install.vim" +PlugUpdate
  \> python install.py --clang --completer
```
