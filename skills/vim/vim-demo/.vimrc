set noerrorbells        " 关闭错误系想你响铃

syntax enable           " 使用语法高亮
syntax on 
set fileformat=unix     " 文件使用unix风格
set fileencoding=utf-8  " 默认utf-8文件使用
set fileformats=unix,dos,mac
set fileencodings=utf-8,ucs-bom,default,latin1
filetype on
filetype indent on
filetype plugin on

set autoindent          " 自动缩进, 自动对齐
set shiftwidth=2        " 将tab键设置为空格替换
set softtabstop=2
set tabstop=2

:set tabstop=2
:set expandtab
:%retab

set nobackup            " 没有备份文件
set number              " 显示行号
set novisualbell        " 关闭可视响铃
set laststatus=2        " 总是显示状态

"colorscheme desert
colorscheme yerik_day

map <F3> :call TitleDescription(2)<cr>'s
map <F4> :call TitleDescription(0)<cr>'s
map <F5> :call TitleDescription(1)<cr>'s
function AddFileHeader4C()
  call append(0, "/*")
  call append(1, " * Copyright (c) ".strftime("%Y")." ASMlover. All rights reserved.")
  call append(2, " *")
  call append(3, " * Redistribution and use in source and binary forms, with or without")
  call append(4, " * modification, are permitted provided that the following conditions")
  call append(5, " * are met:")
  call append(6, " *")
  call append(7, " *  * Redistributions of source code must retain the above copyright")
  call append(8, " *    notice, this list ofconditions and the following disclaimer.")
  call append(9, " *")
  call append(10," *  * Redistributions in binary form must reproduce the above copyright")
  call append(11," *    notice, this list of conditions and the following disclaimer in")
  call append(12," *    the documentation and/or other materialsprovided with the")
  call append(13," *    distribution.")
  call append(14," *")
  call append(15," * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS")
  call append(16," * \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT")
  call append(17," * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS")
  call append(18," * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE")
  call append(19," * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,")
  call append(20," * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,")
  call append(21," * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;")
  call append(22," * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER")
  call append(23," * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT")
  call append(24," * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN")
  call append(25," * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE")
  call append(26," * POSSIBILITY OF SUCH DAMAGE.")
  call append(27," */")
  echohl WarningMsg | echo "Successful in adding the copyright." | echohl None
endfunction

" 更新最近修改时间和文件名
function UpdateTitle()
  normal m'
  execute '/# *Last modified:/s@:.*$@\=strftime(":\t%Y-%m-%d %H:%M")@'
  normal ''
  normal mk
  execute '/# *Filename:/s@:.*$@\=":\t\t".expand("%:t")@'
  execute "noh"
  normal 'k
  echohl WarningMsg | echo "Successful in updating the copy right." | echohl None
endfunction

function AddFileHeader4CPP()
  call append(0, "// Copyright (c) ".strftime("%Y")." ASMlover. All rights reserved.")
  call append(1, "//")
  call append(2, "// Redistribution and use in source and binary forms, with or without")
  call append(3, "// modification, are permitted provided that the following conditions")
  call append(4, "// are met:")
  call append(5, "//")
  call append(6, "//  * Redistributions of source code must retain the above copyright")
  call append(7, "//    notice, this list ofconditions and the following disclaimer.")
  call append(8, "//")
  call append(9, "//  * Redistributions in binary form must reproduce the above copyright")
  call append(10,"//    notice, this list of conditions and the following disclaimer in")
  call append(11,"//    the documentation and/or other materialsprovided with the")
  call append(12,"//    distribution.")
  call append(13,"//")
  call append(14,"// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS")
  call append(15,"// \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT")
  call append(16,"// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS")
  call append(17,"// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE")
  call append(18,"// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,")
  call append(19,"// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,")
  call append(20,"// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;")
  call append(21,"// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER")
  call append(22,"// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT")
  call append(23,"// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN")
  call append(24,"// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE")
  call append(25,"// POSSIBILITY OF SUCH DAMAGE.")
  echohl WarningMsg | echo "Successful in adding the copyright." | echohl None
endfunction

function AddFileHeader4Script()
  call append(0, "# Copyright (c) ".strftime("%Y")." ASMlover. All rights reserved.")
  call append(1, "#")
  call append(2, "# Redistribution and use in source and binary forms, with or without")
  call append(3, "# modification, are permitted provided that the following conditions")
  call append(4, "# are met:")
  call append(5, "#")
  call append(6, "#  * Redistributions of source code must retain the above copyright")
  call append(7, "#    notice, this list ofconditions and the following disclaimer.")
  call append(8, "#")
  call append(9, "#  * Redistributions in binary form must reproduce the above copyright")
  call append(10,"#    notice, this list of conditions and the following disclaimer in")
  call append(11,"#    the documentation and/or other materialsprovided with the")
  call append(12,"#    distribution.")
  call append(13,"#")
  call append(14,"# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS")
  call append(15,"# \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT")
  call append(16,"# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS")
  call append(17,"# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE")
  call append(18,"# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,")
  call append(19,"# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,")
  call append(20,"# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;")
  call append(21,"# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER")
  call append(22,"# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT")
  call append(23,"# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN")
  call append(24,"# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE")
  call append(25,"# POSSIBILITY OF SUCH DAMAGE.")
  echohl WarningMsg | echo "Successful in adding the copyright." | echohl None
endfunction

" 判断前10行代码里面, 是否有Copyright这个单词，
" 如果没有的话, 代表没有添加过作者信息, 需要新添加: 如果有的话, 那么只需要更新即可
" 0: default/C 
" 1: cpp
" 2: Script
function TitleDescription(type)
  let n=1
  " 默认为添加
  while n < 10
    let line = getline(n)
      if line =~ '^\#\s*\S*Copyright:\S*.*$'
        call UpdateTitle()
        return
      endif
      let n = n + 1
  endwhile
  
  if a:type == 0
    call AddFileHeader4C()
  elseif a:type == 1
    call AddFileHeader4CPP()
  elseif a:type == 2
    call AddFileHeader4Script()
  else
    call AddFileHeader4C()
  endif
endfunction

autocmd BufNewFile *.c call AddFileHeader4C()
autocmd BufNewFile *.cpp call AddFileHeader4CPP()
autocmd BufNewFile *.cc call AddFileHeader4CPP()
autocmd BufNewFile *.hpp call AddFileHeader4CPP()
autocmd BufNewFile *.py call AddFileHeader4CPP()
autocmd BufNewFile *.mk call AddFileHeader4CPP()


inoremap ( ()<ESC>i
inoremap ) <c-r>=ClosePair(')')<CR>
inoremap { {}<ESC>i
inoremap } <c-r>=ClosePair('}')<CR>
inoremap [ []<ESC>i
inoremap ] <c-r>=ClosePair(']')<CR>
inoremap < <><ESC>i
inoremap > <c-r>=ClosePair('>')<CR>


function ClosePair(char)
  if getline('.')[col('.') - 1] == a:char
    return "\<Right>"
  else
    return a:char
  endif
endfunction
