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
LABEL_GDT:            Descriptor 0, 0, 0
LABEL_DESC_NORMAL:    Descriptor 0, 0ffffh, DA_DRW 
LABEL_DESC_CODE32:    Descriptor 0, SegCode32Len - 1, DA_C + DA_32 
LABEL_DESC_CODE16:    Descriptor 0, 0ffffh, DA_C
LABEL_DESC_CODE_DEST: Descriptor 0, SegCodeDestLen - 1, DA_C + DA_32 
LABEL_DESC_DATA:      Descriptor 0, SegDataLen - 1, DA_DRW 
LABEL_DESC_STACK:     Descriptor 0, TopOfStack, DA_DRWA + DA_32 
LABEL_DESC_LDT:       Descriptor 0, SegLDTLen - 1, DA_LDT 
LABEL_DESC_VIDEO:     Descriptor 0b8000h, 0ffffh, DA_DRW 

; Gate 
LABEL_CALL_GATE_TEST: Gate SelectorCodeDest, 0, 0, DA_386CGATE + DA_DPL0 

GdtLen            equ $ - LABEL_GDT   ; length of GDT 
GdtPtr            dw  GdtLen - 1      ; limit of GDT 
                  dd  0               ; base address of GDT

; GDT selector  
SelectorNormal        equ LABEL_DESC_NORMAL     - LABEL_GDT
SelectorCode32        equ LABEL_DESC_CODE32     - LABEL_GDT
SelectorCode16        equ LABEL_DESC_CODE16     - LABEL_GDT
SelectorCodeDest      equ LABEL_DESC_CODE_DEST  - LABEL_GDT
SelectorData          equ LABEL_DESC_DATA       - LABEL_GDT
SelectorStack         equ LABEL_DESC_STACK      - LABEL_GDT
SelectorLDT           equ LABEL_DESC_LDT        - LABEL_GDT
SelectorVideo         equ LABEL_DESC_VIDEO      - LABEL_GDT
SelectorCallGateTest  equ LABEL_CALL_GATE_TEST  - LABEL_GDT 







[SECTION .data1]
ALIGN 32
[BITS 32]
LABEL_DATA:
SPValueInRealMode   dw  0
PMMessage:          db  "In Protected Mode now. ^-^", 0
OffsetPmMessage     equ PMMessage - $$
StrTest:            db  "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0
OffsetStrTest       equ StrTest - $$
SegDataLen          equ $ - LABEL_DATA






