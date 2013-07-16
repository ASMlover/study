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

PageDirBase0  equ 200000h ; start address of page directory: 2M 
PageTblBase0  equ 201000h ; start address of page table: 2M + 4K 
PageDirBase1  equ 210000h ; start address of page directory: 2M + 64K 
PageTblBase1  equ 211000h ; start address of page table: 2M + 64K + 4K 

LinearAddrDemo  equ 00401000h
ProcFoo         equ 00401000h
ProcBar         equ 00501000h 
ProcPagingDemo  equ 00301000h 




org 0100h
  jmp LABEL_BEGIN








[SECTION .gdt]
; GDT 
LABEL_GDT:          Descriptor 0, 0, 0
LABEL_DESC_NORMAL:  Descriptor 0, 0ffffh, DA_DRW 
LABEL_DESC_FLAT_C:  Descriptor 0, 0fffffh, DA_CR|DA_32|DA_LIMIT_4K
LABEL_DESC_FLAT_RW: Descriptor 0, 0fffffh, DA_DRW|DA_LIMIT_4K
LABEL_DESC_CODE32:  Descriptor 0, SegCode32Len-1, DA_CR|DA_32 
LABEL_DESC_CODE16:  Descriptor 0, 0ffffh, DA_C 
LABEL_DESC_DATA:    Descriptor 0, SegDataLen-1, DA_DRW 
LABEL_DESC_STACK:   Descriptor 0, TopOfStack, DA_DRWA|DA_32 
LABEL_DESC_VIDEO:   Descriptor 0b8000h, 0ffffh, DA_DRW 

GdtLen      equ $ - LABEL_GDT ; length of GDT 
GdtPtr      dw  GdtLen - 1    ; limit of GDT 
            dd  0             ; base address of GDT 

; GDT selector 
SelectorNormal    equ LABEL_DESC_NORMAL   - LABEL_GDT 
SelectorFlatC     equ LABEL_DESC_FLAT_C   - LABEL_GDT
SelectorFlatRW    equ LABEL_DESC_FLAT_RW  - LABEL_GDT
SelectorCode32    equ LABEL_DESC_CODE32   - LABEL_GDT
SelectorCode16    equ LABEL_DESC_CODE16   - LABEL_GDT
SelectorData      equ LABEL_DESC_DATA     - LABEL_GDT
SelectorStack     equ LABEL_DESC_STACK    - LABEL_GDT
SelectorVideo     equ LABEL_DESC_VIDEO    - LABEL_GDT 







[SECTION .data1]
ALIGN 32 
[BITS 32]
LABEL_DATA:
; used in real mode 
; string 
_szPMMessage:   db  "In Protected Mode now. ^-^", 0ah, 0ah, 0
_szMemChkTitle: db  "BaseAddrL BaseAddrH LengthLow LengthHigh  Type", 0ah, 0
_szRAMSize      db  "RAM size:", 0
_szReturn       db  0ah, 0
; variables
_wSPValueInRealMode dw  0
_dwMCRNumber:       dd  0 
_dwDispPos:         dd  (80 * 6 + 0) * 2
_dwMemSize:         dd  0
_ARDStruct: ; Address Range Descriptor Structure
  _dwBaseAddrLow:   dd  0 
  _dwBaseAddrHigh:  dd  0
  _dwLengthLow:     dd  0
  _dwLengthHigh:    dd  0
  _dwType:          dd  0
_PageTableNumber    dd  0
_MemChkBuf: times 256 db  0 

; used in Protected mode 
szPMMessage     equ _szPMMessage      - $$
szMemChkTitle   equ _szMemChkTitle    - $$
szRAMSize       equ _szRAMSize        - $$
szReturn        equ _szReturn         - $$
dwMCRNumber     equ _dwMCRNumber      - $$
dwDispPos       equ _dwDispPos        - $$
dwMemSize       equ _dwMemSize        - $$
ARDStruct       equ _ARDStruct        - $$
  dwBaseAddrLow   equ _dwBaseAddrLow    - $$
  dwBaseAddrHigh  equ _dwBaseAddrHigh   - $$
  dwLengthLow     equ _dwLengthLow      - $$
  dwLengthHigh    equ _dwLengthHigh     - $$
  dwType          equ _dwType           - $$
PageTableNumber equ _PageTableNumber  - $$
MemChkBuf       equ _MemChkBuf        - $$

SegDataLen  equ $ - LABEL_DATA






[SECTION .idt]
ALIGN 32 
[BITS 32]
LABEL_IDT:
; gate 
%rep 128
        Gate SelectorCode32, SpuriousHandler, 0, DA_386IGATE
%endrep
.080h:  Gate SelectorCode32, UserIntHandler, 0, DA_386IGATE


IdtLen  equ $ - LABEL_IDT
IdtPtr  dw  IdtLen - 1
        dd  0







