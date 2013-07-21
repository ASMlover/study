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

  org 07c00h 




BaseOfStack     equ 07c00h
BaseOfLoader    equ 09000h      ; location of loader.bin be loaded - segment
OffsetOfLoader  equ 0100h       ; location of loader.bin be loaded - offset 
RootDirSectors  equ 14          ; space of root directory
SectorNoOfRootDirectory equ 19  ; root directory's first sector number 
SectorNoOfFAT1  equ 1           ; FAT1 first sector number = BPB_RsvdSecCnt 
DeltaSectorNo   equ 17          ; = BPB_RsvdSecCnt+(BPB_NumFATs*FASz)-2




  jmp short LABEL_START   ; start to boot
  nop 

  ; header of FAT12
  BS_OEMName      db  'ForrestY'    ; must be 8 bytes
  BPB_BytsPerSec  dw  512           ; bytes of per sector
  BPB_SecPerClus  db  1             ; sectors of per clustor
  BPB_RsvdSecCnt  dw  1             ; sectors of boot record
  BPB_NumFATs     db  2             ; FAT tables 
  BPB_RootEntCnt  dw  224           ; max file numbers of root directory
  BPB_TotSec16    dw  2880          ; total logic sectors 
  BPB_Media       db  0xf0          ; media descriptor
  BPB_FATSz16     dw  9             ; sectors of per FAT 
  BPB_SecPerTrk   dw  18            ; sectors of per track
  BPB_NumHeads    dw  2             ; number of disk header 
  BPB_HddSec      dd  0             ; hidded sectors 
  BPB_TotSec32    dd  0             ; sector number 
  BS_DrvNum       db  0             ; driver number of interrupt 13
  BS_Reserved1    db  0             ; unused 
  DS_BootSig      db  29h           ; extend boot mark(29h)
  DS_VolID        dd  0             ; volume id 
  DS_VolLab       db  'OrangeS0.02' ; must 11 bytes 
  BS_FileSysType  db  'FAT12   '    ; type of file system, must 8 bytes 


LABEL_START:
  mov   ax, cs 
  mov   ds, ax 
  mov   es, ax 
  mov   ss, ax 
  mov   sp, BaseOfStack 

  ; clear screen 
  mov   ax, 0600h 
  mov   bx, 0700h   ; black background, white character 
  mov   cx, 0       ; (0, 0)
  mov   dx, 0184fh  ; (80, 50)
  int   10h 

  mov   dh, 0
  call  DisplayStr

  ; reset floppy driver 
  xor   ah, ah 
  xor   dl, dl 
  int   13h 

  ; find the loader.bin from A driver's root directory 
  mov   word [wSectorNo], SectorNoOfRootDirectory 
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
  cmp   word [wRootDirSizeForLoop], 0 
  jz    LABEL_NO_LOADERBIN
  dec   word [wRootDirSizeForLoop]
  mov   ax, BaseOfLoader 
  mov   es, ax 
  mov   bx, OffsetOfLoader
  mov   ax, [wSectorNo]
  mov   cl, 1
  call  ReadSector

  mov   si, LoaderFileName  ; ds:si -> "LOADER  BIN"
  mov   di, OffsetOfLoader  ; es:di -> BaseOfLoader:0100 
  cld 
  mov   dx, 10h 
LABEL_SEARCH_FOR_LOADERBIN:
  cmp   dx, 0 
  jz    LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
  dec   dx
  mov   cx, 11 
LABEL_CMP_FILENAME:
  cmp   cx, 0
  jz    LABEL_FILENAME_FOUND
  dec   cx
  lodsb                     ; ds:si -> al 
  cmp   al, byte [es:di]
  jz    LABEL_GO_ON 
  jmp   LABEL_DIFFERENT

LABEL_GO_ON:
  inc   di 
  jmp   LABEL_CMP_FILENAME

LABEL_DIFFERENT:
  and   di, 0ffe0h
  add   di, 20h 
  mov   si, LoaderFileName
  jmp   LABEL_SEARCH_FOR_LOADERBIN 

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
  add   word [wSectorNo], 1
  jmp   LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
  mov   dh, 2 
  call  DisplayStr 

  jmp   $

LABEL_FILENAME_FOUND:
  mov   ax, RootDirSectors
  and   di, 0ffe0h 
  add   di, 01ah
  mov   cx, word [es:di]
  push  cx 
  add   cx, ax 
  add   cx, DeltaSectorNo
  mov   ax, BaseOfLoader
  mov   es, ax 
  mov   bx, OffsetOfLoader
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
  mov   dh, 1
  call  DisplayStr 

  jmp   BaseOfLoader:OffsetOfLoader









; variables 
wRootDirSizeForLoop   dw  RootDirSectors
wSectorNo             dw  0   ; sector number need be readed 
bOdd                  db  0   ; odd or even 
; string 
LoaderFileName        db  "LOADER  BIN", 0  ; name of loader.bin 
MessageLength         equ 9
BootMessage:          db  "Booting  "       ; 9 bytes 
Message1              db  "Ready.   "       ; 9 bytes 
Message2              db  "No LOADER"       ; 9 bytes 











; display a string 
DisplayStr:
  mov   ax, MessageLength
  mul   dh 
  add   ax, BootMessage 
  mov   bp, ax      ; ed:bp = address of string 
  mov   ax, ds 
  mov   es, ax 
  mov   cx, MessageLength
  mov   ax, 01301h
  mov   bx, 0007h
  mov   dl, 0
  int   10h 
  ret 








; read cl number sector into es:bx from the ax sector 
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






; find number is ax's sector in FAT 
GetFATEntry:
  push  es 
  push  bx 
  push  ax 
  mov   ax, BaseOfLoader 
  sub   ax, 0100 
  mov   es, ax 
  pop   ax 
  mov   byte [bOdd], 0
  mov   bx, 3
  mul   bx        ; ds:ax = ax * 3
  mov   bx, 2
  div   bx        ; ds:ax / 2 => ax -> 商, dx -> 余
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
  







times   510 - ($ - $$)  db  0
                        dw  0xaa55    ; end mark
