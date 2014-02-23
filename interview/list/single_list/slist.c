/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct List List;
struct List {
  List* next;
  int   val;
};


static List* s_head = NULL;

static void 
generate_list(int count)
{
  int i;
  List* n;

  for (i = 0; i < count; ++i) {
    n = (List*)malloc(sizeof(*n));
    assert(NULL != n);

    n->next = s_head;
    n->val = i;
    s_head = n;
  }
}

static void 
show_list(void)
{
  List* n = s_head;
  while (NULL != n) {
    fprintf(stdout, "list node value is : %d\n", n->val);
    n = n->next;
  }
}


static void 
reversal_list(void)
{
  List* head_tmp = s_head;
  List* it = NULL;
  List* n = NULL;

  if (NULL == s_head)
    return;

  it = s_head->next;
  while (NULL != it) {
    n = it->next;
    it->next = head_tmp;
    head_tmp = it;
    it = n;
  }

  s_head->next = NULL;
  s_head = head_tmp;
}



int 
main(int argc, char* argv[])
{
  generate_list(10);
  show_list();

  reversal_list();
  show_list();

  return 0;
}
