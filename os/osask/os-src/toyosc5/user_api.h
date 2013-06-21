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
#ifndef __USER_API_HEADER_H__
#define __USER_API_HEADER_H__

extern void putc(int c);
extern void puts(const char* str);
extern void api_end(void);
extern void init_mem(void);
extern void* malloc(int size);
extern void free(void* ptr, int size);
extern int getkey(int mode);
extern int win_open(char* buf, int w, int h, int alpha, char* title);
extern void win_close(int win);
extern void win_puts(int win, int x, int y, int color, int len, char* str);
extern void win_fill(int win, int x0, int y0, int x1, int y1, int color);
extern void win_point(int win, int x, int y, int color);
extern void win_refresh(int win, int x0, int y0, int x1, int y1);
extern void win_line(int win, int x0, int y0, int x1, int y1, int color);
extern int create_timer(void);
extern void delete_timer(int timer);
extern void init_timer(int timer, int data);
extern void set_timer(int timer, int time);
extern void beep(int tone);

#endif  /* __USER_API_HEADER_H__ */
