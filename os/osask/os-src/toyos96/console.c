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
#include <string.h>
#include "common.h"
#include "boot.h"
#include "graphic.h"
#include "memory.h"
#include "fifo.h"
#include "timer.h"
#include "multitask.h"
#include "window.h"
#include "file.h"
#include "console.h"




static int 
memory_command(layer_t* layer, mem_mgr_t* memmgr, 
    unsigned int mem_total, int cursor_y)
{
  char buf[32];

  sprintf(buf, "total: %dMB", mem_total / (1024 * 1024));
  drawstring_and_refresh(layer, 8, cursor_y, 
      COLOR8_FFFFFF, COLOR8_000000, buf, 30);
  cursor_y = console_newline(cursor_y, layer);
  sprintf(buf, "free:  %dKB", mem_mgr_total(memmgr) / 1024);
  drawstring_and_refresh(layer, 8, cursor_y, 
      COLOR8_FFFFFF, COLOR8_000000, buf, 30);
  cursor_y = console_newline(cursor_y, layer);
  cursor_y = console_newline(cursor_y, layer);

  return cursor_y;
}

static int 
clear_command(layer_t* layer)
{
  /* clear command */
  int x, y;

  for (y = 28; y < 28 + 128; ++y) {
    for (x = 8; x < 8 + 240; ++x)
      layer->buf[x + y * layer->w_size] = COLOR8_000000;
  }

  layers_refresh(layer, 8, 28, 8 + 240, 28 + 128);
  return 28;
}

static int 
list_command(layer_t* layer, int cursor_y)
{
  /* list command */
  fileinfo_t* finfo = (fileinfo_t*)(ADR_DISKIMG + 0x002600);
  int x, y;
  char buf[32];

  for (x = 0; x < 224; ++x) {
    if (0x00 == finfo[x].name[0])
      break;

    if (0xe5 != finfo[x].name[0]) {
      if (0 == (finfo[x].type & 0x18)) {
        sprintf(buf, "filename.ext   %7d", finfo[x].size);
        for (y = 0; y < 8; ++y)
          buf[y] = finfo[x].name[y];
        buf[9]  = finfo[x].ext[0];
        buf[10] = finfo[x].ext[1];
        buf[11] = finfo[x].ext[2];

        drawstring_and_refresh(layer, 8, cursor_y, 
            COLOR8_FFFFFF, COLOR8_000000, buf, 30);
        cursor_y = console_newline(cursor_y, layer);
      }
    }
  }

  return console_newline(cursor_y, layer);
}

static int 
cat_command(mem_mgr_t* memmgr, layer_t* layer, 
    int cursor_x, int cursor_y, char* cmdline)
{
  /* cat command */
  int* fat = (int*)mem_mgr_alloc_4k(memmgr, 4 * 2880);
  fileinfo_t* finfo = (fileinfo_t*)(ADR_DISKIMG + 0x002600);
  int x, y;
  char buf[32];
  char* p;

  file_readfat(fat, (unsigned char*)(ADR_DISKIMG + 0x000200));

  for (y = 0; y < 11; ++y)
    buf[y] = ' ';
  y = 0;
  for (x = 4; y < 11 && 0 != cmdline[x]; ++x) {
    if ('.' == cmdline[x] && y <= 8)
      y = 8;
    else {
      buf[y] = cmdline[x];
      if ('a' <= buf[y] && buf[y] <= 'z')
        buf[y] -= 0x20;
      ++y;
    }
  }

  /* find file */
  for (x = 0; x < 224; ) {
    if (0x00 == finfo[x].name[0])
      break;
    if (0 == (finfo[x].type & 0x18)) {
      for (y = 0; y < 11; ++y) {
        if (finfo[x].name[y] != buf[y])
          goto __cat_next_file;
      }
      break;
    }
__cat_next_file:
    ++x;
  }
  
  if (x < 224 && 0x00 != finfo[x].name[0]) {
    /* found the file */
    p = (char*)mem_mgr_alloc_4k(memmgr, finfo[x].size);
    file_loadfile(finfo[x].clustno, finfo[x].size, 
        p, fat, (char*)(ADR_DISKIMG + 0x003e00));
    cursor_x = 8;

    for (y = 0; y < finfo[x].size; ++y) {
      /* output the file one character by one character */
      buf[0] = p[y];
      buf[1] = 0;
      if (0x09 == buf[0]) {
        /* tab */
        for ( ; ; ) {
          drawstring_and_refresh(layer, cursor_x, cursor_y, 
              COLOR8_FFFFFF, COLOR8_000000, " ", 1);
          cursor_x += 8;
          if ((8 + 240) == cursor_x) {
            cursor_x = 8;
            cursor_y = console_newline(cursor_y, layer);
          }
          if (0 == ((cursor_x - 8) & 0x1f))
            break;
        }
      }
      else if (0x0a == buf[0]) {
        /* Enter */
        cursor_x = 8;
        cursor_y = console_newline(cursor_y, layer);
      }
      else if (0x0d == buf[0]) {
        /* pass */
      }
      else {
        /* normal characters */
        drawstring_and_refresh(layer, cursor_x, cursor_y, 
            COLOR8_FFFFFF, COLOR8_000000, buf, 1);
        cursor_x += 8;
        if ((8 + 240) == cursor_x) {
          cursor_x = 8;
          cursor_y = console_newline(cursor_y, layer);
        }
      }
    }
    mem_mgr_free_4k(memmgr, (int)p, finfo[x].size);
  }
  else {
    /* can't find the file */
    drawstring_and_refresh(layer, 8, cursor_y, 
        COLOR8_FFFFFF, COLOR8_000000, "File not fount.", 15);
    cursor_y = console_newline(cursor_y, layer);
  }

  return console_newline(cursor_y, layer);
}

  
  
  
  
