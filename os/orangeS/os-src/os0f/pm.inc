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


; DA_   : descriptor attribute
; D     : data segment
; C     : code segment
; S     : system segment
; R     : read only 
; RW    : read write
; A     : accessed

; type of descriptor
DA_32       equ       4000h   ; 32bits segment 
DA_LIMIT_4K equ       8000h   ; segment limit 4KB 

DA_DPL0     equ       00h     ; DPL = 0
DA_DPL1     equ       20h     ; DPL = 1
DA_DPL2     equ       40h     ; DPL = 2
DA_DPL3     equ       60h     ; DPL = 3

; descriptor type of storage segment
DA_DR       equ       90h     ; type value - read only
DA_DRW      equ       92h     ; type value - read/write
DA_DRWA     equ       93h     ; type value - accessed read/write
DA_C        equ       98h     ; type value of code segment(execute only)
DA_CR       equ       9ah     ; type value of code segment(exec/read)
DA_CCO      equ       9ch     ; type value of code segment(exec 1 time)
DA_CCOR     equ       9eh     ; type value of code segment(exec 1 time/read)

; descriptor type of system segment
DA_LDT      equ       82h     ; local descriptor type
DA_TASKGATE equ       85h     ; task gate type value
DA_386TSS   equ       89h     ; 386 task status segment value
DA_386CGATE equ       8ch     ; 386 call gate type value
DA_386IGATE equ       8eh     ; 386 interrupt gate type value
DA_386TGATE equ       8fh     ; 386 trap gate type value







; RPL(requested privilege level)
; TI(table indicator)
;   TI = 0  read descriptor from global descriptor table(GDT)
;   TI = 1  read descriptor from local descriptor table(LDT)
; SA_   : selector attribute
SA_RPL0     equ       0       ; _
SA_RPL1     equ       1       ;  |_ RPL
SA_RPL2     equ       2       ;  |
SA_RPL3     equ       3       ; _|

SA_TIG      equ       0       ; _
SA_TIL      equ       4       ; _| TI 





; paging 
PG_P        equ       1       ; page exsit attribute
PG_RWR      equ       0       ; R/W attribute, read/exec 
PG_RWW      equ       2       ; R/W attribute, read/write/exec 
PG_USS      equ       0       ; U/S attribute, system level 
PG_USU      equ       4       ; U/S attribute, user level






; descriptor 
; usage: Descriptor base, limit, attr 
;        base:  dd
;        limit: dd(low 20 bits available)
;        attr:  dw(lower 4 bits ot higher byte are always 0)
%macro Descriptor 3
  dw  %2  & 0ffffh            ; segment limit 1
  dw  %1  & 0ffffh            ; segment base address 1
  db  (%1 >> 16) & 0ffh       ; segment base address 2
  dw  ((%2 >> 8) & 0f00h) | (%3 & 0f0ffh) ; attr1 + segment limit 2 + attr2
  db  (%1 >> 24) &0ffh        ; segment base address 3
%endmacro


; gate
; usage: Gate selector, offset, dcount, attr 
;        selector:  dw
;        offset:    dd 
;        dcount:    db
;        attr:      db 
%macro Gate 4
  dw  (%2 & 0ffffh)           ; offset 1
  dw  %1                      ; selector
  dw  (%3 & 1fh) | ((%4 << 8) & 0ff00h) ; attribute
  dw  ((%2 >> 16) & 0ffffh)   ; offset 2
%endmacro
