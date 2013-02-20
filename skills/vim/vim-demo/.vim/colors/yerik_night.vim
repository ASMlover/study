" vim color file
" maintainer:	night
" last change:	2010-1-9

hi clear
set background=dark
if exists("syntax_on")
	syntax reset
endif
let g:colors_name = "yerik_night"

hi normal		ctermbg=black		ctermfg=green		guibg=black			guifg=#55ff55
hi comment							ctermfg=grey							guifg=grey
hi constant							ctermfg=magenta							guifg=orange		gui=none
hi identifier						ctermfg=cyan							guifg=darkcyan
hi operator							ctermfg=blue							guifg=lightblue
hi preproc							ctermfg=brown							guifg=brown
hi special							ctermfg=magenta							guifg=orange
hi statement						ctermfg=yellow							guifg=yellow		gui=none
hi type								ctermfg=yellow							guifg=yellow		gui=none
hi todo			ctermbg=yellow		ctermfg=blue		guibg=yellow		guifg=blue
hi error		ctermbg=red			ctermfg=white		guibg=red			guifg=white

hi cursor		ctermbg=green		ctermfg=black		guibg=fg			guifg=bg
hi cursorline											guibg=#333

hi diffadd		ctermbg=blue							guibg=darkblue
hi diffchange	ctermbg=magenta							guibg=darkmagenta
hi diffdelete						ctermfg=red								guifg=lightred		gui=none
hi difftext		ctermbg=red								guibg=red

hi matchparen	ctermbg=cyan							guibg=darkcyan
hi search		ctermbg=green		ctermfg=white		guibg=darkgreen		guifg=white			
hi incsearch	ctermbg=white		ctermfg=green		guibg=darkgreen		guifg=white			gui=none
hi visual		ctermbg=white		ctermfg=green		guibg=darkgreen		guifg=white			gui=none

hi warningmsg						ctermfg=red								guifg=red
hi errormsg		ctermbg=red			ctermfg=white		guibg=red			guifg=white
hi modemsg		cterm=bold																		gui=bold
hi moremsg		cterm=bold			ctermfg=green							guifg=green			gui=bold

hi title							ctermfg=red								guifg=darkred		gui=bold
hi question							ctermfg=cyan							guifg=cyan			gui=bold
hi nontext							ctermfg=blue							guifg=blue			gui=bold
hi specialkey						ctermfg=gray							guifg=#222
hi directory						ctermfg=cyan							guifg=cyan

hi folded							ctermfg=cyan							guifg=darkcyan		gui=none
hi linenr		ctermbg=gray		ctermfg=black		guibg=#333			guifg=lightblue
hi statusline	ctermbg=black		ctermfg=gray		guibg=#555			guifg=white			gui=none
hi statuslinenc ctermbg=gray		ctermfg=gray		guibg=#555			guifg=darkgray		gui=none
hi wildmenu		ctermbg=brown		ctermfg=black		guibg=yellow		guifg=black

hi pmenu		ctermbg=yellow		ctermfg=black		guibg=#333			guifg=lightblue		gui=none
hi pmenusel		ctermbg=gray		ctermfg=black		guibg=lightblue		guifg=#333			gui=bold
hi pmenusbar	ctermbg=yellow							guibg=#555
hi pmenuthumb						ctermfg=gray							guifg=lightblue
