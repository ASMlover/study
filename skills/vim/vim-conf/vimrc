" Copyright (c) 2014 ASMlover. All rights reserved.
"
" Redistribution and use in source and binary forms, with or without
" modification, are permitted provided that the following conditions
" are met:
"
"  * Redistributions of source code must retain the above copyright
"    notice, this list ofconditions and the following disclaimer.
"
"  * Redistributions in binary form must reproduce the above copyright
"    notice, this list of conditions and the following disclaimer in
"    the documentation and/or other materialsprovided with the
"    distribution.
"
" THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
" "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
" LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
" FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
" COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
" INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
" BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
" LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
" CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
" ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
" POSSIBILITY OF SUCH DAMAGE.

" do not bother with vi compatibility
set nocompatible                                             " must be first line

" configure vim
silent func! OSX()
  return has("macunix")
endfunction
silent func! LINUX()
  return (has("unix") && !has("macunix") && !has("win32unix"))
endfunction
silent func! WINDOWS()
  return (has("win32") || has("win64"))
endfunction

if !WINDOWS()
  set shell=/bin/sh
endif

" on windows, we alse use '.vim' instead of 'vimfiles'; this make
" synchronization across (heterogeneous) system easier
if WINDOWS()
  set runtimepath=$HOME/.vim,$VIM/vimfiles,$VIMRUNTIME
endif

" install vundle bundles
if filereadable(expand("~/.vim/vimrc.bundles"))
  source ~/.vim/vimrc.bundles
endif

" ensure ftdetect et al work by including this after the Vundle stuff
filetype plugin indent on
" enable syntax highlighting
syntax on

set autoindent
set autoread                                                 " reload files when changed on disk, i.e. via `git checkout`
set backspace=2                                              " Fix broken backspace in some setups
set showcmd
set expandtab                                                " expand tabs to space
set tabstop=8                                                " actual tabs occupy 8 characters
set shiftwidth=2                                             " normal mode indentation commands use 4 spaces
set softtabstop=2                                            " insert mode tab and backspace use 2 spaces
set ignorecase                                               " case-insensitive search
set incsearch                                                " search as you type
set smartcase                                                " case-sensitive search if any caps
set fileencoding=utf-8                                       " default fileencoding is utf-8
set fileencodings=utf-8,ucs-bom,default,latin1
set fileformat=unix                                          " default fileformat is unix
set fileformats=unix,dos,mac
set nobackup                                                 " do not create backup file
set noundofile                                               " do not create undo file
set number                                                   " show line numbers
set ruler                                                    " show where you are
set laststatus=2                                             " always show statusline
set cursorline                                               " show cursor of current line
set autochdir

colorscheme molokai
set t_Co=256
set background=dark
set guifont=Consolas:h11
if WINDOWS()
  au GUIEnter * simalt ~x
endif

" remember the location of last time shut off
if has("autocmd")
  au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif
endif

" keyboard shortcuts
let mapleader = ','
nnoremap <leader>l :Align
nnoremap <leader>a :Ag<space>
nnoremap <silent> <leader>d :NERDTreeToggle<CR>
nnoremap <silent> <leader>f :NERDTreeFind<CR>
nnoremap <silent> <leader>] :TagbarToggle<CR>
nnoremap <silent> <leader>p :CtrlP<CR>
nnoremap <silent> <leader>P :CtrlPClearCache<CR>:CtrlP<CR>
" key-mapping for SingleCompile(compile or run a single source file)
nnoremap <silent> <F9> :SCCompile<CR>
nnoremap <silent> <F10> :SCCompileRun<CR>

" configure for bundles plugins
" neocomplete configure
if filereadable(expand("~/.vim/bundles.vimrc/neocomplete.vimrc"))
  source ~/.vim/bundles.vimrc/neocomplete.vimrc
endif
" vim-marching configure
if filereadable(expand("~/.vim/bundles.vimrc/vim-marching.vimrc"))
  source ~/.vim/bundles.vimrc/vim-marching.vimrc
endif

" source local vimrc
if filereadable(expand("~/.vim/vimrc.local"))
  source ~/.vim/vimrc.local
endif
