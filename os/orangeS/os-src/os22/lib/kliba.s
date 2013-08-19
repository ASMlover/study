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


[section .data]
disp_pos  dd  0

[section .text]
; export functions 
global  display_str
global  display_color_str 
global  out_byte
global  in_byte




; void display_str(char* str);
display_str:
  push  ebp 
  mov   ebp, esp

  mov   esi, [ebp + 8]
  mov   edi, [disp_pos]
  mov   ah, 0fh
.1:
  lodsb
  test  al, al 
  jz    .2
  cmp   al, 0ah
  jnz   .3
  push  eax 
  mov   eax, edi 
  mov   bl, 160
  div   bl 
  and   eax, 0ffh
  inc   eax 
  mov   bl, 160 
  mul   bl 
  mov   edi, eax 
  pop   eax 
  jmp   .1
.3:
  mov   [gs:edi], ax 
  add   edi, 2
  jmp   .1
.2:
  mov   [disp_pos], edi 

  pop   ebp
  ret 





; void display_color_str(char* s, int color);
display_color_str:
  push  ebp
  mov   ebp, esp 

  mov   esi, [ebp + 8]  ; s
  mov   edi, [disp_pos]
  mov   ah, [esp + 12]  ; color
.1:
  lodsb
  test  al, al 
  jz    .2
  cmp   al, 0ah 
  jnz   .3
  push  eax 
  mov   eax, edi
  mov   bl, 160 
  div   bl 
  and   eax, 0ffh 
  inc   eax 
  mov   bl, 160 
  mul   bl 
  mov   edi, eax 
  pop   eax 
  jmp   .1
.3:
  mov   [gs:edi], ax 
  add   edi, 2
  jmp   .1
.2:
  mov   [disp_pos], edi 
  
  pop   ebp
  ret 





; void out_byte(uint16_t port, uint8_t value);
out_byte:
  mov edx, [esp + 4]    ; port 
  mov al, [esp + 4 + 4] ; value
  out dx, al 
  nop
  nop
  ret 





; uint8_t in_byte(uint16_t port);
in_byte:
  mov edx, [esp + 4]  ; port
  xor eax, eax 
  in  al, dx
  nop
  nop 
  ret