[SECTION .gs]
ALIGN 32 
[BITS 32]
LABEL_STACK:
  times 512 db  0
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
  mov [_wSPValueInRealMode], sp 

  ; get number of memory 
  mov ebx, 0
  mov di, _MemChkBuf
.loop:
  mov eax, 0e820h
  mov ecx, 20 
  mov edx, 0534d4150h
  int 15h 
  jc  LABEL_MEM_CHK_FAIL
  add di, 20 
  inc dword [_dwMCRNumber]
  cmp ebx, 0
  jne .loop
  jmp LABEL_MEM_CHK_OK
LABEL_MEM_CHK_FAIL:
  mov dword [_dwMCRNumber], 0
LABEL_MEM_CHK_OK:
  
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

  ; initialize data segment 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_DATA
  mov word [LABEL_DESC_DATA + 2], ax 
  shr eax, 16 
  mov byte [LABEL_DESC_DATA + 4], al 
  mov byte [LABEL_DESC_DATA + 7], ah 

  ; initialize stack segment 
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

  ; prepare for loading IDTR 
  xor eax, eax 
  mov ax, ds 
  shl eax, 4
  add eax, LABEL_IDT
  mov dword [IdtPtr + 2], eax 

  ; load GDTR 
  lgdt [GdtPtr] 

  ; close interrupt 
  cli 

  ; load IDTR 
  lidt [IdtPtr] 

  ; open A20 address line 
  in  al, 92h 
  or  al, 00000010b
  out 92h, al 

  ; prepare for switching to protected mode 
  mov eax, cr0 
  or  eax, 1
  mov cr0, eax 

  jmp dword SelectorCode32:0 

LABEL_REAL_ENTRY:
  mov ax, cs 
  mov ds, ax 
  mov es, ax 
  mov ss, ax 
  mov sp, [_wSPValueInRealMode]

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
  mov ax, SelectorData
  mov ds, ax 
  mov es, ax 
  mov ax, SelectorVideo
  mov gs, ax 
  mov ax, SelectorStack
  mov ss, ax 
  mov esp, TopOfStack 

  call  Init8295A 
  int   080h
  jmp   $

  ; show a string 
  push  szPMMessage
  call  DisplayStr
  add   esp, 4

  push  szMemChkTitle
  call  DisplayStr
  add   esp, 4

  call  DisplayMemSize
  call  PagingDemo

  jmp   SelectorCode16:0



; initialize 8259A
Init8295A:
  ; main 8259A, ICW1
  mov   al, 011h
  out   020h, al 
  call  io_delay

  ;sub 8259A, ICW1 
  out   0a0h, al 
  call  io_delay 

  ; IRQ0, main 8259A, ICW2 
  mov   al, 020h 
  out   021h, al 
  call  io_delay 

  ; IRQ8, sub 8259A, ICW2 
  mov   al, 028h 
  out   0a1h, al 
  call  io_delay 

  ; IR2, main 8259A, ICW3 
  mov   al, 004h 
  out   021h, al 
  call  io_delay 

  ; IR2, sub 8259A, ICW3 
  mov   al, 002h
  out   0a1h, al 
  call  io_delay

  ; main 8259A, ICW4 
  mov   al, 001h 
  out   021h, al 
  call  io_delay

  ; sub 8259A, ICW4 
  out   0a1h, al 
  call  io_delay 

  mov   al, 11111110b   ; open timer interrupt only 
  out   021h, al        ; main 8259A, OCW1 
  call  io_delay

  mov   al, 11111111b   ; disable all interrupts of sub 8259A
  out   0a1h, al        ; sub 8259A, OCW1 
  call  io_delay

  ret 


io_delay:
  nop
  nop 
  nop
  nop 
  ret 


_UserIntHandler:
UserIntHandler  equ _UserIntHandler - $$
  mov ah, 0ch 
  mov al, 'I'
  mov [gs:((80 * 0 + 70) * 2)], ax 
  iretd

_SpuriousHandler:
SpuriousHandler equ _SpuriousHandler - $$
  mov ah, 0ch
  mov al, '!'
  mov [gs:((80 * 0 + 75) * 2)], ax 
  jmp $
  iretd




; set up paging 
SetupPaging:
  ; calculate how many PED and page tables by number of memory 
  xor   edx, edx 
  mov   eax, [dwMemSize]
  mov   ebx, 400000h      ; = 4M = 4096 * 1024 
  div   ebx 
  mov   ecx, eax 
  test  edx, edx 
  jz    .no_remainder
  inc   ecx 
.no_remainder:
  mov [PageTableNumber], ecx 

  ; initialize page directory 
  mov   ax, SelectorFlatRW 
  mov   es, ax 
  mov   edi, PageDirBase0
  xor   eax, eax 
  mov   eax, PageTblBase0 | PG_P | PG_USU | PG_RWW 
.1:
  stosd 
  add   eax, 4096
  loop  .1 

  ; initialize all page tables 
  mov   eax, [PageTableNumber]
  mov   ebx, 1024 
  mul   ebx 
  mov   ecx, eax 
  mov   edi, PageTblBase0 
  xor   eax, eax 
  mov   eax, PG_P | PG_USU | PG_RWW
