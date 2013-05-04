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
#ifndef __DESCRIPTOR_TABLE_HEADER_H__
#define __DESCRIPTOR_TABLE_HEADER_H__

typedef struct segment_descriptor_s {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
} segment_descriptor_t;

typedef struct gate_descriptor_s {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
} gate_descriptor_t;


#define ADR_IDT         (0x0026f800)
#define LIMIT_IDT       (0x000007ff)
#define ADR_GDT         (0x00270000)
#define LIMIT_GDT       (0x0000ffff)
#define ADR_BOOTPACK    (0x00280000)
#define LIMIT_BOOTPACK  (0x0007ffff)
#define AR_DATA32_RW    (0x4092)
#define AR_CODE32_ER    (0x409a)


extern void init_gdt_idt(void);

#endif  /* __DESCRIPTOR_TABLE_HEADER_H__ */
