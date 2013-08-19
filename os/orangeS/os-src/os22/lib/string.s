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


[section .text]

; import functions 
global  memcpy 




; void* memcpy(void* es:dest, void* ds:src, int size);
memcpy:
  push  ebp 
  mov   ebp, esp 

  push  esi 
  push  edi 
  push  ecx 

  mov   edi, [ebp + 8]  ; destination 
  mov   esi, [ebp + 12] ; source 
  mov   ecx, [ebp + 16] ; counter
.1:
  cmp   ecx, 0          ; judge the counter
  jz    .2

  mov   al, [ds:esi]
  inc   esi 

  mov   byte [es:edi], al 
  inc   edi 

  dec   ecx 
  jmp   .1
.2:
  mov   eax, [ebp + 8]  ; return value 

  pop   ecx 
  pop   edi 
  pop   esi 
  mov   esp, ebp 
  pop   ebp 

  ret
