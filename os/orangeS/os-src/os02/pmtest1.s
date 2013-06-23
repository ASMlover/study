; Copyright (c) 2013 ASMlover. All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
;
;  * Redistributions of source code must retain the above copyright
;    notice, this list ofconditions and the following disclaimer.
;
;    notice, this list of conditions and the following disclaimer in
;  * Redistributions in binary form must reproduce the above copyright
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


%include "pm.inc"


org 07c00h
  jmp LABEL_BEGIN 


[SECTION .gdt]
; GDT 
;                            segment base, segment limit, attribute 
; null descriptor
LABEL_GDT:          Descriptor 0, 0, 0 
; non-uniform code segment
LABEL_DESC_CODE32:  Descriptor 0, SegCode32Len - 1, DA_C + DA_32 
; first address of video memory 
LABEL_DESC_VIDEO:   Descriptor 0b8000h, 0ffffh, DA_DRW 

GdtLen    equ   $ - LABEL_GDT     ; length of GDT 
GdtPtr    dw    GdtLen - 1        ; GDT limit 
          dd    0                 ; GDT base address

; GDT selector
SelectorCode32  equ   LABEL_DESC_CODE32 - LABEL_GDT
SelectorVideo   equ   LABEL_DESC_VIDEO  - LABEL_GDT 


SegGdtLen       equ   ($-$$)      ; length of gdt segment

[SECTION .s16]
[BITS 16]
LABEL_BEGIN:
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax 
  mov sp, 0100h 

  ; initialize descriptor of 32bits code segment
  xor eax, eax 
  mov ax, cs
  shl eax, 4
  add eax, LABEL_SEG_CODE32 
  mov word [LABEL_DESC_CODE32 + 2], ax
  shl eax, 16
  mov byte [LABEL_DESC_CODE32 + 4], al 
  mov byte [LABEL_DESC_CODE32 + 7], ah 

  ; prepare for loading GDTR
  xor eax, eax
  mov ax, ds
  shl eax, 4
  add eax, LABEL_GDT          ; eax <- gdt base address
  mov dword [GdtPtr + 2], eax ; [GdtPtr + 2] <- gdt base address

  ; load GDTR
  lgdt  [GdtPtr]

  ; close interruptor 
  cli 

  ; open address line a20
  in  al, 92h
  or  al, 00000010b
  out 92h, al 

  ; prepare to switch to protected mode
  mov eax, cr0
  or  eax, 1
  mov cr0, eax

  ; enter to protected mode 
  jmp dword SelectorCode32:0  ; load cs to SelectorCode32, and jump to 
                              ; SelectorCode32:0 



  SegCode16Len  equ   ($-$$)


[SECTION .s32]
[BITS 32]
LABEL_SEG_CODE32:
  mov ax, SelectorVideo
  mov gs, ax                  ; selector of video segment

  mov edi, (80 * 11 + 79) * 2 ; screen -> 11th row, 79th line 
  mov ah, 0ch                 ; 0000: black background, 1100: red charcter
  mov al, 'P'
  mov [gs:edi], ax

  jmp $


SegCode32Len    equ   $ - LABEL_SEG_CODE32 


times 510 - (SegGdtLen + SegCode16Len + SegCode32Len) - 9 db 0
dw    0xaa55
