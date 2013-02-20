" Vim color file
"
" Maintainer:  night 
" Last Change: 2010-1-9

set background=light
hi clear
if exists("syntax_on")
	syntax reset
endif

let g:colors_name="yerik_day"


hi normal			guibg=white				guifg=black				gui=none
hi comment									guifg=SkyBlue			gui=none
hi comment    ctermfg=darkcyan
hi constant									guifg=magenta			gui=none
hi identifier								guifg=darkcyan			gui=none
hi statement								guifg=blue				gui=none
hi preproc									guifg=brown				gui=none
hi type										guifg=blue				gui=none
hi operator									guifg=magenta
hi special									guifg=orange			gui=none
hi error			guibg=red				guifg=white				gui=none
hi todo				guibg=#339900			guifg=white				gui=none

hi cursor			guibg=black				guifg=white				gui=none
hi cursorline		guibg=lightgreen								gui=none

hi diffadd			guibg=lightgreen								gui=none
hi diffchange		guibg=lightyellow		guifg=grey				gui=none
hi diffdelete								guifg=lightred			gui=none
hi difftext			guibg=yellow									gui=none

hi folded			guibg=lightgray			guifg=blue				gui=none
hi linenr			guibg=#555				guifg=lightblue			gui=none
hi statusline		guibg=#555				guifg=white				gui=none
hi statuslinenc		guibg=#555				guifg=darkgray			gui=none

hi incsearch		guibg=yellow									gui=none
hi search			guibg=yellow									gui=none
hi visual			guibg=grey										gui=none
hi matchparen		guibg=grey										gui=none

hi warningmsg								guifg=red				gui=none
hi errormsg			guibg=red				guifg=white				gui=none
hi modemsg									guifg=darkgray			gui=none
hi moremsg			guibg=#339900			guifg=white				gui=none

hi wildmenu			guibg=#339900			guifg=white				gui=none
hi pmenu			guibg=grey										gui=none
hi pmenusel			guibg=lightblue			guifg=white				gui=bold
hi pmenusbar		guibg=lightgray
hi pmenuthumb								guifg=black

hi nontext									guifg=darkgray			gui=none
hi specialkey								guifg=#eee				gui=none
hi title									guifg=blue				gui=none
hi question			guibg=#339900			guifg=white				gui=none
hi directory								guifg=blue				gui=none

