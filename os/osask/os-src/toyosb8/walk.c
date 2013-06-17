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
#include "user_api.h"


void 
HariMain(void)
{
  char* buf;
  int win;
  int key, x, y;

  init_mem();

  buf = (char*)malloc(160 * 100);
  win = win_open(buf, 160, 100, -1, "WIN-WALK");
  win_fill(win, 4, 24, 155, 95, 0 /* black */);
  x = 76;
  y = 56;

  win_puts(win, x, y, 3 /* yellow */, 1, "*");
  for ( ; ; ) {
    key = getkey(1);
    win_puts(win, x, y, 0 /* black */, 1, "*");
    if ('a' == key && x > 4)
      x -= 8;
    if ('d' == key && x < 148)
      x += 8;
    if ('w' == key && y > 24)
      y -= 8;
    if ('s' == key && y < 80)
      y += 8;

    if (0x0a == key)
      break;  /* quit */

    win_puts(win, x, y, 3 /* yellow */, 1, "*");
  }
  win_close(win);

  api_end();
}
