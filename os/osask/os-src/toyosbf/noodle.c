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
#include <stdio.h>
#include "user_api.h"



void 
HariMain(void)
{
  char* buf;
  char str[32];
  int win, timer, sec = 0, min = 0, hour = 0;
  int key;

  init_mem();

  buf = (char*)malloc(150 * 50);
  win = win_open(buf, 150, 50, -1, "Win-NOODLE");
  timer = create_timer();
  init_timer(timer, 128);
  for ( ; ; ) {
    sprintf(str, "%05d:%02d:%02d", hour, min, sec);
    win_fill(win, 28, 27, 115, 41, 7 /* white */);
    win_puts(win, 28, 27, 0 /* black */, 11, str);
    
    set_timer(timer, 100);  /* 1 second */
    
    key = getkey(1);
    if (128 != key)
      break;

    ++sec;
    if (60 == sec) {
      sec = 0;
      ++min;
      if (60 == min) {
        min = 0;
        ++hour;
      }
    }
  }

  api_end();
}
