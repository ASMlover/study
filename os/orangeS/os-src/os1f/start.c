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

extern void* memcpy(void* dest, void* src, int size);
extern void display_str(char* str);

uint8_t gdt_ptr[6]; /* 0~15:limit 16~47:base */
descriptor_t  gdt[GDT_SIZE];



void 
cstart(void)
{
  uint16_t* p_gdt_limit;
  uint32_t* p_gdt_base;

  display_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
      "=====<cstart> begins=====\n");

  /* copy GDT from LOADER to new GDT */
  memcpy(&gdt, (void*)(*((uint32_t*)(&gdt_ptr[2]))), /* base of old GDT */
      *((uint16_t*)(&gdt_ptr[0])) + 1); /* limit of old GDT */
  
  p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
  p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
  *p_gdt_limit  = GDT_SIZE * sizeof(descriptor_t) - 1;
  *p_gdt_base   = (uint32_t)&gdt;
}
