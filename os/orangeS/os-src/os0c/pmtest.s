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


PageDirBase   equ 200000h   ; base address of page directory: 2MB
PageTblBase   equ 201000h   ; base address of page table: 2MB + 4KB


org 0100h
  jmp LABEL_BEGIN





[SECTION .gdt]
; GDT 
LABEL_GDT:            Descriptor 0, 0, 0
LABEL_DESC_NORMAL:    Descriptor 0, 0ffffh, DA_DRW
LABEL_DESC_PAGE_DIR:  Descriptor PageDirBase, 4095, DA_DRW 
LABEL_DESC_PAGE_TBL:  Descriptor PageTblBase, 1023, DA_DRW|DA_LIMIT_4K 
LABEL_DESC_CODE32:    Descriptor 0, SegCode32Len-1, DA_C+DA_32 
LABEL_DESC_CODE16:    Descriptor 0, 0ffffh, DA_C 
LABEL_DESC_DATA:      Descriptor 0, SegDataLen-1, DA_DRW
LABEL_DESC_STACK:     Descriptor 0, TopOfStack, DA_DRWA+DA_32 
LABEL_DESC_VIDEO:     Descriptor 0b8000h, 0ffffh, DA_DRW

GdtLen      equ $ - LABEL_GDT ; length of GDT 
GdtPtr      dw  GdtLen - 1    ; limit of GDT 
            dd  0             ; base address of GDT 

; GDT selector 
SelectorNormal    equ LABEL_DESC_NORMAL   - LABEL_GDT
SelectorPageDir   equ LABEL_DESC_PAGE_DIR - LABEL_GDT 
SelectorPageTbl   equ LABEL_DESC_PAGE_TBL - LABEL_GDT
SelectorCode32    equ LABEL_DESC_CODE32   - LABEL_GDT
SelectorCode16    equ LABEL_DESC_CODE16   - LABEL_GDT
SelectorData      equ LABEL_DESC_DATA     - LABEL_GDT
SelectorStack     equ LABEL_DESC_STACK    - LABEL_GDT
SelectorVideo     equ LABEL_DESC_VIDEO    - LABEL_GDT








[SECTION .data1]
ALIGN 32 
[BITS 32]
LABEL_DATA:
SPValueInRealMode   dw  0
PMMessage:          db  "In Protected Mode now. ^_^", 0
OffsetPMMessage     equ PMMessage - $$
SegDataLen          equ $ - LABEL_DATA 







[SECTION .gs]
ALIGN 32 
[BITS 32]
LABEL_STACK:
  times 512 db 0
TopOfStack  equ $ - LABEL_STACK - 1








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

  ; initialize 16 bits code segment descriptor 
  mov ax, cs 
  movzx eax, ax 
  shl eax, 4
  add eax, LABEL_SEG_CODE16 
  mov word [LABEL_DESC_CODE16 + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_CODE16 + 4], al 
  mov byte [LABEL_DESC_CODE16 + 7], ah 

  ; initialize 32 bits code segment descriptor 
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

  ; loading GDTR 
  lgdt [GdtPtr]

  ; close interrupt 
  cli 

  ; open A20 address line 
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

  mov ax, 4c00h
  int 21h 






[SECTION .s32]
[BITS 32]
LABEL_SEG_CODE32:
  call  SetupPaging
  mov   ax, SelectorData
  mov   ds, ax 
  mov   ax, SelectorVideo
  mov   gs, ax 
  mov   ax, SelectorStack
  mov   ss, ax 
  mov   esp, TopOfStack 

  mov ah, 0ch
  xor esi, esi 
  xor edi, edi 
  mov esi, OffsetPMMessage
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
  jmp SelectorCode16:0


; set up paging 
SetupPaging:
  ; initialize page directory
  mov ax, SelectorPageDir
  mov es, ax 
  mov ecx, 1024 
  xor edi, edi 
  xor eax, eax 
  mov eax, PageTblBase | PG_P | PG_USU | PG_RWW 
.1:
  stosd
  add   eax, 4096 
  loop  .1

  ; initialize all page tables
  mov ax, SelectorPageTbl 
  mov es, ax 
  mov ecx, 1024 * 1024 
  xor edi, edi 
  xor eax, eax 
  mov eax, PG_P | PG_USU | PG_RWW
.2:
  stosd
  add   eax, 4096 
  loop  .2 

  mov eax, PageDirBase
  mov cr3, eax
  mov eax, cr0
  or  eax, 80000000h
  mov cr0, eax 
  jmp short .3
.3:
  nop 

  ret 


SegCode32Len  equ $ - LABEL_SEG_CODE32 








[SECTION .s16code]
ALIGN 32 
[BITS 16]
LABEL_SEG_CODE16:
  mov ax, SelectorNormal
  mov ds, ax 
  mov es, ax 
  mov fs, ax 
  mov gs, ax 
  mov ss, ax 

  mov eax, cr0
  and eax, 7ffffffeh  ; PE=0, PG=0
  mov cr0, eax 
LABEL_GO_BACK_TO_REAL:
  jmp 0:LABEL_REAL_ENTRY

SegCode16Len  equ $ - LABEL_SEG_CODE16

