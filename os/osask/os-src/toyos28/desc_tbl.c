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
#include "common.h"
#include "desc_tbl.h"

static void 
set_segment_descriptor(segment_descriptor_t* sd, 
    unsigned int limit, int base, int access_right)
{
  if (limit > 0xfffff) {
    access_right |= 0x8000;
    limit /= 0x1000;
  }
  sd->limit_low     = limit & 0xffff;
  sd->base_low      = base & 0xffff;
  sd->base_mid      = (base >> 16) & 0xff;
  sd->access_right  = access_right & 0xff;
  sd->limit_high    = ((limit >> 16) & 0x0f) | ((access_right >> 8) & 0xf0);
  sd->base_high     = (base >> 24) & 0xff;
}

static void 
set_gate_descriptor(gate_descriptor_t* gd, 
    int offset, int selector, int access_right) 
{
  gd->offset_low    = offset & 0xffff;
  gd->selector      = selector;
  gd->dw_count      = (access_right >> 8) & 0xff;
  gd->access_right  = access_right & 0xff;
  gd->offset_high   = (offset >> 16) & 0xffff;
}


void 
init_gdt_idt(void)
{
  segment_descriptor_t* gdt = (segment_descriptor_t*)ADR_GDT;
  gate_descriptor_t* idt = (gate_descriptor_t*)ADR_IDT;
  int i;

  /* initialize GDT (13bits 0~8191) */
  for (i = 0; i < 8192; ++i)
    set_segment_descriptor(gdt + i, 0, 0, 0);
  set_segment_descriptor(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
  set_segment_descriptor(gdt + 2, 
      LIMIT_BOOTPACK, ADR_BOOTPACK, AR_CODE32_ER);
  load_gdtr(LIMIT_GDT, ADR_GDT);

  /* initialize IDT */
  for (i = 0; i < LIMIT_IDT / 8; ++i) 
    set_gate_descriptor(idt + i, 0, 0, 0);
  load_idtr(LIMIT_IDT, ADR_IDT);

  /* IDT setting */
  set_gate_descriptor(idt + 0x21, 
      (int)asm_interrupt_handler21, 2 * 8, AR_INTGATE32);
  set_gate_descriptor(idt + 0x27, 
      (int)asm_interrupt_handler27, 2 * 8, AR_INTGATE32);
  set_gate_descriptor(idt + 0x2c, 
      (int)asm_interrupt_handler2c, 2 * 8, AR_INTGATE32);
}
