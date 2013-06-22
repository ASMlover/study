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

  CYLS  equ   10    ; cylinders -> 10(柱面)

  org 0x7c00        ; program loading address
  jmp entry
  db  0x90
  db  "helloipl"    ; 启动区的名字可以是任意字符(8字节)
  dw  512           ; size of per sector(must be 512 bytes)
  db  1             ; size of per cluster(must be 1)
  dw  1             ; bengining address of FAT
  db  2             ; number of FAT(must be 2)
  dw  224           ; size of root directory
  dw  2880          ; size of this disk(must be 2880 cluster)
  db  0xf0          ; kinds of disk(must be 0xf0)
  dw  9             ; length of FAT(must be 9)
  dw  18            ; clusters of per track(must be 18)
  dw  2             ; number of track-head(must be 2)
  dd  0             ; donot use the partition(must be 0)
  dd  2880          ; rewrite the size of disk
  db  0, 0, 0x29
  dd  0xffffffff
  db  "TOY-OS     " ; name of disk(11 bytes)
  db  "FAT12   "    ; format name of disk(8 bytes)
  resb  18

entry:
  mov ax, 0         ; initialize the register
  mov ss, ax
  mov sp, 0X7C00
  mov ds, ax

  ; read disks
  mov ax, 0X0820
  mov es, ax
  mov ch, 0         ; 柱面0
  mov dh, 0         ; 磁头0 
  mov cl, 2         ; 扇区2 

  ; loop for read disks
readloop:
  mov si, 0         ; record error times
retry:
  mov ah, 0x02      ; ah=0x02, read disk
  mov al, 1         ; 1 sector
  mov bx, 0
  mov dl, 0x00      ; A驱动器
  int 0x13          ; call BIOS of disk
  jnc next          ; if not error jump to next
  add si, 1         ; add 1 to si
  cmp si, 5
  jae error         ; if si >= 5, goto error
  mov ah, 0x00
  mov dl, 0x00      ; driver A
  int 0x13          ; reset drivers
  jmp retry

next:
  mov ax, es        ; 内存地址后移0x200
  add ax, 0x0020
  mov es, ax        ; es = es + 0x0020
  add cl, 1
  cmp cl, 18
  jbe readloop      ; if cl <= 18, goto readloop
  mov cl, 1
  add dh, 1
  cmp dh, 2
  jb  readloop      ; if dh < 2, goto readloop
  mov dh, 0
  add ch, 1
  cmp ch, CYLS
  jb  readloop      ; if ch < CYLS, goto readloop 


  ; begin to execute os from boot sector
  mov [0xff0], ch   ; [0xff0] = CYLS
  jmp 0xc200

finish:
  hlt               ; stop CPU, waitting for new instruction
  jmp finish

error:
  mov si, msg

display:
  mov al, [si]
  add si, 1
  cmp al, 0
  je  finish
  mov ah, 0x0e      ; display a character of the msg
  mov bx, 15        ; set color of the msg
  int 0x10          ; call BIOS
  jmp display

msg:
  db  0x0a, 0x0a
  db  "toy-os: load error"
  db  0x0a
  db  0

  resb  0x7dfe-$
  db  0x55, 0xaa
