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
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
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
#include "type.h"
#include "const.h"
#include "protect.h"
#include "global.h"
#include "proto.h"


/* interrupt handler */
extern void divide_error(void);
extern void single_step_exception(void);
extern void nmi(void);
extern void breakpoint_exception(void);
extern void overflow(void);
extern void bounds_check(void);
extern void inval_opcode(void);
extern void copr_not_available(void);
extern void double_fault(void);
extern void copr_seg_overrun(void);
extern void inval_tss(void);
extern void segment_not_present(void);
extern void stack_exception(void);
extern void general_protection(void);
extern void page_fault(void);
extern void copr_error(void);
extern void hwint00(void);
extern void hwint01(void);
extern void hwint02(void);
extern void hwint03(void);
extern void hwint04(void);
extern void hwint05(void);
extern void hwint06(void);
extern void hwint07(void);
extern void hwint08(void);
extern void hwint09(void);
extern void hwint10(void);
extern void hwint11(void);
extern void hwint12(void);
extern void hwint13(void);
extern void hwint14(void);
extern void hwint15(void);




void 
exception_handler(int vec_no, int err_code, int eip, int cs, int eflags)
{
  int i;
  int text_color = 0x74;

  char* err_msg[] = {
    "#DE divide error", 
    "#DB reserved", 
    "--  nmi interrupt", 
    "#BP breakpoint", 
    "#OF overflow", 
    "#BR bound range exceeded", 
    "#UD invalid opcode (undefined opcode)", 
    "#NM device not available (no match coprocessor)", 
    "#DF double fault", 
    "    coprocessor segment overrun (reserved)", 
    "#TS invalid tss", 
    "#NP segment not present", 
    "#SS stack segment fault", 
    "#GP general protection", 
    "#PF page fault", 
    "--  (intel reserved. do not use)", 
    "#MF x87 FPU floating-point error (math fault)", 
    "#AC alignment check", 
    "#MC machine check", 
    "#XF SIMD floating-point exception"
  };

  disp_pos = 0;
  for (i = 0; i < 80 * 5; ++i)
    display_str(" ");
  disp_pos = 0;

  display_color_str("exception! --> ", text_color);
  display_color_str(err_msg[vec_no], text_color);
  display_color_str("\n\n", text_color);
  display_color_str("eflags: ", text_color);
  display_int(eflags);
  display_color_str("cs: ", text_color);
  display_int(cs);
  display_color_str("eip: ", text_color);
  display_int(eip);

  if (0xffffffff != err_code) {
    display_color_str("error code: ", text_color);
    display_int(err_code);
  }
}




static void 
init_idt_desc(unsigned char vector, uint8_t desc_type, 
    int_handler_cb handler, unsigned char privilege)
{
  gate_t* gate  = &idt[vector];
  uint32_t base = (uint32_t)handler;

  gate->offset_low  = base & 0xffff;
  gate->selector    = SELECTOR_KERNEL_CS;
  gate->dcount      = 0;
  gate->addr        = desc_type | (privilege << 5);
  gate->offset_high = (base >> 16) & 0xffff;
}


void 
init_port(void)
{
  init_8259A();

  /* all initialize as interrupt gate */
  init_idt_desc(INT_VECTOR_DIVIDE, DA_386IGATE, 
      divide_error, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_DEBUG, DA_386IGATE, 
      single_step_exception, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_NMI, DA_386IGATE, 
      nmi, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_BREAKPOINT, DA_386IGATE, 
      breakpoint_exception, PRIVILEGE_USER);
  init_idt_desc(INT_VECTOR_OVERFLOW, DA_386IGATE, 
      overflow, PRIVILEGE_USER);
  init_idt_desc(INT_VECTOR_BOUNDS, DA_386IGATE, 
      bounds_check, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_INVAL_OP, DA_386IGATE, 
      inval_opcode, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_COPROC_NOT, DA_386IGATE, 
      copr_not_available, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_DOUBLE_FAULT, DA_386IGATE, 
      double_fault, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_COPROC_SEG, DA_386IGATE, 
      copr_seg_overrun, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_INVAL_TSS, DA_386IGATE, 
      inval_tss, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_SEG_NOT, DA_386IGATE, 
      segment_not_present, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_STACK_FAULT, DA_386IGATE, 
      stack_exception, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_PROTECTION, DA_386IGATE, 
      general_protection, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_PAGE_FAULT, DA_386IGATE, 
      page_fault, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_COPROC_ERR, DA_386IGATE, 
      copr_error, PRIVILEGE_KERNAL);

  init_idt_desc(INT_VECTOR_IRQ0 + 0, DA_386IGATE, 
      hwint00, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 1, DA_386IGATE, 
      hwint01, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 2, DA_386IGATE, 
      hwint02, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 3, DA_386IGATE, 
      hwint03, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 4, DA_386IGATE, 
      hwint04, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 5, DA_386IGATE, 
      hwint05, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 6, DA_386IGATE, 
      hwint06, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ0 + 7, DA_386IGATE, 
      hwint07, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 0, DA_386IGATE, 
      hwint08, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 1, DA_386IGATE, 
      hwint09, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 2, DA_386IGATE, 
      hwint10, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 3, DA_386IGATE, 
      hwint11, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 4, DA_386IGATE, 
      hwint12, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 5, DA_386IGATE, 
      hwint13, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 6, DA_386IGATE, 
      hwint14, PRIVILEGE_KERNAL);
  init_idt_desc(INT_VECTOR_IRQ8 + 7, DA_386IGATE, 
      hwint15, PRIVILEGE_KERNAL);
}
