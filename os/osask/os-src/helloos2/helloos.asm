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

; helloos

DB  0xeb, 0x4e, 0x90
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

DB  0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
DB  0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
DB  0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
DB  0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
DB  0xee, 0xf4, 0xeb, 0xfd

; display information
DB  0x0a, 0x0a
DB  "HELLO, WORLD"
DB  0x0a
DB  0

RESB  0x1fe-$
DB  0x55, 0xaa

DB  0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB  4600
DB  0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB  1469432
