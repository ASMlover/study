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
