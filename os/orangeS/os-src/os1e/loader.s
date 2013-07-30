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

org 0100h 
  jmp LABEL_START               ; start 

%include "fat12hdr.inc"
%include "load.inc"
%include "pm.inc"






; GDT 
LABEL_GDT:          Descriptor 0, 0, 0
LABEL_DESC_FLAT_C:  Descriptor 0, 0fffffh, DA_CR|DA_32|DA_LIMIT_4K
LABEL_DESC_FLAT_RW: Descriptor 0, 0fffffh, DA_DRW|DA_32|DA_LIMIT_4K
LABEL_DESC_VIDEO:   Descriptor 0b8000h, 0ffffh, DA_DRW|DA_DPL3 

GdtLen  equ $ - LABEL_GDT
GdtPtr  dw  GdtLen - 1                      ; limit of segment 
        dd  BaseOfLoaderPhyAddr + LABEL_GDT ; base address of segment 

; GDT selector
SelectorFlatC   equ LABEL_DESC_FLAT_C   - LABEL_GDT
SelectorFlatRW  equ LABEL_DESC_FLAT_RW  - LABEL_GDT
SelectorVideo   equ LABEL_DESC_VIDEO    - LABEL_GDT + SA_RPL3 





BaseOfStack         equ 0100h 






LABEL_START:
  mov   ax, cs
  mov   ds, ax 
  mov   es, ax 
  mov   ss, ax 
  mov   sp, BaseOfStack 

  mov   dh, 0               ; "Loading  "
  call  DisplayStrRealMode  ; display a string  

  ; get memory information 
  mov   ebx, 0
  mov   di, _MemChkBuf
.MemChkLoop:
  mov   eax, 0e820h
  mov   ecx, 20 
  mov   edx, 0534d4150h
  int   15h
  jc    .MemChkFail 
  add   di, 20 
  inc   dword [_dwMCRNumber]
  cmp   ebx, 0
  jne   .MemChkLoop
  jmp   .MemChkOK 
.MemChkFail:
  mov   dword [_dwMCRNumber], 0
.MemChkOK:

  ; find kernel.bin in root directory of disk A 
  mov   word [wSectorNo], SectorNoOfRootDirectory
  xor   ah, ah          ; _
  xor   dl, dl          ;  |_ reset floppy driver 
  int   13h             ; _| 
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
  cmp   word [wRootDirSizeForLoop], 0
  jz    LABEL_NO_KERNELBIN
  dec   word [wRootDirSizeForLoop]
  mov   ax, BaseOfKernelFile 
  mov   es, ax 
  mov   bx, OffsetOfKernelFile
  mov   ax, [wSectorNo]
  mov   cl, 1
  call  ReadSector 

  mov   si, KernelFileName    ; ds:si -> "KERNEL  BIN"
  mov   di, OffsetOfKernelFile
  cld 
  mov   dx, 10h
LABEL_SEARCH_FOR_KERNELBIN:
  cmp   dx, 0 
  jz    LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
  dec   dx 
  mov   cx, 11 
LABEL_CMP_FILENAME:
  cmp   cx, 0 
  jz    LABEL_FILENAME_FOUND
  dec   cx 
  lodsb
  cmp   al, byte [es:di]      ; if al == es:di 
  jz    LABEL_GO_ON
  jmp   LABEL_DIFFERENT
LABEL_GO_ON:
  inc   di 
  jmp   LABEL_CMP_FILENAME    ; go on loop 

LABEL_DIFFERENT:
  and   di, 0ffe0h
  add   di, 20h 
  mov   si, KernelFileName
  jmp   LABEL_SEARCH_FOR_KERNELBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
  add   word [wSectorNo], 1
  jmp   LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_KERNELBIN:
  mov   dh, 2                 ; "No KERNEL"
  call  DisplayStrRealMode
  
  jmp   $



LABEL_FILENAME_FOUND:
  mov   ax, RootDirSectors 
  and   di, 0fff0h

  push  eax 
  mov   eax, [es:di + 01ch]
  mov   dword [dwKernelSize], eax 
  pop   eax 

  add   di, 01ah
  mov   cx, word [es:di]
  push  cx 
  add   cx, ax 
  add   cx, DeltaSectorNo
  mov   ax, BaseOfKernelFile
  mov   es, ax 
  mov   bx, OffsetOfKernelFile
  mov   ax, cx 
