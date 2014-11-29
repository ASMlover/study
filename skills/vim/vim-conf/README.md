# **vim-conf**
***
  
  Configure files for Vim.


# **Usage**
  * `,d` brings up [NERDTree](https://github.com/scrooloose/nerdtree), a sidebar buffer for navigating and manipulating files
  * `,a` starts project search with [ag.vim](https://github.com/rking/ag.vim) using [the silver searcher](https://github.com/ggreer/the_silver_searcher)(like ack, but faster)
  * `,l` begins aligning lines on a string, usually used as `,l=` to align assignments
  * `,]` brings up [tagbar](https://github.com/majutsushi/tagbar), a sidebar buffer for displaying tags
  * `,p` brings up [ctrlp.vim](https://github.com/kien/ctrlp.vim), a project file filter for easily opening specific files
  * `F9` compile a single source file(C/C++)
  * `F10` compile and run a single source file(C/C++)
  * `F12` making the GVIM window transparent(just for windows platform)


# **Install**
## **Windows**
  On Windows [Git](http://msysgit.github.io/), [MinGW](http://www.mingw.org/) and [Clang](http://www.llvm.org/) are required. Also, if you haven't already, you will need to install [Vim](http://www.vim.org/).  
  Requires Vim7.3.885+ compiled with [if_lua](http://vimdoc.sourceforge.net/htmldoc/if_lua.html). If :echo has("lua") returns 1, then you're done; otherwise, see below.

  * **Installing dependencies**
    - [Vim 32bit](http://files.kaoriya.net/vim/2013/vim73-kaoriya-win32-20130706.zip)
    - [Vim 64bit](http://files.kaoriya.net/vim/2013/vim73-kaoriya-win64-20130706.zip)
    - [Vim 32bit alternative](https://tuxproject.de/projects/vim/)
    - [Another alternative](http://wyw.dcweb.cn/#download)
    - Install [MinGW](http://www.mingw.org/) to C:\
    - Install [Clang](http://www.llvm.org/)

    **Note:** The Vim build may not include the Lua DLL. In that case, [download lua](http://lua-users.org/wiki/LuaBinaries) and put the `lua52.dll` file in the same directory as `gvim.exe`.  
    **Note:** After installing Vim, you need to add it to your environment variable path. Just like:  

        \> echo %VIMRUNTIME%
        \> D:\Vim\vim74

  * **Installing vim-conf**
    
        \> git clone https://github.com/ASMlover/vim-conf.git
        \> install.bat

  * **Building vimproc.vim**

        \> cd %HOME%\.vim\bundle\vimproc.vim
        \> make -f make_mingw32.mak
