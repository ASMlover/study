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

[format "WCOFF"]
[instrset "i486p"]
[bits 32]


[file "api_common.s"]

  global  _putc
  global  _puts
  global  _api_end
  global  _init_mem
  global  _malloc
  global  _free
  global  _getkey
  global  _win_open
  global  _win_close
  global  _win_puts
  global  _win_fill
  global  _win_point
  global  _win_refresh
  global  _win_line
  global  _create_timer
  global  _delete_timer
  global  _init_timer
  global  _set_timer
  global  _beep

[section .text]

_putc:                  ; void putc(int c);
  mov edx, 1
  mov al, [esp + 4]
  int 0x40
  ret 


_puts:                  ; void puts(const char* str);
  push  ebx
  mov   edx, 2
  mov   ebx, [esp + 8]  ; str 
  int   0x40
  pop   ebx
  ret


_api_end:               ; void api_end(void);
  mov edx, 4
  int 0x40 



_win_open:              ; int win_open(char* buf, int w, int h, 
                        ;     int alpha, char* title);
  push  edi
  push  esi 
  push  ebx
  mov   edx, 5
  mov   ebx, [esp + 16] ; buf
  mov   esi, [esp + 20] ; w
  mov   edi, [esp + 24] ; h
  mov   eax, [esp + 28] ; alpha
  mov   ecx, [esp + 32] ; title
  int   0x40
  pop   ebx
  pop   esi
  pop   edi
  ret 




_win_close:             ; void win_close(int win);
  push  ebx
  mov   edx, 14
  mov   ebx, [esp + 8]  ; win
  int   0x40
  pop   ebx
  ret




_win_puts:              ; void win_puts(int win, int x, int y, 
                        ;     int color, int len, char* str);
  push  edi
  push  esi
  push  ebp
  push  ebx
  mov   edx, 6
  mov   ebx, [esp + 20] ; win
  mov   esi, [esp + 24] ; x
  mov   edi, [esp + 28] ; y
  mov   eax, [esp + 32] ; color
  mov   ecx, [esp + 36] ; len
  mov   ebp, [esp + 40] ; str
  int   0x40
  pop   ebx
  pop   ebp
  pop   esi
  pop   edi
  ret



_win_fill:              ; void win_fill(int win, int x0, int y0, 
                        ;     int x1, int y1, int color);
  push  edi
  push  esi
  push  ebp
  push  ebx
  mov   edx, 7
  mov   ebx, [esp + 20] ; win
  mov   eax, [esp + 24] ; x0
  mov   ecx, [esp + 28] ; y0
  mov   esi, [esp + 32] ; x1
  mov   edi, [esp + 36] ; y1
  mov   ebp, [esp + 40] ; color
  int   0x40
  pop   ebx
  pop   ebp
  pop   esi
  pop   edi
  ret



_init_mem:              ; void init_mem(void);
  push  ebx
  mov   edx, 8
  mov   ebx, [cs:0x0020]    ; address of malloc memory
  mov   eax, ebx
  add   eax, 32 * 1024
  mov   ecx, [cs:0x0000]    ; size of data segment 
  sub   ecx, eax
  int   0x40
  pop   ebx
  ret



_malloc:                ; void* malloc(int size);
  push  ebx
  mov   edx, 9
  mov   ebx, [cs:0x0020]
  mov   ecx, [esp + 8]  ; size
  int   0x40
  pop   ebx
  ret



_free:                  ; void free(void* ptr, int size);
  push  ebx
  mov   edx, 10 
  mov   ebx, [cs:0x0020]
  mov   eax, [esp + 8]  ; ptr
  mov   ecx, [esp + 12] ; size
  int   0x40
  pop   ebx
  ret



_win_point:             ; void win_point(int win, int x, int y, int color);
  push  edi
  push  esi
  push  ebx
  mov   edx, 11 
  mov   ebx, [esp + 16] ; win
  mov   esi, [esp + 20] ; x
  mov   edi, [esp + 24] ; y
  mov   eax, [esp + 28] ; color
  int   0x40
  pop   ebx
  pop   esi
  pop   edi
  ret




_win_refresh:           ; void win_refresh(int win, 
                        ;   int x0, int y0, int x1, int y1);
  push  edi
  push  esi
  push  ebx
  mov   edx, 12
  mov   ebx, [esp + 16] ; win 
  mov   eax, [esp + 20] ; x0
  mov   ecx, [esp + 24] ; y0
  mov   esi, [esp + 28] ; x1
  mov   edi, [esp + 32] ; y1
  int   0x40
  pop   ebx
  pop   esi
  pop   edi
  ret 


_win_line:              ; void win_line(int win, 
                        ;     int x0, int y0, int x1, int y1, int color);
  push  edi
  push  esi
  push  ebp
  push  ebx
  mov   edx, 13
  mov   ebx, [esp + 20] ; win
  mov   eax, [esp + 24] ; x0
  mov   ecx, [esp + 28] ; y0
  mov   esi, [esp + 32] ; x1
  mov   edi, [esp + 36] ; y1
  mov   ebp, [esp + 40] ; color
  int   0x40
  pop   ebx
  pop   ebp
  pop   esi
  pop   edi
  ret



_getkey:                ; int getkey(int mode);
  mov   edx, 15
  mov   eax, [esp + 4]  ; mode
  int   0x40
  ret




_create_timer:          ; int create_timer(void);
  mov   edx, 16
  int   0x40
  ret




_init_timer:            ; void init_timer(int timer, int data);
  push  ebx
  mov   edx, 17
  mov   ebx, [esp + 8]  ; timer
  mov   eax, [esp + 12] ; data 
  int   0x40
  pop   ebx
  ret



_set_timer:             ; void set_timer(int timer, int time);
  push  ebx
  mov   edx, 18
  mov   ebx, [esp + 8]  ; timer
  mov   eax, [esp + 12] ; time 
  int   0x40
  pop   ebx
  ret



_delete_timer:          ; void delete_timer(int timer);
  push  ebx
  mov   edx, 19
  mov   ebx, [esp + 8]  ; timer
  int   0x40
  pop   ebx
  ret




_beep:                  ; void beep(int tone);
  mov   edx, 20 
  mov   eax, [esp + 4]  ; tone
  int   0x40
  ret

