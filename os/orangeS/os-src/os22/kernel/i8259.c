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
#include "proto.h"



void 
init_8259A(void)
{
  out_byte(INT_M_CTL, 0x11);  /* master 8259, ICW1 */
  out_byte(INT_S_CTL, 0x11);  /* slave 8259, ICW1 */

  out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0); /* setting main 8259 */
  out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8); /* setting sub 8259 */

  out_byte(INT_M_CTLMASK, 0x4); /* master 8259, ICW3, IR2->sub 8259 */
  out_byte(INT_S_CTLMASK, 0x2); /* slave 8259, ICW3 IR2->main 8259 */

  out_byte(INT_M_CTLMASK, 0x1); /* master 8259 ICW4 */
  out_byte(INT_S_CTLMASK, 0x1); /* slave 8259, ICW4 */
  out_byte(INT_M_CTLMASK, 0xFF);  /* master 8259, OCW1 */
  out_byte(INT_S_CTLMASK, 0xFF);  /* slave 8259, OCW1 */
}
