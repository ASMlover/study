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
package slist



type node_t struct {
  next  *node_t
  value interface{}
}

type slist_t struct {
  head  *node_t
  tail  *node_t
  size  int
}




func New() *slist_t {
  return &slist_t{nil, nil, 0}
}

func (l *slist_t) Clear() {
  for l.head != nil {
    l.head = l.head.next
  }
  l.head = nil
  l.tail = nil
  l.size = 0
}

func (l *slist_t) Empty() bool {
  return l.head == nil
}

func (l *slist_t) Size() int {
  return l.size
}

func (l *slist_t) PushBack(value interface{}) {
  n := node_t{nil, value}

  if l.head == nil {
    l.head = &n
    l.tail = &n
  } else {
    l.tail.next = &n
    l.tail = &n
  }
  l.size++
}

func (l *slist_t) PushFront(value interface{}) {
  n := node_t{l.head, value}

  if l.head == nil {
    l.tail = &n
  }
  l.head = &n
  l.size++
}

func (l *slist_t) PopFront() interface{} {
  if l.head == nil {
    return nil
  }

  value := l.head.value
  l.head = l.head.next
  l.size--

  return value
}

func (l *slist_t) Front() interface{} {
  if l.head == nil {
    return nil
  }

  return l.head.value
}

func (l *slist_t) Back() interface{} {
  if l.head == nil {
    return nil
  }

  return l.tail.value
}
