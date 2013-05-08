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

; common functions

[format "WCOFF"]        ; making target file's mode
[instrset "i486p"]      ; for 486
[bits 32]               ; 32bits's machine language


; making target file's information
[file "common.s"]       ; source file's information 

  ; export functions
  global  _io_hlt, _io_cli, _io_sti, _io_stihlt
  global  _io_in8, _io_in16, _io_in32
  global  _io_out8, _io_out16, _io_out32
  global  _io_load_eflags, _io_store_eflags
  global  _load_gdtr, _load_idtr
  global  _asm_interrupt_handler21
  global  _asm_interrupt_handler27
  global  _asm_interrupt_handler2c
  extern  _interrupt_handler21
  extern  _interrupt_handler27 
  extern  _interrupt_handler2c


[section .text]
_io_hlt:                ; void io_hlt(void);
  hlt
  ret 

_io_cli:                ; void io_cli(void);
  cli
  ret

_io_sti:                ; void io_sti(void);
  sti
  ret 

_io_stihlt:             ; void io_stihlt(void);
  sti
  hlt
  ret

_io_in8:                ; int io_in8(int port);
  mov edx, [esp + 4]
  mov eax, 0
  in  al,  dx
  ret

_io_in16:               ; int io_in16(int port);
  mov edx, [esp + 4]
  mov eax, 0
  in  ax, dx
  ret

_io_in32:               ; int io_in32(int port);
  mov edx, [esp + 4]
  in eax, dx
  ret

_io_out8:               ; void io_out8(int port, int data);
  mov edx, [esp + 4]
  mov al,  [esp + 8]
  out dx, al
  ret

_io_out16:              ; void io_out16(int port, int data);
  mov edx, [esp + 4]
  mov eax, [esp + 8]
  out dx, ax
  ret

_io_out32:              ; void io_out32(int port, int data);
  mov edx, [esp + 4]
  mov eax, [esp + 8]
  out dx, eax
  ret

_io_load_eflags:        ; int io_load_eflags(void);
  pushfd                ;  push eflags
  pop eax
  ret

_io_store_eflags:       ; void io_store_eflags(int eflags);
  mov   eax, [esp + 4]
  push  eax
  popfd                 ; pop eflags
  ret 

_load_gdtr:             ; void load_gdtr(int limit, int addr);
  mov   ax, [esp + 4]
  mov   [esp + 6], ax
  lgdt  [esp + 6]
  ret

_load_idtr:             ; void load_idtr(int limit, int addr);
  mov   ax, [esp + 4]
  mov   [esp + 6], ax
  lidt  [esp + 6]
  ret

_asm_interrupt_handler21: ; void asm_interrupt_handler21(void);
  push    es 
  push    ds
  pushad
  mov     eax, esp
  push    eax
  mov     ax, ss
  mov     ds, ax
  mov     es, ax
  call    _interrupt_handler21
  pop     eax
  popad
  pop     ds
  pop     es 
  iretd

_asm_interrupt_handler27: ; void asm_interrupt_handler27(void);
  push    es
  push    ds
  pushad
  mov     eax, esp
  push    eax
  mov     ax, ss
  mov     ds, ax
  mov     es, ax
  call    _interrupt_handler27
  pop     eax
  popad
  pop     ds
  pop     es
  iretd 

_asm_interrupt_handler2c: ; void asm_interrupt_handler2c(void);
  push    es
  push    ds
  pushad
  mov     eax, esp
  push    eax
  mov     ax, ss
  mov     ds, ax
  mov     es, ax
  call    _interrupt_handler2c
  pop     eax
  popad
  pop     ds
  pop     es
  iretd
