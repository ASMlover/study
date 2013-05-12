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
#include "common.h"
#include "boot.h"
#include "graphic.h"
#include "desc_tbl.h"
#include "pic.h"
#include "fifo.h"
#include "keyboard.h"
#include "mouse.h"



extern fifo8_t g_keybuf;
extern fifo8_t g_mousebuf;

#define EFLAGS_AC_BIT       (0x00040000)
#define CR0_CACHE_DISABLE   (0x60000000)

#define MEMMGR_FREES        (4090)  /* about 32KB */
#define MEMMGR_ADDR         (0x003c0000)


typedef struct free_info_t {  /* available information */
  unsigned int addr;
  unsigned int size;
} free_info_t;

typedef struct mem_mgr_t {
  int frees;      /* available information number */
  int max_frees;  /* max value of free */
  int fail_frees; /* total memory size of free failed */
  int fail_num;   /* number of free failed */
  free_info_t free_list[MEMMGR_FREES];
} mem_mgr_t;


void 
mem_mgr_init(mem_mgr_t* mgr)
{
  mgr->frees      = 0;
  mgr->max_frees  = 0;
  mgr->fail_frees = 0;
  mgr->fail_num   = 0;
}

unsigned int 
mem_mgr_total(mem_mgr_t* mgr)
{
  /* get free memory size */
  unsigned int i, total = 0;
  for (i = 0; i < mgr->frees; ++i)
    total += mgr->free_list[i].size;

  return total;
}

unsigned int 
mem_mgr_alloc(mem_mgr_t* mgr, unsigned int size)
{
  unsigned int i, address;

  for (i = 0; i < mgr->frees; ++i) {
    if (mgr->free_list[i].size >= size) {
      /* find enough memory space */
      address = mgr->free_list[i].addr;
      mgr->free_list[i].addr += size;
      mgr->free_list[i].size -= size;

      if (0 == mgr->free_list[i].size) {
        /* take away an available memory information */
        --mgr->frees;
        for ( ; i < mgr->frees; ++i)
          mgr->free_list[i] = mgr->free_list[i + 1];
      }

      return address;
    }
  }

  return 0;   /* no space can be used */
}

int 
mem_mgr_free(mem_mgr_t* mgr, unsigned int addr, unsigned int size)
{
  int i, j;

  /* arrangement of memory order by address */
  for (i = 0; i < mgr->frees; ++i) {
    if (mgr->free_list[i].addr > addr)
      break;
  }

  /* if free_list[i - 1].addr < addr < free_list[i].addr */
  if (i > 0) {
    if (mgr->free_list[i - 1].addr + mgr->free_list[i - 1].size == addr) {
      mgr->free_list[i - 1].size += size;
      if (i < mgr->frees) {
        if (addr + size == mgr->free_list[i].addr) {
          mgr->free_list[i - 1].size += mgr->free_list[i].size;
          --mgr->frees;
          for ( ; i < mgr->frees; ++i)
            mgr->free_list[i] = mgr->free_list[i + 1];
        }
      }

      return 0; /* success */
    }
  }

  /* can't be merged together with front */
  if (i < mgr->frees) {
    if (addr + size == mgr->free_list[i].addr) {
      mgr->free_list[i].addr = addr;
      mgr->free_list[i].size += size;

      return 0;
    }
  }

  /* can't be merged together with front and back */
  if (mgr->frees < MEMMGR_FREES) {
    for (j = mgr->frees; j > i; --j) 
      mgr->free_list[j] = mgr->free_list[j - 1];
    
    ++mgr->frees;
    if (mgr->max_frees < mgr->frees)
      mgr->max_frees = mgr->frees;

    mgr->free_list[i].addr = addr;
    mgr->free_list[i].size = size;
    return 0;
  }

  /* can't move to back */
  ++mgr->fail_num;
  mgr->fail_frees += size;
  return -1;
}






static unsigned int 
memory_test(unsigned int start, unsigned int end)
{
  char flag486 = 0;
  unsigned int eflags, cr0, address;

  /* check the type of CPU (386? 486?) */
  eflags = io_load_eflags();
  eflags |= EFLAGS_AC_BIT;  /* AC-bit = 1 */
  io_store_eflags(eflags);
  if (0 != (eflags & EFLAGS_AC_BIT))  /* if 386, set AC=1, AC back to 0 */
    flag486 = 1;
  eflags &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
  io_store_eflags(eflags);

  if (0 != flag486) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;   /* disable cache */
    store_cr0(cr0);
  }

  address = memory_test_sub(start, end);

  if (0 != flag486) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;  /* allow cache */
    store_cr0(cr0);
  }

  return address;
}


