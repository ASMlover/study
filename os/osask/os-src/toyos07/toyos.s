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


; toy-os

; BOOT_INFO
CYLS  equ 0x0ff0    ; setting boot sector
LEDS  equ 0x0ff1
VMODE equ 0x0ff2    ; information about number of color, bits of color
SCRNX equ 0x0ff4    ; screen X
SCRNY equ 0x0ff6    ; screen Y
VRAM  equ 0x0ff8    ; start address of bitmap buffer
  
  org 0xc200

  mov al, 0x13      ; VGA, 320 * 200 * 8 bits color
  mov ah, 0x00
  int 0x10

  mov byte  [VMODE], 8            ; record mode of screen 
  mov word  [SCRNX], 320
  mov word  [SCRNY], 200
  mov dword [VRAM],  0x000a0000

  ; get all status of leds used BIOS
  mov ah, 0x02
  int 0x16          ; keyboard BIOS
  mov [LEDS], al

finish:
  hlt
  jmp finish
