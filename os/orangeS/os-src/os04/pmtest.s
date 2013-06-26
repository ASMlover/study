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

%include "pm.inc"



org 0100h
  jmp LABEL_BEGIN


[SECTION .gdt]
; GDT
LABEL_GDT:          Descriptor  0, 0, 0           ; null descriptor 
LABEL_DESC_NORMAL:  Descriptor  0, 0ffffh, DA_DRW ; normal descriptor 
; uniform code segment, 32bits
LABEL_DESC_CODE32:  Descriptor  0, SegCode32Len - 1, DA_C + DA_32 
LABEL_DESC_CODE16:  Descriptor  0, 0ffffh, DA_C   ; uniform CS (16bits)
LABEL_DESC_DATA:    Descriptor  0, DataLen - 1, DA_DRW  ; data 
; Stack 32bits
LABEL_DESC_STACK:   Descriptor  0, TopOfStack, DA_DRWA + DA_32 
LABEL_DESC_TEST:    Descriptor  0500000h, 0ffffh, DA_DRW 
LABEL_DESC_VIDEO:   Descriptor  0b8000h, 0ffffh, DA_DRW ; video 1st address

GdtLen      equ   $ - LABEL_GDT       ; length of GDT 
GdtPtr      dw    GdtLen - 1          ; limit of GDT 
            dd    0                   ; base address of GDT 


; GDT selector 
SelectorNormal    equ LABEL_DESC_NORMAL - LABEL_GDT
SelectorCode32    equ LABEL_DESC_CODE32 - LABEL_GDT
SelectorCode16    equ LABEL_DESC_CODE16 - LABEL_GDT
SelectorData      equ LABEL_DESC_DATA   - LABEL_GDT
SelectorStack     equ LABEL_DESC_STACK  - LABEL_GDT
SelectorTest      equ LABEL_DESC_TEST   - LABEL_GDT
SelectorVideo     equ LABEL_DESC_VIDEO  - LABEL_GDT





[SECTION .data1]    ; data segment
ALIGN 32 
[BITS 32]
LABEL_DATA:
SPValueInRealMode   dw    0
PMMessage:          db    "In Protected Mode now. ^-^", 0
OffsetPMMessage     equ   PMMessage - $$ 
StrTest:            db    "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0
OffsetStrTest       equ   StrTest - $$
DataLen             equ   $ - LABEL_DATA





[SECTION .gs]       ; global stack segment 
ALIGN 32 
[BITS 32]
LABEL_STACK:
  times 512 db 0
TopOfStack          equ   $ - LABEL_STACK - 1





[SECTION .16]
[BITS 16]
LABEL_BEGIN:
  mov ax, cs 
  mov ds, ax
  mov es, ax 
  mov ss, ax 
  mov sp, 0100h 

  mov [LABEL_GO_BACK_TO_REAL + 3], ax
  mov [SPValueInRealMode], sp 

  ; initialize code segment descriptor of 16bits
  mov ax, cs 
  movzx eax, ax 
  shl eax, 4
  add eax, LABEL_SEG_CODE16 
  mov word [LABEL_DESC_CODE16 + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_CODE16 + 4], al 
  mov byte [LABEL_DESC_CODE16 + 7], ah 

  ; initialize code segment descriptor of 32bits 
  xor eax, eax 
  mov ax, cs 
  shl eax, 4 
  add eax, LABEL_SEG_CODE32 
  mov word [LABEL_DESC_CODE32 + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_CODE32 + 4], al 
  mov byte [LABEL_DESC_CODE32 + 7], ah 

  ; initialize data segment descriptor 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_DATA 
  mov word [LABEL_DESC_DATA + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_DATA + 4], al 
  mov byte [LABEL_DESC_DATA + 7], ah 

  ; initialize stack segment descriptor 
  xor eax, eax
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_STACK 
  mov word [LABEL_DESC_STACK + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_STACK + 4], al 
  mov byte [LABEL_DESC_STACK + 7], ah 

  ; prepare for loading GDTR 
  xor eax, eax
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_GDT 
  mov dword [GdtPtr + 2], eax 

  ; load GDTR 
  lgdt [GdtPtr] 

  ; close interruptor 
  cli 

  ; open A20 address line 
  in al, 92h 
  or al, 00000010b 
  out 92h, al 

  ; prepare for switching to protected mdoe 
  mov eax, cr0 
  or  eax, 1
  mov cr0, eax 

  ; enter into protected mode 
  jmp dword SelectorCode32:0 


LABEL_REAL_ENTRY:       ; return back here from protected mode 
  mov ax, cs 
  mov ds, ax 
  mov es, ax 
  mov ss, ax 

  mov sp, [SPValueInRealMode] 

  ; close A20 address line 
  in al, 92h 
  and al, 11111101b 
  out 92h, al 

  sti                   ; open interruptor 

  mov ax, 4c00h
  int 21h






[SECTION .s32]    ; 32bits code segment 
[BITS 32] 
LABEL_SEG_CODE32: 
  mov ax, SelectorData 
  mov ds, ax 
  mov ax, SelectorTest 
  mov es, ax 
  mov ax, SelectorVideo 
  mov gs, ax 
  mov ax, SelectorStack 
  mov ss, ax 
  mov esp, TopOfStack 

  ; show a string 
  mov ah, 0ch           ; 0000: black background    1100: red charcter 
  xor esi, esi 
  xor edi, edi 
  mov esi, OffsetPMMessage 
  mov edi, (80 * 10 + 0) * 2
  cld 
.1:
  lodsb 
  test  al, al 
  jz  .2 
  mov [gs:edi], ax 
  add edi, 2 
  jmp .1 
.2:   ; display over 
  call  DispReturn 
  call  TestRead 
  call  TestWrite
  call  TestRead 

  jmp SelectorCode16:0


TestRead:
  xor esi, esi 
  mov ecx, 8
.loop:
  mov   al, [es:esi]
  call  Display 
  inc   esi 
  loop  .loop 
  call  DispReturn 
  ret 


TestWrite:
  push  esi 
  push  edi 
  xor   esi, esi 
  xor   edi, edi 
  mov   esi, OffsetStrTest
  cld 
.1:
  lodsb
  test  al, al 
  jz    .2 
  mov   [es:edi], al 
  inc   edi 
  jmp   .1 
.2:
  pop   edi 
  pop   esi 
  ret 




Display:
  ; show number of AL 
  push  ecx 
  push  edx  

  mov   ah, 0ch 
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
  add   edi, 2 

  pop   edx 
  pop   ecx 

  ret 



DispReturn:
  push  eax 
  push  ebx 
  mov   eax, edi 
  mov   bl, 160 
  div   bl 
  and   eax, 0ffh 
  inc   eax 
  mov   bl, 160 
  mul   bl 
  mov   edi, eax 
  pop   ebx 
  pop   eax 
  ret 



SegCode32Len    equ   $ - LABEL_SEG_CODE32 








[SECTION .s16code]
ALIGN 32 
[BITS 16]
LABEL_SEG_CODE16:
  ; back to real mode 
  mov ax, SelectorNormal 
  mov ds, ax 
  mov es, ax 
  mov fs, ax 
  mov gs, ax 
  mov ss, ax 

  mov eax, cr0
  and al, 11111110b
  mov cr0, eax 

LABEL_GO_BACK_TO_REAL:
  jmp 0:LABEL_REAL_ENTRY

SegCode16Len    equ   $ - LABEL_SEG_CODE16
