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
#include "desc_tbl.h"
#include "fifo.h"
#include "timer.h"
#include "multitask.h"
#include "window.h"
#include "file.h"
#include "console.h"




void 
console_task(layer_t* layer, unsigned int memtotal)
{
#define INPUT_BEG_POS_X   (16)
#define INPUT_BEG_POS_Y   (28)
  timer_t* timer;
  task_t* task = task_now();
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;
  int data, fifobuf[128];
  int* fat = (int*)mem_mgr_alloc_4k(memmgr, 4 * 2880);
  char cmdline[32];
  console_t console;
  console.layer = layer;
  console.cur_x = 8;
  console.cur_y = INPUT_BEG_POS_Y;
  console.cur_c = -1;
  *((int*)0x0fec) = (int)&console;
  
  
  fifo_init(&task->fifo, fifobuf, 128, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settimer(timer, 50);
  file_readfat(fat, (unsigned char*)(ADR_DISKIMG + 0x000200));

  /* display the prompt of console window */
  console_putchar(&console, '>', 1);

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
          if (console.cur_c >= 0)
            console.cur_c = COLOR8_FFFFFF;
        }
        else {
          timer_init(timer, &task->fifo, 1);
          if (console.cur_c >= 0)
            console.cur_c = COLOR8_000000;
        }

        timer_settimer(timer, 50);
      }
      if (2 == data)  /* cursor ON */
        console.cur_c = COLOR8_FFFFFF;
      if (3 == data) {  /* cursor OFF */
        fill_box8(layer->buf, layer->w_size,
            COLOR8_000000, console.cur_x, console.cur_y, 
            console.cur_x + 7, console.cur_y + 15);
        console.cur_c = -1;
      }
      if (256 <= data && data <= 511) {
        /* keyboard data */
        if ((8 + 256) == data) {
          /* backspace */
          if (console.cur_x > INPUT_BEG_POS_X) {
            /* erase the cursor and move forward one character */
            console_putchar(&console, ' ', 0);
            console.cur_x -= 8;
          }
        }
        else if ((10 + 256) == data) {
          /* Enter Key */
          /* erase cursor by space */
          console_putchar(&console, ' ', 0);
          
          cmdline[console.cur_x / 8 - 2] = 0;
          console_newline(&console);
          console_runcmd(&console, cmdline, fat, memtotal);

          /* show the prompt */
          console_putchar(&console, '>', 1);
        }
        else {
          /* general character */
          if (console.cur_x < 240) {
            /* display one character and move backward one character */
            cmdline[console.cur_x / 8 - 2] = data - 256;
            console_putchar(&console, data - 256, 1);
          }
        }
      }
      /* show the cursor again */
      if (console.cur_c >= 0) {
        fill_box8(layer->buf, layer->w_size, 
            console.cur_c, console.cur_x, console.cur_y, 
            console.cur_x + 7, console.cur_y + 15);
      }
      layers_refresh(layer, console.cur_x, console.cur_y, 
          console.cur_x + 8, console.cur_y + 16);
    }
  }
}

void 
console_newline(console_t* console)
{
  int x, y;
  layer_t* layer = console->layer;

  if (console->cur_y < 28 + 112)
    console->cur_y += 16;   /* new line */
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
  console->cur_x = 8;
}

void 
console_putchar(console_t* console, int c, char move)
{
  char buf[2];

  buf[0] = c;
  buf[1] = 0;

  if (0x09 == buf[0]) {
    /* TAB */
    for ( ; ; ) {
      drawstring_and_refresh(console->layer, console->cur_x, 
          console->cur_y, COLOR8_FFFFFF, COLOR8_000000, " ", 1);
      console->cur_x += 8;
      if ((8 + 240) == console->cur_x)
        console_newline(console);
      if (0 == ((console->cur_x - 8) & 0x1f))
        break;
    }
  }
  else if (0x0a == buf[0])  /* Enter */
    console_newline(console);
  else if (0x0d == buf[0]) {
    /* pass */
  }
  else {
    /* normal characters */
    drawstring_and_refresh(console->layer, console->cur_x, 
        console->cur_y, COLOR8_FFFFFF, COLOR8_000000, buf, 1);
    if (0 != move) {
      /* donot move cursor when move is 0 */
      console->cur_x += 8;
      if ((8 + 240) == console->cur_x)
        console_newline(console);
    }
  }
}

