; Copyright (c) 2013 ASMlover. All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
;
;  * Redistributions of source code must retain the above copyright
;    notice, this list ofconditions and the following disclaimer.
;
;  * Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in
;    the documentation and/or other materialsprovided with the
;    distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
; LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
; FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
; COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
; BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
; LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
; ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.

  org   07c00h
  mov   ax, cs
  mov   ds, ax
  mov   es, ax
  call  display_str
  jmp   $                       ; infinite loop
display_str:
  mov   ax, boot_msg
  mov   bp, ax                  ; es:bp = address of string
  mov   cx, 16                  ; cx = length of string
  mov   ax, 01301h              ; ah = 13, al = 01 
  mov   bx, 000ch               ; page number is 0(bh=0), background is 
                                ; black, foreground string is red(bl=0ch) 
  mov   dl, 0
  int   10h                     ; interrupt 0x10
  ret
boot_msg:             db  "Hello, OS world!"
times   510 - ($-$$)  db  0
dw      0xaa55                  ; flag of end
