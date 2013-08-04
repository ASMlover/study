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

package test

import (
  "testing"
  "../stl/list"
)



func TestList(t *testing.T) {
  l := list.New()
  if l == nil  || !l.Empty() || l.Size() != 0 {
    t.Error("error in New function of list module")
    return
  }
  if l.Front() != nil || l.Back() != nil {
    t.Error("error in New function of list module")
    return
  }

  for i := 0; i < 10; i++ {
    l.PushBack(i * i)
  }
  if l.Empty() || l.Size() != 10 || l.Front() != 0 || l.Back() != 81 {
    t.Error("error in PushBack function of list module")
  }

  for i := 0; i < 5; i++ {
    l.PushFront((i + 1) * 100)
  }
  if l.Size() != 15 || l.Front() != 500 || l.Back() != 81 {
    t.Error("error in PushFront function of list module")
  }

  l.PopFront()
  l.PopBack()
  if l.Size() != 13 || l.Front() != 400 || l.Back() != 64 {
    t.Error("error in PopFront/PopBack function of list module")
  }

  l.Clear()
  if !l.Empty() || l.Size() != 0 || l.Front() != nil || l.Back() != nil {
    t.Error("error in Clear function of list module")
  }
}