void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  char debug_info[64], mouse_cursor[256], keybuf[32], mousebuf[128];
  int mouse_x, mouse_y;
  int data;
  mouse_dec_t mdec;
  unsigned int memory_total;
  mem_mgr_t* mem_mgr = (mem_mgr_t*)MEMMGR_ADDR;

  init_gdt_idt();
  init_pic();
  io_sti();

  fifo_init(&g_keybuf, keybuf, 32);
  fifo_init(&g_mousebuf, mousebuf, 128);
  io_out8(PIC0_IMR, 0xf9);
  io_out8(PIC1_IMR, 0xef);

  init_keyboard();      /* initialize keyboard */
  enable_mouse(&mdec);  /* enabled mouse */

  memory_total = memory_test(0x00400000, 0xbfffffff);
  mem_mgr_init(mem_mgr);
  mem_mgr_free(mem_mgr, 0x00001000, 0x0009e000);  /*0x00001000~0x0009e000*/
  mem_mgr_free(mem_mgr, 0x00400000, memory_total - 0x00400000);

  init_palette();
  init_screen(binfo->vram, binfo->screen_x, binfo->screen_y);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;
  init_mouse_cursor8(mouse_cursor, COLOR8_848484);
  draw_block8_8(binfo->vram, binfo->screen_x, 16, 16, 
      mouse_x, mouse_y, mouse_cursor, 16);
  sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
  draw_font8_asc(binfo->vram, binfo->screen_x, 0, 0, 
      COLOR8_FFFFFF, debug_info);

  sprintf(debug_info, "memory total: %dMB, free space: %dKB", 
      memory_total / (1024 * 1024), mem_mgr_total(mem_mgr) / 1024);
  draw_font8_asc(binfo->vram, binfo->screen_x, 
      0, 32, COLOR8_FFFFFF, debug_info);

  for ( ; ; ) {
    io_cli();

    if (0 == fifo_size(&g_keybuf) && 0 == fifo_size(&g_mousebuf)) 
      io_stihlt();
    else {
      if (0 != fifo_size(&g_keybuf)) {
          data = fifo_get(&g_keybuf);

          io_sti();
          sprintf(debug_info, "%02X", data);
          fill_box8(binfo->vram, binfo->screen_x, 
            COLOR8_848484, 0, 16, 15, 31);
          draw_font8_asc(binfo->vram, binfo->screen_x, 
            0, 16, COLOR8_FFFFFF, debug_info);
      }
      else if (0 != fifo_size(&g_mousebuf)) {
        data = fifo_get(&g_mousebuf);
        io_sti();

        if (0 != mouse_decode(&mdec, data)) {
          /* show all mouse bytes code */
          sprintf(debug_info, "[lcr %4d %4d]", mdec.x, mdec.y);
          if (0 != (mdec.state & 0x01)) 
            debug_info[1] = 'L';
          if (0 != (mdec.state & 0x02))
            debug_info[3] = 'R';
          if (0 != (mdec.state & 0x04))
            debug_info[2] = 'C';
          fill_box8(binfo->vram, binfo->screen_x, 
              COLOR8_848484, 32, 16, 32 + 15 * 8 - 1, 31);
          draw_font8_asc(binfo->vram, binfo->screen_x, 
              32, 16, COLOR8_FFFFFF, debug_info);

          /* move mouse pointer */
          fill_box8(binfo->vram, binfo->screen_x, COLOR8_848484, 
              mouse_x, mouse_y, mouse_x + 15, mouse_y + 15);/* hide mouse */
          mouse_x += mdec.x;
          mouse_y += mdec.y;

          if (mouse_x < 0)
            mouse_x = 0;
          if (mouse_y < 0)
            mouse_y = 0;
          if (mouse_x > binfo->screen_x - 16)
            mouse_x = binfo->screen_x - 16;
          if (mouse_y > binfo->screen_y - 16)
            mouse_y = binfo->screen_y - 16;

          sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
          fill_box8(binfo->vram, binfo->screen_x, 
              COLOR8_848484, 0, 0, 79, 15);     /* hide mouse position */
          draw_font8_asc(binfo->vram, binfo->screen_x, 
              0, 0, COLOR8_FFFFFF, debug_info); /* show mouse position */
          draw_block8_8(binfo->vram, binfo->screen_x, 
              16, 16, mouse_x, mouse_y, mouse_cursor,  16); /* draw mouse */
        }
      }
    }
  }
}

