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
  "../stl/vector"
)




func TestVector(t *testing.T) {
  v := vector.New()
  if v == nil || !v.Empty() || v.Size() != 0 {
    t.Error("error in New function of vector module")
    return
  }
  if v.Front() != nil || v.Back() != nil {
    t.Error("error in New function of vector module")
    return
  }

  for i := 0; i < 10; i++ {
    v.PushBack(i * i)
  }
  if v.Empty() || v.Size() != 10 || v.Front() != 0 || v.Back() != 81 {
    t.Error("error in PushBack function of vector module")
  }

  for i := 0; i < 5; i++ {
    v.PushFront((i + 1) * 100)
  }
  if v.Size() != 15 || v.Front() != 500 || v.Back() != 81 {
    t.Error("error in PushFront function of vector module")
  }

  v.PopFront()
  if v.Front() != 400 || v.Back() != 81 || v.Size() != 14 {
    t.Error("error in PopFront function of vector module")
  }
  v.PopBack()
  if v.Front() != 400 || v.Back() != 64 || v.Size() != 13 {
    t.Error("error in PopBack function of vector module")
  }

  v.Set(-1, 3000)
  v.Set(15, 3000)
  v.Set(5, 3000)
  if v.Get(-1) != nil || v.Get(15) != nil || v.Get(5) != 3000 {
    t.Error("error in Set/Get function of vector module")
  }

  v.Clear()
  if !v.Empty() || v.Size() != 0 {
    t.Error("error in Clear function of vector module")
  }
}
