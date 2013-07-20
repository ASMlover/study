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
  BS_FileSysType  db  'FAT12'       ; type of file system, must 8 bytes 


LABEL_START:
  mov   ax, cs 
  mov   ds, ax 
  mov   es, ax 
  call  DisplayStr
  jmp   $
DisplayStr:
  mov   ax, BootMsg
  mov   bp, ax      ; ed:bp = address of string 
  mov   cx, 16 
  mov   ax, 01301h
  mov   bx, 000ch
  mov   dl, 0
  int   10h 
  ret 

BootMsg:                db  "Hello, OS world!"
times   510 - ($ - $$)  db  0
                        dw  0xaa55    ; end mark
