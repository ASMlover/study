" configure for vim-marching 
" this plugin needs vimproc.vim
let g:marching_clang_command = "clang"
let g:marching#clang_command#options = {
  \ "cpp" : "-std=gnu++1y"
  \}
let g:marching_include_paths = [
  \ "C:/MinGW/lib/gcc/mingw32/4.8.1/include",
  \ "C:/MinGW/lib/gcc/mingw32/4.8.1/include/c++"
  \]
let g:marching_enable_neocomplete = 1
if !exists('g:neocomplete#force_omni_input_patterns')
  let g:neocomplete#force_omni_input_patterns = {}
endif
let g:neocomplete#force_omni_input_patterns.cpp = '[^.[:digit:] *\t]\%(\.\|->\)\w*\|\h\w*::\w*'
set updatetime=200
imap <buffer> <C-x><C-o> <Plug>(marching_start_omni_complete)
imap <buffer> <C-x><C-x><C-o> <Plug>(marching_force_start_omni_complete)
