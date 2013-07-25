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

BaseOfStack         equ 0100h
BaseOfKernelFile    equ 08000h  ; kernel.bin addr loaded - segment address 
OffsetOfKernelFile  equ 0h      ; kernel.bin addr loaded - offset address


  jmp LABEL_START               ; start 

%include "fat12hdr.inc"




LABEL_START:
  mov   ax, cs
  mov   ds, ax 
  mov   es, ax 
  mov   ss, ax 
  mov   sp, BaseOfStack 

  mov   dh, 0           ; "Loading  "
  call  DisplayStr      ; display a string 

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
  call  DisplayStr
  
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
  call  DisplayStr

  jmp   $








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








; DisplayStr 
; display a string, dh->string index(0 baseded)
DisplayStr:
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



