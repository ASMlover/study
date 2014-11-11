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

#define DEF_STORAGE (128)

typedef int ElementType;

typedef struct SequentialStack {
  int top;
  int storage;
  ElementType* elements;
} SequentialStack;

static SequentialStack* stack_init(int storage) {
  SequentialStack* stack = (SequentialStack*)malloc(sizeof(*stack));

  if (NULL != stack) {
    stack->top = 0;
    stack->storage = (storage > DEF_STORAGE ? storage : DEF_STORAGE);
    stack->elements = (ElementType*)malloc(
        sizeof(ElementType) * stack->storage);
    if (NULL == stack->elements) {
      free(stack);
      stack = NULL;
    }
  }

  return stack;
}

static void stack_destroy(SequentialStack** stack) {
  assert(NULL != *stack);

  free((*stack)->elements);
  free(*stack);
  *stack = NULL;
}

static int stack_empty(SequentialStack* stack) {
  assert(NULL != stack);

  return (0 == stack->top);
}

static void stack_push(SequentialStack* stack, ElementType value) {
  assert(NULL != stack);

  if (stack->top < stack->storage)
    stack->elements[stack->top++] = value;
}

static void stack_pop(SequentialStack* stack) {
  assert(NULL != stack);

  if (stack->top > 0)
    --stack->top;
}

static ElementType stack_top(SequentialStack* stack) {
  assert(NULL != stack && stack->top > 0);

  return stack->elements[stack->top - 1];
}

void sequential_stack(void) {
  SequentialStack* stack = stack_init(DEF_STORAGE);
  int i;

  fprintf(stdout, "\n**************SequentialStack**************\n");
  for (i = 0; i < 10; ++i)
    stack_push(stack, i * 2);

  while (!stack_empty(stack)) {
    fprintf(stdout, "SequentialStack element value is : %d\n",
        stack_top(stack));
    stack_pop(stack);
  }

  stack_destroy(&stack);
}