[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
  times 512 db 0

TopOfStack  equ $ - LABEL_STACK







[SECTION .s16]
[BITS 16]
LABEL_BEGIN:
  mov ax, cs 
  mov ds, ax 
  mov es, ax 
  mov ss, ax 
  mov sp, 0100h 

  mov [LABEL_GO_BACK_TO_REAL + 3], ax 
  mov [SPValueInRealMode], sp 

  ; initialize 16bits code segment
  mov ax, cs 
  movzx eax, ax 
  shl eax, 4
  add eax, LABEL_SEG_CODE16
  mov word [LABEL_DESC_CODE16 + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_CODE16 + 4], al 
  mov byte [LABEL_DESC_CODE16 + 7], ah 

  ; initialize 32bits code segment 
  xor eax, eax 
  mov ax, cs 
  shl eax, 4 
  add eax, LABEL_SEG_CODE32 
  mov word [LABEL_DESC_CODE32 + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_CODE32 + 4], al 
  mov byte [LABEL_DESC_CODE32 + 7], ah

  ; initialize code segment descriptor of test call gate 
  xor eax, eax 
  mov ax, cs 
  shl eax, 4
  add eax, LABEL_SEG_CODE_DEST
  mov word [LABEL_DESC_CODE_DEST + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_CODE_DEST + 4], al 
  mov byte [LABEL_DESC_CODE_DEST + 7], ah 

  ; initialize descriptor of data segment 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4 
  add eax, LABEL_DATA 
  mov word [LABEL_DESC_DATA + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_DATA + 4], al 
  mov byte [LABEL_DESC_DATA + 7], ah 

  ; initialize descriptor of stack segment 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_STACK
  mov word [LABEL_DESC_STACK + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_STACK + 4], al 
  mov byte [LABEL_DESC_STACK + 7], ah 

  ; initialize LDT descriptor in GDT 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4 
  add eax, LABEL_LDT 
  mov word [LABEL_DESC_LDT + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_LDT + 4], al 
  mov byte [LABEL_DESC_LDT + 7], ah 

  ; initialize descriptor in LDT 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_CODE_A 
  mov word [LABEL_LDT_DESC_CODEA + 2], ax 
  shr eax, 16 
  mov byte [LABEL_LDT_DESC_CODEA + 4], al 
  mov byte [LABEL_LDT_DESC_CODEA + 7], ah 

  ; prepare for loading GDTR 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_GDT 
  mov dword [GdtPtr + 2], eax 

  ; loading GDTR 
  lgdt [GdtPtr] 

  ; close interrupt 
  cli 

  ; open A2 address line 
  in  al, 92h 
  or  al, 00000010b
  out 92h, al 

  ; prepare for switching to Protected mode 
  mov eax, cr0 
  or  eax, 1 
  mov cr0, eax 

  jmp dword SelectorCode32:0


LABEL_REAL_ENTRY:
  mov ax, cs 
  mov ds, ax 
  mov es, ax 
  mov ss, ax 
  mov sp, [SPValueInRealMode] 

  ; close A20 address line 
  in  al, 92h 
  and al, 11111101b 
  out 92h, al 

  ; open interrupt 
  sti 

  ; back to DOS 
  mov ax, 4c00h 
  int 21h 








[SECTION .s32]
[BITS 32]
LABEL_SEG_CODE32:
  mov ax, SelectorData
  mov ds, ax 
  mov ax, SelectorVideo 
  mov gs, ax 
  mov ax, SelectorStack
  mov ss, ax 
  mov esp, TopOfStack

  ; show a string 
  mov ah, 0ch 
  xor esi, esi 
  xor edi, edi 
  mov esi, OffsetPmMessage
  mov edi, (80 * 10 + 0) * 2
  cld 
.1:
  lodsb
  test  al, al 
  jz    .2
  mov   [gs:edi], ax 
  add   edi, 2
  jmp   .1 
.2:
  call  DisplayReturn
  call  SelectorCallGateTest:0

  ; load LDT 
  mov   ax, SelectorLDT
  lldt  ax 
  jmp   SelectorLDTCodeA:0

DisplayReturn:
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

SegCode32Len  equ $ - LABEL_SEG_CODE32 






[SECTION .sdest]
[BITS 32]
LABEL_SEG_CODE_DEST:
  mov ax, SelectorVideo
  mov gs, ax 

  mov edi, (80 * 12 + 0) * 2
  mov ah, 0ch 
  mov al, 'C'
  mov [gs:edi], ax 

  retf

SegCodeDestLen  equ $ - LABEL_SEG_CODE_DEST 







[SECTION .s16code]
ALIGN 32
[BITS 16]
LABEL_SEG_CODE16:
  ; return back to real mode 
  mov ax, SelectorNormal 
  mov ds, ax 
  mov es, ax 
  mov fs, ax 
  mov gs, ax 
  mov ss, ax 

  mov eax, cr0
  and eax, 11111110b 
  mov cr0, eax 
LABEL_GO_BACK_TO_REAL:
  jmp 0:LABEL_REAL_ENTRY

SegCode16Len  equ $ - LABEL_SEG_CODE16







[SECTION .ldt]
ALIGN 32
LABEL_LDT:
LABEL_LDT_DESC_CODEA: Descriptor 0, SegCodeALen - 1, DA_C + DA_32 

SegLDTLen         equ $ - LABEL_LDT
SelectorLDTCodeA  equ LABEL_LDT_DESC_CODEA - LABEL_LDT + SA_TIL 







[SECTION .la]
ALIGN 32 
[BITS 32]
LABEL_CODE_A:
  mov ax, SelectorVideo 
  mov gs, ax 
  
  mov edi, (80 * 13 + 0) * 2
  mov ah, 0ch
  mov al, 'L'
  mov [gs:edi], ax 

  jmp SelectorCode16:0

SegCodeALen   equ $ - LABEL_CODE_A
