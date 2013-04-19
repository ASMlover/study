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

  ORG 0x7c00        ; program loading address
  JMP entry
  DB  0x90
  DB  "HELLOIPL"    ; 启动区的名字可以是任意字符(8字节)
  DW  512           ; size of per sector(must be 512 bytes)
  DB  1             ; size of per cluster(must be 1)
  DW  1             ; bengining address of FAT
  DB  2             ; number of FAT(must be 2)
  DW  224           ; size of root directory
  DW  2880          ; size of this disk(must be 2880 cluster)
  DB  0xf0          ; kinds of disk(must be 0xf0)
  DW  9             ; length of FAT(must be 9)
  DW  18            ; clusters of per track(must be 18)
  DW  2             ; number of track-head(must be 2)
  DD  0             ; donot use the partition(must be 0)
  DD  2880          ; rewrite the size of disk
  DB  0, 0, 0x29
  DD  0xffffffff
  DB  "HELLO-OS   " ; name of disk(11 bytes)
  DB  "FAT12   "    ; format name of disk(8 bytes)
  RESB  18

entry:
  MOV AX, 0         ; initialize the register
  MOV SS, AX
  MOV SP, 0x7c00
  MOV DS, AX
  MOV ES, AX

  MOV SI, msg
display:
  MOV AL, [SI]
  ADD SI, 1
  CMP AL, 0
  JE  loop
  MOV AH, 0x0e      ; display a character of the msg
  MOV BX, 15        ; set color of the msg
  INT 0x10          ; call BIOS
  JMP display

loop:
  HLT               ; stop CPU, waitting for new instruction
  JMP loop

msg:
  DB  0x0a, 0x0a
  DB  "hello, OS world"
  DB  0x0a
  DB  0

  RESB  0x7dfe-$
  DB  0x55, 0xAA

  DB  0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
  RESB  4600
  DB  0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
  RESB  1469432
