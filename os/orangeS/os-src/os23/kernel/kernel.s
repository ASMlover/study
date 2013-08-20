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



SELECTOR_KERNEL_CS    equ 8

; import function 
extern cstart 
extern exception_handler 
extern spurous_irq

; import global variable 
extern gdt_ptr
extern idt_ptr
extern disp_pos





[section .bss]
StackSpace  resb  2 * 1024
StackTop:                   ; top of stack 





[section .text]

global _start 

global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error
global hwint00
global hwint01
global hwint02
global hwint03
global hwint04
global hwint05
global hwint06
global hwint07
global hwint08
global hwint09
global hwint10
global hwint11
global hwint12
global hwint13
global hwint14
global hwint15



_start:
  ; move esp from LOADER to KERNEL
  mov   esp, StackTop     ; stack in bss segment 

  mov   dword [disp_pos], 0

  sgdt  [gdt_ptr]         ; gdt_ptr will be used in cstart()
  call  cstart
  lgdt  [gdt_ptr]         ; use new GDT 

  lidt  [idt_ptr]

  jmp   SELECTOR_KERNEL_CS:csinit
csinit:
  ;ud2
  ;jmp 0x40:0

  ; push  0
  ; popfd                   ; pop top of stack into EFLAGS

  sti
  hlt 



; interrupt and exception - hardware interrupt
%macro hwint_master 1
  push  %1
  call  spurous_irq
  add   esp, 4
  hlt
%endmacro 






ALIGN 16 
hwint00:    ; interrupt routine for irq 0 (the clock)
  hwint_master  0

ALIGN 16 
hwint01:    ; interrupt routine for irq 1 (keyboard)
  hwint_master  1

ALIGN 16
hwint02:    ; interrupt routine for irq 2 (cascade!)
  hwint_master  2

ALIGN 16
hwint03:    ; interrupt routine for irq 3 (second serial)
  hwint_master  3

ALIGN 16
hwint04:    ; interrupt routine for irq 4 (first serial)
  hwint_master  4

ALIGN 16 
hwint05:    ; interrupt routine for irq 5 (XT winchester)
  hwint_master  5

ALIGN 16 
hwint06:    ; interrupt routine for irq 6 (floppy)
  hwint_master  6

ALIGN 16 
hwint07:    ; interrupt routine for irq 7 (printer)
  hwint_master  7 




%macro hwint_slave  1
  push  %1
  call  spurous_irq
  add   esp, 4
  hlt
%endmacro 


ALIGN 16 
hwint08:    ; interrupt routine for irq 8 (realtime clock)
  hwint_slave 8

ALIGN 16 
hwint09:    ; interrupt routine for irq 9 (irq 2 redirected)
  hwint_slave 9

ALIGN 16
hwint10:    ; interrupt routine for irq 10 
  hwint_slave 10 

ALIGN 16 
hwint11:    ; interrupt routine for irq 11 
  hwint_slave 11 

ALIGN 16
hwint12:    ; interrupt routine for irq  12 
  hwint_slave 12 

ALIGN 16
hwint13:    ; interrupt routine for irq 13 (FPU exception)
  hwint_slave 13 

ALIGN 16 
hwint14:    ; interrupt routine for irq 14 (AT winchester)
  hwint_slave 14 

ALIGN 16 
hwint15:    ; interrupt routine for irq 15 
  hwint_slave 15




; interrupt and exception -> exception
divide_error:
  push  0xffffffff        ; no error code 
  push  0                 ; vector_no = 0
  jmp   exception
single_step_exception:
  push  0xffffffff        ; no error code 
  push  1                 ; vector_no = 1
  jmp   exception
nmi:
  push  0xffffffff        ; no error code 
  push  2                 ; vector_no = 2
  jmp   exception 
breakpoint_exception:
  push  0xffffffff        ; no error code 
  push  3                 ; vector_no = 3
  jmp   exception 
overflow:
  push  0xffffffff        ; no error code 
  push  4                 ; vector_no = 4
  jmp   exception 
bounds_check:
  push  0xffffffff        ; no error code 
  push  5                 ; vector_no = 5
  jmp   exception
inval_opcode:
  push  0xffffffff        ; no error code 
  push  6                 ; vector_no = 6
  jmp   exception 
copr_not_available:
  push  0xffffffff        ; no error code 
  push  7                 ; vector_no = 7
  jmp   exception 
double_fault:
  push  8                 ; vector_no = 8
  jmp   exception 
copr_seg_overrun:
  push  0xffffffff        ; no error code 
  push  9                 ; vector_no = 9
  jmp   exception
inval_tss:
  push  10                ; vector_no = a
  jmp   exception 
segment_not_present:
  push  11                ; vector_no = b
  jmp   exception 
stack_exception:
  push  12                ; vector_no = c
  jmp   exception 
general_protection:
  push  13                ; vector_no = d
  jmp   exception 
page_fault:
  push  14                ; vector_no = e
  jmp   exception 
copr_error:
  push  0xffffffff        ; no error code 
  push  16                ; vector_no = 10h 
  jmp   exception

exception:
  call  exception_handler
  add   esp, 4 * 2
  hlt
