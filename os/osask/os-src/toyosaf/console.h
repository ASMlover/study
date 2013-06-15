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
#ifndef __CONSOLE_HEADER_H__
#define __CONSOLE_HEADER_H__ 

#include "layer.h"

typedef struct console_t console_t;
struct console_t {
  layer_t* layer;
  int cur_x, cur_y, cur_c;
};

extern void console_task(layer_t* layer, unsigned int memtotal);
extern void console_newline(console_t* console);
extern void console_putchar(console_t* console, int c, char move);
extern void console_runcmd(console_t* console, 
    char* cmdline, int* fat, unsigned int memtotal);
extern void cmd_mem(console_t* console, unsigned int memtotal);
extern void cmd_clear(console_t* console);
extern void cmd_ls(console_t* console);
extern void cmd_cat(console_t* console, int* fat, char* cmdline);
extern int cmd_app(console_t* console, int* fat, char* cmdline);

extern void console_putstr0(console_t* console, char* str);
extern void console_putstr1(console_t* console, char* str, int len);
extern int* toy_api(int edi, int esi, int ebp, 
    int esp, int ebx, int edx, int ecx, int eax);

extern int* interrupt_handler0c(int* esp);
extern int* interrupt_handler0d(int* esp);

extern void toy_api_drawline(layer_t* layer, 
    int x0, int y0, int x1, int y1, int color);

#endif  /* __CONSOLE_HEADER_H__ */
