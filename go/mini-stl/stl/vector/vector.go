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
package vector



type vector_t struct {
  elems []interface{}
  size  int
}




func New() *vector_t {
  v := new(vector_t)
  v.size = 0

  return v
}

func (v *vector_t) Clear() {
  v.elems = v.elems[0:0]
  v.size  = 0
}

func (v *vector_t) Empty() bool {
  return v.size == 0
}

func (v *vector_t) Size() int {
  return v.size
}

func (v *vector_t) PushBack(value interface{}) {
  v.elems = append(v.elems, value)
  v.size++
}

func (v *vector_t) PushFront(value interface{}) {
  v.elems = append(v.elems[0:1], v.elems[0:]...)
  v.elems[0] = value
  v.size++
}

func (v *vector_t) PopBack() {
  v.size--
  v.elems = v.elems[0:v.size]
}

func (v *vector_t) PopFront() {
  v.size--
  v.elems = v.elems[1:]
}

func (v *vector_t) Set(i int, value interface{}) {
  if i < 0 || i >= v.size {
    return
  }

  v.elems[i] = value
}

func (v *vector_t) Get(i int) interface{} {
  if i < 0 || i >= v.size {
    return nil
  }

  return v.elems[i]
}

func (v *vector_t) Front() interface{} {
  if v.size == 0 {
    return nil
  }

  return v.elems[0]
}

func (v *vector_t) Back() interface{} {
  if v.size == 0 {
    return nil
  }

  return v.elems[v.size - 1]
}