LABEL_GOON_LOADING_FILE:
  push  ax 
  push  bx 
  mov   ah, 0eh
  mov   al, '.'
  mov   bl, 0fh 
  int   10h 
  pop   bx 
  pop   ax 

  mov   cl, 1
  call  ReadSector
  pop   ax 
  call  GetFATEntry
  cmp   ax, 0fffh
  jz    LABEL_FILE_LOADED
  push  ax 
  mov   dx, RootDirSectors
  add   ax, dx 
  add   ax, DeltaSectorNo
  add   bx, [BPB_BytsPerSec]
  jmp   LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
  call  KillMotor
  mov   dh, 1         ; "Ready."
  call  DisplayStrRealMode

  ; enter into protected mode 

  ; load GDTR 
  lgdt  [GdtPtr]

  ; close interrupt 
  cli 

  ; open A20 address line 
  in    al, 92h 
  or    al, 00000010b
  out   92h, al 

  ; prepare switch to protected mode 
  mov   eax, cr0 
  or    eax, 1
  mov   cr0, eax 

  jmp   dword SelectorFlatC:(BaseOfLoaderPhyAddr+LABEL_PM_START)







; variables
wRootDirSizeForLoop dw  RootDirSectors
wSectorNo           dw  0
bOdd                db  0
dwKernelSize        dd  0

; strings 
KernelFileName      db  "KERNEL  BIN", 0
MessageLength       equ 9
LoadMessage:        db  "Loading  "
Message1            db  "Ready.   "
Message2            db  "NO KERNEL"








; DisplayStrRealMode
; display a string, dh->string index(0 baseded)
DisplayStrRealMode:
  mov   ax, MessageLength
  mul   dh 
  add   ax, LoadMessage
  mov   bp, ax            ; _
  mov   ax, ds            ;  |_ es:bp = string address
  mov   es, ax            ; _| 
  mov   cx, MessageLength
  mov   ax, 01301h 
  mov   bx, 0007h 
  mov   dl, 0 
  add   dh, 3
  int   10h 
  ret






; ReadSector
; from sectors[ax], read cl numbers sector into es:bx 
ReadSector:
  push  bp 
  mov   bp, sp 
  sub   esp, 2

  mov   byte [bp - 2], cl 
  push  bx 
  mov   bl, [BPB_SecPerTrk]
  div   bl 
  inc   ah 
  mov   cl, ah 
  mov   dh, al 
  shr   al, 1
  mov   ch, al 
  and   dh, 1
  pop   bx 
  mov   dl, [BS_DrvNum]
.GoOnReading:
  mov   ah, 2
  mov   al, byte [bp - 2]
  int   13h 
  jc    .GoOnReading 

  add   esp, 2
  pop   bp 
  ret 







; GetFATEntry
; find index is ax's sector in FAT 
GetFATEntry:
  push  es 
  push  bx 
  push  ax 
  mov   ax, BaseOfKernelFile
  sub   ax, 0100h
  mov   es, ax 
  pop   ax 
  mov   byte [bOdd], 0
  mov   bx, 3
  mul   bx 
  mov   bx, 2
  div   bx 
  cmp   dx, 0
  jz    LABEL_EVEN
  mov   byte [bOdd], 1
LABEL_EVEN:
  xor   dx, dx 
  mov   bx, [BPB_BytsPerSec]
  div   bx 

  push  dx 
  mov   bx, 0 
  add   ax, SectorNoOfFAT1
  mov   cl, 2
  call  ReadSector
  pop   dx 
  add   bx, dx 
  mov   ax, [es:bx]
  cmp   byte [bOdd], 1 
  jnz   LABEL_EVEN_2
  shr   ax, 4
LABEL_EVEN_2:
  and   ax, 0fffh
LABEL_GET_FAT_ENTRY_OK:
  pop   bx 
  pop   es 
  ret 





; KillMotor
; close motor of floppy driver 
KillMotor:
  push  dx 
  mov   dx, 03f2h
  mov   al, 0 
  out   dx, al 
  pop   dx 
  ret 









