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

[instrset "i486p"]
VBEMODE   equ 0x103 ; 800 * 600 * 8bit
BOOTPACK  equ 0x00280000
DSKCAC    equ 0x00100000
DSKCAC0   equ 0x00008000

; BOOT_INFO
CYLS  equ 0x0ff0    ; setting boot sector
LEDS  equ 0x0ff1
VMODE equ 0x0ff2    ; information about number of color, bits of color
SCRNX equ 0x0ff4    ; screen X
SCRNY equ 0x0ff6    ; screen Y
VRAM  equ 0x0ff8    ; start address of bitmap buffer
  
  org 0xc200

  ; check if there's VBE 
  mov ax, 0x9000
  mov es, ax 
  mov di, 0
  mov ax, 0x4f00
  int 0x10 
  cmp ax, 0x004f
  jne screen320

  ; check version of VBE
  mov ax, [es:di + 4]
  cmp ax, 0x0200
  jb  screen320     ; if (ax < 0x0200) goto screen320

  ; get information of screen
  mov cx, VBEMODE
  mov ax, 0x4f01
  int 0x10
  cmp ax, 0x004f
  jne screen320 

  ; check the information of screen
  cmp byte [es:di + 0x19], 8
  jne screen320
  cmp byte [es:di + 0x1b], 4
  jne screen320
  mov ax, [es:di + 0x00]
  and ax, 0x0080
  jz  screen320     ; if mode attribute's bit7 = 0, give up 

  ; change screen mode 
  mov bx, VBEMODE + 0x4000
  mov ax, 0x4f02
  int 0x10
  mov byte [VMODE], 8             ; record mode of screen
  mov ax, [es:di + 0x12]
  mov [SCRNX], ax 
  mov ax, [es:di + 0x14]
  mov [SCRNY], ax 
  mov eax, [es:di + 0x28]
  mov [VRAM], eax
  jmp keystatus 

screen320:
  mov al, 0x13      ; VGA graphics, 320 * 200 * 8bit 
  mov ah, 0x00 
  int 0x10 
  mov byte [VMODE], 8             ; record mode of screen
  mov word [SCRNX], 320 
  mov word [SCRNY], 200 
  mov dword [VRAM], 0x000a0000

  ; get all status of leds used BIOS 
keystatus:
  mov ah, 0x02
  int 0x16          ; keyboard BIOS
  mov [LEDS], al 


  ; PIC close all interrupt 
  ; must be initialize before cli 
  mov al, 0xff 
  out 0x21, al 
  nop
  out 0xa1, al

  cli               ; disabled interrupts of CPU level

  ; CPU xxxx
  call  waitkeyboardout
  mov al, 0xd1
  out 0x64, al 
  call  waitkeyboardout
  mov al, 0xdf      ; enable a20
  out 0x60, al 
  call  waitkeyboardout

  lgdt  [gdtr0]
  mov   eax, cr0
  and   eax, 0x7fffffff
  or    eax, 0x00000001
  mov   cr0, eax
  jmp   piplelineflush
piplelineflush:
  mov   al, 1 * 8
  mov   ds, ax
  mov   es, ax
  mov   fs, ax
  mov   gs, ax
  mov   ss, ax

; bootpack 
  mov   esi, bootpack 
  mov   edi, BOOTPACK
  mov   ecx, 512 * 1024 / 4
  call  memcpy

; 
; 
  mov   esi, 0x7c00
  mov   edi, DSKCAC
  mov   ecx, 512 / 4
  call  memcpy

  mov   esi, DSKCAC0 + 512
  mov   edi, DSKCAC + 512
  mov   ecx, 0
  mov   cl,  byte [CYLS]
  imul  ecx, 512 * 18 * 2 / 4
  sub   ecx, 512 / 4
  call  memcpy

  mov   ebx, BOOTPACK
  mov   ecx, [ebx + 16]
  add   ecx, 3
  shr   ecx, 2
  jz    skip
  mov   esi, [ebx + 20]
  add   esi, ebx
  mov   edi, [ebx + 12]
  call  memcpy
skip:
  mov   esp, [ebx + 12]
  jmp   dword 2 * 8 : 0x0000001b

waitkeyboardout:
  in    al, 0x64
  and   al, 0x02
  jnz   waitkeyboardout
  ret 

memcpy:
  mov   eax, [esi]
  add   esi, 4
  mov   [edi], eax
  add   edi, 4
  sub   ecx, 1
  jnz   memcpy
  ret 

  alignb  16 

gdt0:
  resb  8
  dw    0xffff, 0x0000, 0x9200, 0x00cf
  dw    0xffff, 0x0000, 0x9a28, 0x0047
  dw    0
gdtr0:
  dw    8 * 3 - 1
  dd    gdt0
  alignb  16 

bootpack:
