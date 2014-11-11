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

typedef int ElementType;

typedef struct Node {
  struct Node* next;
  ElementType  value;
} Node;

typedef struct Stack {
  Node* head;
} Stack;

static Node* node_create(ElementType value) {
  Node* node = (Node*)malloc(sizeof(*node));
  if (NULL != node) {
    node->next = NULL;
    node->value = value;
  }

  return node;
}

static void stack_clear(Stack* stack) {
  assert(NULL != stack);

  {
    Node* node;
    while (NULL != stack->head) {
      node = stack->head;
      stack->head = stack->head->next;
      free(node);
    }
  }
}

static Stack* stack_init(void) {
  Stack* stack = (Stack*)malloc(sizeof(Stack));

  if (NULL != stack) 
    stack->head = NULL;

  return stack;
}

static void stack_destroy(Stack** stack) {
  if (NULL != *stack) {
    stack_clear(*stack);
    free(*stack);
    *stack = NULL;
  }
}

static int stack_empty(Stack* stack) {
  assert(NULL != stack);

  return (NULL == stack->head);
}

static void stack_push(Stack* stack, ElementType value) {
  assert(NULL != stack);

  {
    Node* node = node_create(value);
    if (NULL != node) {
      node->next = stack->head;
      stack->head = node;
    }
  }
}

static void stack_pop(Stack* stack) {
  assert(NULL != stack);

  {
    Node* node = stack->head;
    if (NULL != node) {
      stack->head = node->next;
      free(node);
    }
  }
}

static ElementType stack_top(Stack* stack) {
  assert(NULL != stack && NULL != stack->head);

  return stack->head->value;
}

void linked_stack(void) {
  Stack* stack = stack_init();
  int i;
  
  fprintf(stdout, "\n**************LinkedStack**************\n");

  for (i = 0; i < 10; ++i)
    stack_push(stack, i * i);

  while (!stack_empty(stack)) {
    fprintf(stdout, 
        "LinkedStack element value is : %d\n", stack_top(stack));
    stack_pop(stack);
  }

  stack_destroy(&stack);
}