.2:
  stosd
  add   eax, 4096
  loop  .2 

  mov   eax, PageDirBase0
  mov   cr3, eax 
  mov   eax, cr0
  or    eax, 80000000h 
  mov   cr0, eax 
  jmp   short .3
.3:
  nop
  ret 



; testing paging 
PagingDemo:
  mov   ax, cs 
  mov   ds, ax 
  mov   ax, SelectorFlatRW 
  mov   es, ax 

  push  LenFoo
  push  OffsetFoo
  push  ProcFoo
  call  MemCpy 
  add   esp, 12 

  push  LenBar
  push  OffsetBar 
  push  ProcBar
  call  MemCpy 
  add   esp, 12 

  push  LenPagingDemoAll
  push  OffsetPagingDemoProc
  push  ProcPagingDemo
  call  MemCpy
  add   esp, 12 

  mov   ax, SelectorData
  mov   ds, ax 
  mov   es, ax 

  call  SetupPaging 

  call  SelectorFlatC:ProcPagingDemo
  call  PagingDirSwitch
  call  SelectorFlatC:ProcPagingDemo

  ret  

; switching page tables 
PagingDirSwitch:
  ; initialize page directory 
  mov   ax, SelectorFlatRW
  mov   es, ax 
  mov   edi, PageDirBase1 
  xor   eax, eax 
  mov   eax, PageTblBase1 | PG_P | PG_USU | PG_RWW
  mov   ecx, [PageTableNumber]
.1:
  stosd
  add   eax, 4096
  loop  .1 

  ; initialize all page tables 
  mov   eax, [PageTableNumber]
  mov   ebx, 1024
  mul   ebx 
  mov   ecx, ecx 
  mov   edi, PageTblBase1 
  xor   eax, eax 
  mov   eax, PG_P | PG_USU | PG_RWW
.2:
  stosd
  add   eax, 4096 
  loop  .2

  mov   eax, LinearAddrDemo
  shr   eax, 22 
  mov   ebx, 4096
  mul   ebx 
  mov   ecx, eax 
  mov   eax, LinearAddrDemo
  shr   eax, 12 
  and   eax, 03ffh
  mov   ebx, 4
  mul   ebx 
  add   eax, ecx 
  add   eax, PageTblBase1 
  mov   dword [es:eax], ProcBar | PG_P | PG_USU | PG_RWW 

  mov   eax, PageDirBase1 
  mov   cr3, eax 
  jmp   short .3
.3:
  nop 
  ret 


PagingDemoProc:
OffsetPagingDemoProc  equ PagingDemoProc - $$
  mov   eax, LinearAddrDemo
  call  eax 
  retf 
LenPagingDemoAll      equ $  - PagingDemoProc


foo:
OffsetFoo equ foo - $$
  mov ah, 0ch 
  mov al, 'F'
  mov [gs:((80 * 17 + 0) * 2)], ax 
  mov al, 'o'
  mov [gs:((80 * 17 + 1) * 2)], ax 
  mov [gs:((80 * 17 + 2) * 2)], ax 
  ret 
LenFoo    equ $ - foo 


bar:
OffsetBar equ bar - $$
  mov ah, 0ch
  mov al, 'B'
  mov [gs:((80 * 18 + 0) * 2)], ax 
  mov al, 'a'
  mov [gs:((80 * 18 + 1) * 2)], ax 
  mov al, 'r'
  mov [gs:((80 * 18 + 2) * 2)], ax 
  ret 
LenBar    equ $ - bar 


; display memory information 
DisplayMemSize:
  push  esi 
  push  edi 
  push  ecx 

  mov   esi, MemChkBuf
  mov   ecx, [dwMCRNumber]
.loop:
  mov   edx, 5
  mov   edi, ARDStruct
.1:
  push  dword [esi]
  call  DisplayInt
  pop   eax 
  stosd
  add   esi, 4
  dec   edx 
  cmp   edx, 0 
  jnz   .1
  call  DisplayReturn
  cmp   dword [dwType], 1
  jne   .2
  mov   eax, [dwBaseAddrLow]
  add   eax, [dwLengthLow]
  cmp   eax, [dwMemSize]
  jb    .2
  mov   [dwMemSize], eax 
.2:
  loop  .loop

  call  DisplayReturn
  push  szRAMSize
  call  DisplayStr
  add   esp, 4

  push  dword [dwMemSize]
  call  DisplayInt
  add   esp, 4

  pop   ecx
  pop   edi 
  pop   esi 
  ret 

%include "lib.inc"

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
  and eax, 7ffffffeh
  mov cr0, eax 

LABEL_GO_BACK_TO_REAL:
  jmp 0:LABEL_REAL_ENTRY

SegCode16Len  equ $ - LABEL_SEG_CODE16
