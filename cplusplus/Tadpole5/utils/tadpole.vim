" Copyright (c) 2021 ASMlover. All rights reserved.
"
"  _____         _             _
" |_   _|_ _  __| |_ __   ___ | | ___
"   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
"   | | (_| | (_| | |_) | (_) | |  __/
"   |_|\__,_|\__,_| .__/ \___/|_|\___|
"                 |_|
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

if exists("b:current_syntax")
  finish
endif

syn keyword tadpoleNil  nil
syn keyword tadpoleBoolean  true false
syn match tadpoleNumeric  "\v\-?\d*(\.\d+)?"
syn region tadpoleString  start="\v\"" end="\v\""
syn keyword tadpoleKeyword  fn var
syn keyword tadpoleStatement  fn nextgroup=tadpoleFunction skipwhite

syn match tadpoleOperator "\v\+"
syn match tadpoleOperator "\v\-"
syn match tadpoleOperator "\v\*"
syn match tadpoleOperator "\v/"
syn match tadpoleOperator "\v\="

syn match tadpoleFunction "\h\w*" display contained
syn region tadpoleComment  start="//" end="$" contains=tadpoleTodo,@Spell
syn keyword tadpoleTodo FIXME NOTE TODO XXX contained

syn sync match pythonSync grouphere NONE "^\%(fn\|class\)\s\+\h\w*\s*[(:]"

hi def link tadpoleStatement  Statement
hi def link tadpoleNil  Constant
hi def link tadpoleBoolean  Boolean
hi def link tadpoleNumeric  Number
hi def link tadpoleString String
hi def link tadpoleKeyword  Keyword
hi def link tadpoleComment  Comment
hi def link tadpoleOperator Operator
hi def link tadpoleFunction Function
hi def link tadpoleTodo Todo

let b:current_syntax = "tadpole"