int 
console_newline(int cursor_y, layer_t* layer)
{
  int x, y;

  if (cursor_y < 28 + 112)
    cursor_y += 16;   /* new line */
  else {
    /* scroll window */
    for (y = 28; y < 28 + 112; ++y) {
      for (x = 8; x < 8 + 240; ++x) {
        layer->buf[x + y * layer->w_size] = 
          layer->buf[x + (y + 16) * layer->w_size];
      }
    }
    for (y = 28 + 112; y < 28 + 128; ++y) {
      for (x = 8; x < 8 + 240; ++x)
        layer->buf[x + y * layer->w_size] = COLOR8_000000;
    }

    layers_refresh(layer, 8, 28, 8 + 240, 28 + 128);
  }

  return cursor_y;
}

void 
console_task(layer_t* layer, unsigned int mem_total)
{
#define INPUT_BEG_POS_X   (24)
#define INPUT_BEG_POS_Y   (28)
#define PROMPT_CHAR       ("> ")
#define PROMPT_LEN        (2)
  timer_t* timer;
  task_t*  task = task_now();
  int data, fifobuf[128];
  int cursor_x = INPUT_BEG_POS_X, cursor_y = INPUT_BEG_POS_Y; 
  int cursor_c = -1;
  char buf[32], cmdline[32];
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;
  
  fifo_init(&task->fifo, fifobuf, 128, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settimer(timer, 50);

  /* display the prompt of console window */
  drawstring_and_refresh(layer, 8, 28, 
      COLOR8_FFFFFF, COLOR8_000000, PROMPT_CHAR, PROMPT_LEN);

  for ( ; ; ) {
    io_cli();

    if (0 == fifo_size(&task->fifo)) {
      task_sleep(task);
      io_sti();
    }
    else {
      data = fifo_get(&task->fifo);
      io_sti();

      if (data <= 1) {
        /* timer for cursor */
        if (0 != data) {
          timer_init(timer, &task->fifo, 0);
          if (cursor_c >= 0)
            cursor_c = COLOR8_FFFFFF;
        }
        else {
          timer_init(timer, &task->fifo, 1);
          if (cursor_c >= 0)
            cursor_c = COLOR8_000000;
        }

        timer_settimer(timer, 50);
      }
      if (2 == data)  /* cursor ON */
        cursor_c = COLOR8_FFFFFF;
      if (3 == data) {  /* cursor OFF */
        fill_box8(layer->buf, layer->w_size,
            COLOR8_000000, cursor_x, 28, cursor_x + 7, cursor_y + 15);
        cursor_c = -1;
      }
      if (256 <= data && data <= 511) {
        /* keyboard data */
        if ((8 + 256) == data) {
          /* backspace */
          if (cursor_x > INPUT_BEG_POS_X) {
            /* erase the cursor and move forward one character */
            drawstring_and_refresh(layer, cursor_x, cursor_y, 
                COLOR8_FFFFFF, COLOR8_000000, " ", 1);
            cursor_x -= 8;
          }
        }
        else if ((10 + 256) == data) {
          /* Enter Key */
          /* erase cursor by space */
          drawstring_and_refresh(layer, cursor_x, cursor_y, 
              COLOR8_FFFFFF, COLOR8_000000, " ", 1);
          
          cmdline[cursor_x / 8 - 3] = 0;
          cursor_y = console_newline(cursor_y, layer);

          /* execute command */
          if (0 == strcmp("mem", cmdline)) {
            /* check memory command */
            cursor_y = memory_command(layer, memmgr, mem_total, cursor_y);
          }
          else if (0 == strcmp("clear", cmdline)) {
            /* clear command */
            cursor_y = clear_command(layer);
          }
          else if (0 == strcmp("ls", cmdline)) {
            /* ls(dir) command */
            cursor_y = list_command(layer, cursor_y);
          }
          else if (0 == strncmp("cat ", cmdline, 4)) {
            /* cat command */
            cursor_y = cat_command(memmgr, 
                layer, cursor_x, cursor_y, cmdline);
          }
          else if (0 != cmdline[0]) {
            /* neither command nor null string */
            drawstring_and_refresh(layer, 8, cursor_y, 
                COLOR8_FFFFFF, COLOR8_000000, "Bad Command.", 12);
            cursor_y = console_newline(cursor_y, layer);
            cursor_y = console_newline(cursor_y, layer);
          }

          /* show prompt */
          drawstring_and_refresh(layer, 8, cursor_y, 
              COLOR8_FFFFFF, COLOR8_000000, PROMPT_CHAR, PROMPT_LEN);
          cursor_x = INPUT_BEG_POS_X;
        }
        else {
          /* general character */
          if (cursor_x < 240) {
            /* display one character and move backward one character */
            buf[0] = data - 256;
            buf[1] = 0;
            cmdline[cursor_x / 8 - 3] = data - 256;
            drawstring_and_refresh(layer, cursor_x, cursor_y, 
                COLOR8_FFFFFF, COLOR8_000000, buf, 1);
            cursor_x += 8;
          }
        }
      }
      /* show the cursor again */
      if (cursor_c >= 0) {
        fill_box8(layer->buf, layer->w_size, 
            cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
      }
      layers_refresh(layer, cursor_x, cursor_y, 
          cursor_x + 8, cursor_y + 16);
    }
  }
}
