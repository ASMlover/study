/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __COMMON_HEADER_H__
#define __COMMON_HEADER_H__

extern void io_hlt(void);
extern void io_cli(void);
extern void io_sti(void);
extern void io_stihlt(void);
extern int io_in8(int port);
extern int io_in16(int port);
extern int io_in32(int port);
extern void io_out8(int port, int data);
extern void io_out16(int port, int data);
extern void io_out32(int port, int data);
extern int io_load_eflags(void);
extern void io_store_eflags(int eflags);
extern void load_gdtr(int limit, int addr);
extern void load_idtr(int limit, int addr);
extern int load_cr0(void);
extern void store_cr0(int cr0);
extern  void load_tr(int tr);
extern void asm_interrupt_handler20(void);
extern void asm_interrupt_handler21(void);
extern void asm_interrupt_handler27(void);
extern void asm_interrupt_handler2c(void);
extern void asm_interrupt_handler0c(void);
extern void asm_interrupt_handler0d(void);
extern unsigned int memory_test_sub(unsigned int start, unsigned int end);
extern void farjump(int eip, int cs);
extern void farcall(int eip, int cs);
extern void asm_toy_api(void);
extern void start_user_app(int eip, int cs, int esp, int ds, int* tss_esp0);

#endif  /* __COMMON_HEADER_H__ */
