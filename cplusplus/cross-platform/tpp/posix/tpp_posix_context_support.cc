// Copyright (c) 2016 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <sys/mman.h>
#include "../tpp_context_support.h"

namespace tpp {

__thread uint32_t t_protect_stack_page = 0;

int __libtpp_protect_stack(void* top, uint32_t stack_size, uint32_t page) {
  if (0 == page)
    return -1;

  uint32_t pagesize = getpagesize();
  if (stack_size <= pagesize * (page + 1))
    return -1;

  void* protect_addr = ((uint32_t)top & 0xfff) ?
    (void*)(((uint32_t)top & ~(uint32_t)0xfff) + 0x1000) : top;
  if (-1 == mprotect(protect_addr, pagesize * page, PROT_NONE))
    return -1;

  return 0;
}

void __libtpp_unprotect_stack(void* top, uint32_t page) {
  if (0 == page)
    return;

  void* protect_addr = ((uint32_t)top & 0xfff) ?
    (void*)(((uint32_t)top & ~(uint32_t)0xfff) + 0x1000) : top;
  mprotect(protect_addr, getpagesize() * page, PROT_READ | PROT_WRITE);
}

uint32_t __libtpp_get_protect_stack_page(void) {
  return t_protect_stack_page;
}

void __libtpp_set_protect_stack_page(uint32_t page) {
  t_protect_stack_page = page;
}

}
