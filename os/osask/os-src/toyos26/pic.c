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
#include "pic.h"


void 
init_pic(void)
{
  io_out8(PIC0_IMR, 0xff);    /* disabled all interruptors */
  io_out8(PIC1_IMR, 0xff);    /* disabled all interruptors */

  io_out8(PIC0_ICW1, 0x11);   /* edge trigegr mode */
  io_out8(PIC0_ICW2, 0x20);   /* IRQ0~7 received by INT20~27 */
  io_out8(PIC0_ICW3, 1 << 2); /* PIC1 connected by IRQ2 */
  io_out8(PIC0_ICW4, 0x01);   /* no-cached mode */

  io_out8(PIC1_ICW1, 0x11);   /* edge trigegr mode */
  io_out8(PIC1_ICW2, 0x28);   /* IRQ8~15 received by INT28~2f */
  io_out8(PIC1_ICW3, 2);      /* PIC1 connected by IRQ2 */
  io_out8(PIC1_ICW4, 0x01);   /* no-cached mode */

  io_out8(PIC0_IMR, 0xfb);    /* 11111011 disabled outside PIC1 */
  io_out8(PIC1_IMR, 0xff);    /* 11111111 disabled all interruptors */
}
