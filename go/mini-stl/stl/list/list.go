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

package list





type node_t struct {
  prev  *node_t
  next  *node_t
  value interface{}
}

type list_t struct {
  head  node_t
  size  uint
}






func (l *list_t) begin() *node_t {
  return l.head.next
}

func (l *list_t) end() *node_t {
  return &l.head
}

func (l *list_t) insert(pos *node_t, value interface{}) {
  n := node_t{nil, nil, value}
  n.prev = pos.prev
  n.next = pos
  pos.prev.next = &n
  pos.prev = &n
  l.size++
}

func (l *list_t) erase(pos *node_t) {
  prev := pos.prev
  next := pos.next
  prev.next = next
  next.prev = prev
  l.size--
}


func New() *list_t {
  l := new(list_t)
  l.head.prev = &l.head
  l.head.next = &l.head
  l.size      = 0

  return l
}

func (l *list_t) Clear() {
  l.head.prev = &l.head
  l.head.next = &l.head
  l.size      = 0
}

func (l *list_t) Empty() bool {
  return l.begin() == l.end()
}

func (l *list_t) Size() uint {
  return l.size
}

func (l *list_t) PushBack(value interface{}) {
  l.insert(l.end(), value)
}

func (l *list_t) PushFront(value interface{}) {
  l.insert(l.begin(), value)
}

func (l *list_t) PopBack() {
  if l.begin() == l.end() {
    return
  }

  n := l.end()
  l.erase(n.prev)
}

func (l *list_t) PopFront() {
  if l.begin() == l.end() {
    return
  }

  l.erase(l.begin())
}

func (l *list_t) Front() interface{} {
  if l.begin() == l.end() {
    return nil
  }

  return l.begin().value
}

func (l *list_t) Back() interface{} {
  if l.begin() == l.end() {
    return nil
  }

  n := l.end()
  return n.prev.value
}
