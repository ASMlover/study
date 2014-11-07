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

typedef struct DoubleStack {
  int top1;
  int top2;
  int storage;
  ElementType* elements;
} DoubleStack;


static DoubleStack* stack_init(int storage) {
  DoubleStack* stack = (DoubleStack*)malloc(sizeof(*stack));
  if (NULL == stack)
    return NULL;

  stack->storage = storage > DEF_STORAGE ? storage : DEF_STORAGE;
  stack->top1 = 0;
  stack->top2 = stack->storage - 1;

  stack->elements = (ElementType*)malloc(
      sizeof(ElementType) * stack->storage);
  if (NULL == stack->elements) {
    free(stack);
    stack = NULL;
  }

  return stack;
}

static void stack_destroy(DoubleStack** stack) {
  assert(NULL != *stack);

  if (NULL != (*stack)->elements)
    free((*stack)->elements);
  free(*stack);
  *stack = NULL;
}

static int stack_empty1(DoubleStack* stack) {
  assert(NULL != stack);

  return (0 == stack->top1);
}

static int stack_empty2(DoubleStack* stack) {
  assert(NULL != stack);

  return (stack->storage - 1 == stack->top2);
}

static void stack_push1(DoubleStack* stack, ElementType value) {
  assert(NULL != stack);

  if (stack->top1 + 1 < stack->top2) 
    stack->elements[stack->top1++] = value;
}

static void stack_push2(DoubleStack* stack, ElementType value) {
  assert(NULL != stack);

  if (stack->top1 < stack->top2 - 1)
    stack->elements[stack->top2--] = value;
}

static void stack_pop1(DoubleStack* stack) {
  assert(NULL != stack);

  if (stack->top1 > 0)
    --stack->top1;
}

static void stack_pop2(DoubleStack* stack) {
  assert(NULL != stack);

  if (stack->top2 < stack->storage - 1)
    ++stack->top2;
}

static ElementType stack_top1(DoubleStack* stack) {
  assert(NULL != stack && stack->top1 > 0);

  return stack->elements[stack->top1];
}

static ElementType stack_top2(DoubleStack* stack) {
  assert(NULL != stack && stack->top2 < stack->storage - 1);

  return stack->elements[stack->top2];
}

void double_stack(void) {
}
