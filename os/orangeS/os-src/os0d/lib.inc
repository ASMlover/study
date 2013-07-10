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


; display number in AL 
DisplayAL:
  push  ecx
  push  edx 
  push  edi 

  mov   edi, [dwDispPos]

  mov   ah, 0fh
  mov   dl, al 
  shr   al, 4
  mov   ecx, 2
.begin:
  and   al, 01111b
  cmp   al, 9
  ja    .1
  add   al, '0'
  jmp   .2
.1:
  sub   al, 0ah 
  add   al, 'A'
.2:
  mov   [gs:edi], ax 
  add   edi, 2
  mov   al, dl 
  loop  .begin

  mov   [dwDispPos], edi 

  pop   edi 
  pop   edx 
  pop   ecx 
  ret 





; display an integer number 
DisplayInt:
  mov   eax, [esp + 4]
  shr   eax, 24 
  call  DisplayAL 

  mov   eax, [esp + 4]
  shr   eax, 16 
  call  DisplayAL

  mov   eax, [esp + 4]
  shr   eax, 8
  call  DisplayAL 

  mov   eax, [esp + 4]
  call  DisplayAL 

  mov   ah, 07h
  mov   al, 'h'
  push  edi 
  mov   edi, [dwDispPos]
  mov   [gs:edi], ax 
  add   edi, 4
  mov   [dwDispPos], edi 
  pop   edi 
  ret 





; display a string 
DisplayStr:
  push  ebp 
  mov   ebp, esp
  push  ebx 
  push  esi 
  push  edi 

  mov   esi, [ebp + 8]
  mov   edi, [dwDispPos]
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
  mov   [dwDispPos], edi 

  pop   edi 
  pop   esi 
  pop   ebx 
  pop   ebp 
  ret 








; display return 
DisplayReturn:
  push  szReturn
  call  DisplayStr
  add   esp, 4
  ret