void 
console_runcmd(console_t* console, 
    char* cmdline, int* fat, unsigned int memtotal)
{
  /* execute command */
  if (0 == strcmp("mem", cmdline)) {
    /* check memory command */
    cmd_mem(console, memtotal);
  }
  else if (0 == strcmp("clear", cmdline)) {
    /* clear command */
    cmd_clear(console);
  }
  else if (0 == strcmp("ls", cmdline)) {
    /* ls(dir) command */
    cmd_ls(console);
  }
  else if (0 == strncmp("cat ", cmdline, 4)) {
    /* cat command */
    cmd_cat(console, fat, cmdline);
  }
  else if (0 == strcmp("hlt", cmdline)) {
    /* run hlt application */
    cmd_hlt(console, fat);
  }
  else if (0 != cmdline[0]) {
    /* neither command nor null string */
    drawstring_and_refresh(console->layer, 8, console->cur_y, 
        COLOR8_FFFFFF, COLOR8_000000, "Bad Command.", 12);
    console_newline(console);
    console_newline(console);
  }
}

void 
cmd_mem(console_t* console, unsigned int memtotal)
{
  char buf[32];
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;

  sprintf(buf, "total: %dMB", memtotal / (1024 * 1024));
  drawstring_and_refresh(console->layer, 8, console->cur_y, 
      COLOR8_FFFFFF, COLOR8_000000, buf, 30);
  console_newline(console);
  sprintf(buf, "free:  %dKB", mem_mgr_total(memmgr) / 1024);
  drawstring_and_refresh(console->layer, 8, console->cur_y, 
      COLOR8_FFFFFF, COLOR8_000000, buf, 30);
  console_newline(console);
  console_newline(console);
}

void 
cmd_clear(console_t* console)
{
  /* clear command */
  int x, y;
  layer_t* layer = console->layer;

  for (y = 28; y < 28 + 128; ++y) {
    for (x = 8; x < 8 + 240; ++x)
      layer->buf[x + y * layer->w_size] = COLOR8_000000;
  }

  layers_refresh(layer, 8, 28, 8 + 240, 28 + 128);
  console->cur_y = 28;
}

void 
cmd_ls(console_t* console)
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

        drawstring_and_refresh(console->layer, 8, console->cur_y, 
            COLOR8_FFFFFF, COLOR8_000000, buf, 30);
        console_newline(console);
      }
    }
  }

  console_newline(console);
}

void 
cmd_cat(console_t* console, int* fat, char* cmdline)
{
  /* cat command */
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;
  fileinfo_t* finfo = 
    file_search((fileinfo_t*)(ADR_DISKIMG + 0x002600), cmdline + 4, 224);
  char* p;
  int i;

  if (0 != finfo) {
    /* found the file */
    p = (char*)mem_mgr_alloc_4k(memmgr, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, 
        p, fat, (char*)(ADR_DISKIMG + 0x003e00));
    for (i = 0; i < finfo->size; ++i)
      console_putchar(console, p[i], 1);
    mem_mgr_free_4k(memmgr, (int)p, finfo->size);
  }
  else {
    /* donot find the file */
    drawstring_and_refresh(console->layer, 8, console->cur_y, 
        COLOR8_FFFFFF, COLOR8_000000, "File not found.", 15);
    console_newline(console);
  }
  console_newline(console);
}

void 
cmd_hlt(console_t* console, int* fat)
{
  /* hlt command */
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;
  fileinfo_t* finfo = 
    file_search((fileinfo_t*)(ADR_DISKIMG + 0x002600), "hlt.toy", 224);
  segment_descriptor_t* gdt = (segment_descriptor_t*)ADR_GDT;
  char* p;

  if (0 != finfo) {
    /* found the file */
    p = (char*)mem_mgr_alloc_4k(memmgr, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, 
        p, fat, (char*)(ADR_DISKIMG + 0x003e00));
    set_segment_descriptor(gdt + 1003, 
        finfo->size - 1, (int)p, AR_CODE32_ER);
    farcall(0, 1003 * 8);
    mem_mgr_free_4k(memmgr, (int)p, finfo->size);
  }
  else {
    /* donot find the file */
    drawstring_and_refresh(console->layer, 8, console->cur_y, 
        COLOR8_FFFFFF, COLOR8_000000, "File not found.", 15);
    console_newline(console);
  }
  console_newline(console);
}