[SECTION .s32]
ALIGN 32 
[BITS 32]
LABEL_PM_START:
  mov   ax, SelectorVideo
  mov   gs, ax 
  mov   ax, SelectorFlatRW
  mov   ds, ax 
  mov   es, ax 
  mov   fs, ax 
  mov   ss, ax 
  mov   esp, TopOfStack 

  push  szMemChkTitle
  call  DisplayStr
  add   esp, 4

  call  DisplayMemInfo 
  call  SetupPaging

  call  InitKernel 

  jmp   SelectorFlatC:KernelEntryPointPhyAddr ; enter kernel


%include "lib.inc" 





; display memory information 
DisplayMemInfo:
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



;; setting up paging 
SetupPaging:
  xor   edx, edx 
  mov   eax, [dwMemSize]
  mov   ebx, 400000h    ; =4M = 4096 * 1024 
  div   ebx 
  mov   ecx, eax 
  test  edx, edx 
  jz    .no_remainder 
  inc   ecx 
.no_remainder:
  push  ecx 

  mov   ax, SelectorFlatRW
  mov   es, ax 
  mov   edi, PageDirBase 
  xor   eax, eax 
  mov   eax, PageTblBase | PG_P | PG_USU | PG_RWW 
.1:
  stosd 
  add   eax, 4096 
  loop  .1 

  pop   eax 
  mov   ebx, 1024 
  mul   ebx 
  mov   ecx, eax 
  mov   edi, PageTblBase
  xor   eax, eax 
  mov   eax, PG_P | PG_USU | PG_RWW
.2:
  stosd 
  add   eax, 4096
  loop  .2 
  
  mov   eax, PageDirBase
  mov   cr3, eax 
  mov   eax, cr0 
  or    eax, 80000000h
  mov   cr0, eax 
  jmp   short .3
.3:
  nop 
  ret








; InitKernel
; put memory of KERNEL.bin into new address 
InitKernel:
  xor   esi, esi 
  mov   cx, word [BaseOfKernelFilePhyAddr + 2ch]
  movzx eax, cx 
  mov   esi, [BaseOfKernelFilePhyAddr + 1ch]
  add   esi, BaseOfKernelFilePhyAddr 
.begin:
  mov   eax, [esi + 0]
  cmp   eax, 0
  jz    .no_action
  push  dword [esi + 010h]
  mov   eax, [esi + 04h]
  add   eax, BaseOfKernelFilePhyAddr 
  push  eax 
  push  dword [esi + 08h]
  call  MemCpy
  add   esp, 12 
.no_action:
  add   esi, 020h 
  dec   ecx 
  jnz   .begin

  ret 








[SECTION .data1]
ALIGN 32
LABEL_DATA:
; real mode 
; strings 
_szMemChkTitle: db  "BaseAddrL BaseAddrH LengthLow LengthHigh  Type", 0ah, 0
_szRAMSize:     db  "RAM size:", 0
_szReturn:      db  0ah, 0
; variables
_dwMCRNumber:   dd  0   ; memory check result 
_dwDispPos:     dd  (80 * 6 + 0) * 2
_dwMemSize:     dd  0
_ARDStruct:             ; address range descriptor structure
  _dwBaseAddrLow:   dd  0 
  _dwBaseAddrHigh:  dd  0 
  _dwLengthLow:     dd  0 
  _dwLengthHigh:    dd  0 
  _dwType:          dd  0
_MemChkBuf: times 256 db  0 

; protected mode 
szMemChkTitle   equ BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize       equ BaseOfLoaderPhyAddr + _szRAMSize
szReturn        equ BaseOfLoaderPhyAddr + _szReturn 
dwMCRNumber     equ BaseOfLoaderPhyAddr + _dwMCRNumber
dwDispPos       equ BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize       equ BaseOfLoaderPhyAddr + _dwMemSize 
ARDStruct       equ BaseOfLoaderPhyAddr + _ARDStruct
  dwBaseAddrLow   equ BaseOfLoaderPhyAddr + _dwBaseAddrLow
  dwBaseAddrHigh  equ BaseOfLoaderPhyAddr + _dwBaseAddrHigh 
  dwLengthLow     equ BaseOfLoaderPhyAddr + _dwLengthLow
  dwLengthHigh    equ BaseOfLoaderPhyAddr + _dwLengthHigh 
  dwType          equ BaseOfLoaderPhyAddr + _dwType
MemChkBuf       equ BaseOfLoaderPhyAddr + _MemChkBuf


; stack in the end of data segment 
StackSpace: times 1024  db  0 
TopOfStack  equ BaseOfLoaderPhyAddr + $
