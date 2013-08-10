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
package main

import (
  "fmt"
  "reflect"
)

type Student struct {
  Id    int
  Name  string
  Age   int
}

func (s Student) Hello() {
  fmt.Println("Hello, world!")
}

func Info(o interface{}) {
  t := reflect.TypeOf(o)
  fmt.Println("o=>type: ", t.Name())

  if k := t.Kind(); k != reflect.Struct {
    fmt.Println("error")
    return
  }

  v := reflect.ValueOf(o)
  fmt.Println("o=>value: ")
  for i := 0; i < t.NumField(); i++ {
    f := t.Field(i)
    //val := v.Field(i)
    val := v.Field(i).Interface()
    fmt.Printf("%s\t: %v=>%v\n", f.Name, f.Type, val)
  }

  for i := 0; i < t.NumMethod(); i++ {
    m := t.Method(i)
    fmt.Printf("%s\t: %v", m.Name, m.Type)
  }
}


func main() {
  s := Student{1001, "Jack", 18}
  Info(s)
}
