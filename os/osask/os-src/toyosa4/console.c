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
  else if (0 != cmdline[0]) {
    if (0 == cmd_app(console, fat, cmdline)) {
      /* neither command nor null string */
      console_putstr0(console, "Bad Command.\n\n");
    }
  }
}

void 
cmd_mem(console_t* console, unsigned int memtotal)
{
  char buf[64];
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;

  sprintf(buf, "total: %dMB\nfree:  %dKB\n\n", 
      memtotal / (1024 * 1024), mem_mgr_total(memmgr) / 1024);
  console_putstr0(console, buf);
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
        sprintf(buf, "filename.ext   %7d\n", finfo[x].size);
        for (y = 0; y < 8; ++y)
          buf[y] = finfo[x].name[y];
        buf[9]  = finfo[x].ext[0];
        buf[10] = finfo[x].ext[1];
        buf[11] = finfo[x].ext[2];

        console_putstr0(console, buf);
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

  if (0 != finfo) {
    /* found the file */
    p = (char*)mem_mgr_alloc_4k(memmgr, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, 
        p, fat, (char*)(ADR_DISKIMG + 0x003e00));
    console_putstr1(console, p, finfo->size);
    mem_mgr_free_4k(memmgr, (int)p, finfo->size);
  }
  else {
    /* donot find the file */
    console_putstr0(console, "File not found.\n");
  }
  console_newline(console);
}

int 
cmd_app(console_t* console, int* fat, char* cmdline)
{
  mem_mgr_t* memmgr = (mem_mgr_t*)MEMMGR_ADDR;
  fileinfo_t* finfo;
  segment_descriptor_t* gdt = (segment_descriptor_t*)ADR_GDT;
  char name[18];
  char* p;
  char* user_app;
  int i;
  task_t* task = task_now();

  /* generate the filename by cmdline */
  for (i = 0; i < 13; ++i) {
    if (cmdline[i] <= ' ')
      break;
    name[i] = cmdline[i];
  }
  name[i] = 0;

  /* search the file */
  finfo = file_search((fileinfo_t*)(ADR_DISKIMG + 0x002600), name, 224);
  if (0 == finfo && '.' != name[i - 1]) {
    /* can't find the find, add ext and research again */
    name[i + 0] = '.';
    name[i + 1] = 't';
    name[i + 2] = 'o';
    name[i + 3] = 'y';
    name[i + 4] = 0;
    finfo = file_search((fileinfo_t*)(ADR_DISKIMG + 0x002600), name, 224);
  }

  if (0 != finfo) {
    /* find the file */
    p = (char*)mem_mgr_alloc_4k(memmgr, finfo->size);
    user_app = (char*)mem_mgr_alloc_4k(memmgr, 64 * 1024);
    *((int*)0x0fe8) = (int)p;
    file_loadfile(finfo->clustno, finfo->size, 
        p, fat, (char*)(ADR_DISKIMG + 0x003e00));
    set_segment_descriptor(gdt + 1003, 
        finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
    set_segment_descriptor(gdt + 1004, 
        64 * 1024 - 1, (int)user_app, AR_DATA32_RW + 0x60);
    if (finfo->size >= 8 && 0 == strncmp(p + 4, "Hari", 4)) {
      p[0] = 0xe8;
      p[1] = 0x16;
      p[2] = 0x00;
      p[3] = 0x00;
      p[4] = 0x00;
      p[5] = 0xcb;
    }
    start_user_app(0, 1003 * 8, 64 * 1024, 1004 * 8, &(task->tss.esp0));
    mem_mgr_free_4k(memmgr, (int)p, finfo->size);
    mem_mgr_free_4k(memmgr, (int)user_app, 64 * 1024);
    
    console_newline(console);
    return 1;
  }

  /* do not found the file */
  return 0;
}

void 
console_putstr0(console_t* console, char* str)
{
  for ( ; 0 != *str; ++str) 
    console_putchar(console, *str, 1);
}

void 
console_putstr1(console_t* console, char* str, int len)
{
  int i;
  for (i = 0; i < len; ++i)
    console_putchar(console, str[i], 1);
}

int* 
toy_api(int edi, int esi, int ebp, 
    int esp, int ebx, int edx, int ecx, int eax)
{
  int cs_base = *((int*)0x0fe8);
  task_t* task = task_now();
  struct console_t* console = (console_t*)(*((int*)0x0fec));
  switch (edx) {
  case 1:
    console_putchar(console, eax & 0xff, 1);
    break;
  case 2:
    console_putstr0(console, (char*)ebx + cs_base);
    break;
  case 3:
    console_putstr1(console, (char*)ebx + cs_base, ecx);
    break;
  case 4:
    return &(task->tss.esp0);
  }

  return 0;
}

int* 
interrupt_handler0d(int* esp)
{
  console_t* console = (console_t*)*((int*)0x0fec);
  task_t* task = task_now();
  console_putstr0(console, "\nint 0d :\n General protected exception.\n");
  return &task->tss.esp0; /* force to stop the application */
}
