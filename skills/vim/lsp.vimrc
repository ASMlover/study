syntax on

set autoindent
set autoread
set backspace=2
set showcmd
set smarttab
set noexpandtab
set tabstop=4
set shiftwidth=4
set softtabstop=4
set ignorecase
set incsearch
set smartcase
set smartindent
set showmatch
set encoding=utf-8
set fileencoding=utf-8
set fileencodings=utf-8,ucs-bom,default,latin1
set fileformat=unix
set fileformats=unix,dos,mac
set nobackup
set noundofile
set number
set ruler
set hlsearch
set laststatus=2
set cursorline
set listchars=tab:>-,trail:-
set colorcolumn=120
set list!

set t_Co=256
set termguicolors
set background=dark

if &diff
  colorscheme evening
endif

au! FileType python setlocal tabstop=4 shiftwidth=4 softtabstop=4 noexpandtab

" automatically removing all trailing whitespace
autocmd! BufWritePre * :%s/\s\+$//e

" region coc.nvim 自动补全
call plug#begin('~/.vim/plugged')
  Plug 'morhetz/gruvbox'
  Plug 'neoclide/coc.nvim', {'branch': 'release'}
call plug#end()

filetype plugin indent on
inoremap <expr> <Tab> pumvisible() ? "\<C-y>" : "\<Tab>"
" endregion coc.nvim 自动补全

colorscheme gruvbox
