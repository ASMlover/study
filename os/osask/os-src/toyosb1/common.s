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
  global  _asm_interrupt_handler20
  global  _asm_interrupt_handler21
  global  _asm_interrupt_handler27
  global  _asm_interrupt_handler2c 
  global  _asm_interrupt_handler0c
  global  _asm_interrupt_handler0d
  global  _asm_stop_user_app
  global  _load_cr0, _store_cr0 
  global  _load_tr
  global  _memory_test_sub
  global  _farjump, _farcall
  global  _asm_toy_api, _start_user_app
  extern  _interrupt_handler20
  extern  _interrupt_handler21
  extern  _interrupt_handler27 
  extern  _interrupt_handler2c
  extern  _interrupt_handler0c
  extern  _interrupt_handler0d
  extern  _toy_api


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

_asm_interrupt_handler20: ; void asm_interrupt_handler20(void);
  push    es
  push    ds
  pushad 
  mov     eax, esp
  push    eax
  mov     ax, ss
  mov     ds, ax
  mov     es, ax
  call    _interrupt_handler20
  pop     eax
  popad
  pop     ds
  pop     es 
  iretd


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



_asm_interrupt_handler0c: ; void asm_interrupt_handler0c(void)
  sti
  push    es
  push    ds
  pushad
  mov     eax, esp
  push    eax
  mov     ax, ss
  mov     ds, ax
  mov     es, ax
  call    _interrupt_handler0c
  cmp     eax, 0
  jne     _asm_stop_user_app
  pop     eax
  popad
  pop     ds
  pop     es
  add     esp, 4          ; need this in int 0x0c
  iretd



_asm_interrupt_handler0d: ; void asm_interrupt_handler0d(void);
  sti
  push    es
  push    ds
  pushad
  mov     eax, esp
  push    eax
  mov     ax, ss
  mov     ds, ax
  mov     es, ax
  call    _interrupt_handler0d
  cmp     eax, 0
  jne     _asm_stop_user_app
  pop     eax
  popad
  pop     ds
  pop     es 
  add     esp, 4
  iretd



_load_cr0:                ; int load_cr0(void);
  mov eax, cr0
  ret

_store_cr0:               ; void store_cr0(int cr0);
  mov eax, [esp + 4]
  mov cr0, eax
  ret 

_load_tr:                 ; void load_tr(int tr);
  ltr [esp + 4]           ; tr 
  ret


; unsigned int memory_test_sub(unsigned int start, unsigned int end);
_memory_test_sub:
  push    edi
  push    esi
  push    ebx
  mov     esi,    0xaa55aa55      ; val0 = 0xaa55aa55
  mov     edi,    0x55aa55aa      ; val1 = 0x55aa55aa
  mov     eax,    [esp + 12 + 4]  ; i = start
_mts_loop:
  mov     ebx,    eax
  add     ebx,    0xffc           ; address = i + 0xffc
  mov     edx,    [ebx]           ; old_val = *address
  mov     [ebx],  esi             ; *address = val0
  xor     dword [ebx], 0xffffffff ; *address ^= 0xffffffff
  cmp     edi,    [ebx]           ; if (*address != val1) goto _mts_finish
  jne     _mts_finish
  xor     dword [ebx], 0xffffffff ; *address ^= 0xffffffff
  cmp     esi,    [ebx]           ; if (*address != val0) goto _mts_finish
  jne     _mts_finish
  mov     [ebx],  edx             ; *address = old_val
  add     eax,    0x1000          ; i += 0x1000
  cmp     eax,    [esp + 12 + 8]  ; if (i <= end) goto _mts_loop
  jbe     _mts_loop
  pop     ebx
  pop     esi
  pop     edi
  ret
_mts_finish:
  mov     [ebx],  edx             ; *address = old_val
  pop     ebx
  pop     esi
  pop     edi
  ret 


_farjump:                 ; void farjump(int eip, int cs);
  jmp far [esp + 4]       ; eip, cs
  ret 


_farcall:                 ; void farcall(int eip, int cs);
  call  far [esp + 4]     ; eip, cs
  ret


_asm_toy_api:
  sti
  push  ds
  push  es 
  pushad                  ; used to storage push 
  pushad                  ; push used to pass to toy_api
  mov   ax, ss
  mov   ds, ax            ; storage segment address of OS to ds and ss
  mov   es, ax
  call  _toy_api
  cmp   eax, 0
  jne   _asm_stop_user_app
  add   esp, 32
  popad
  pop   es 
  pop   ds
  iretd
_asm_stop_user_app:
  ; eax is the address of tss.esp0
  mov   esp, [eax]
  mov   dword [eax + 4], 0
  popad
  ret                     ; return to cmd_app



_start_user_app:          ; void start_user_app(int eip, 
                          ;     int cs, int esp, int ds, int* tss_esp0);
  pushad                  ; storage all 32bits registers
  mov   eax, [esp + 36]   ; eip
  mov   ecx, [esp + 40]   ; cs
  mov   edx, [esp + 44]   ; esp
  mov   ebx, [esp + 48]   ; ds/ss
  mov   ebp, [esp + 52]   ; tss.esp0
  mov   [ebp], esp        ; storage the esp of OS
  mov   [ebp + 4], ss     ; storage the ss of OS
  mov   es, bx
  mov   ds, bx
  mov   fs, bx
  mov   gs, bx
  or    ecx, 3            ; (segment number of application) or 3
  or    ebx, 3            ; (segment number of application) or 3
  push  ebx               ; ss of application
  push  edx               ; esp of application
  push  ecx               ; cs of application
  push  eax               ; eip of application
  retf
