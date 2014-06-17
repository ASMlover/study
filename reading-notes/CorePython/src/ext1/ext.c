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
#include <Python.h>


static int fac(int n) {
  if (n < 2)
    return 1;
  return n * fac(n - 1);
}

static char* reverse(char* s) {
  register char t;
  register char* p = s;
  register char* q = (s + (strlen(s) - 1));

  while (p < q) {
    t = *p;
    *p++ = *q;
    *q-- = t;
  }

  return s;
}


static PyObject* Ext_Fac(PyObject* self, PyObject* args) {
  int num;
  if (0 == PyArg_ParseTuple(args, "i", &num))
    return NULL;

  return (PyObject*)Py_BuildValue("i", fac(num));
}

static PyObject* Ext_Reverse(PyObject* self, PyObject* args) {
  char* s;
  PyObject* r;
  if (0 == PyArg_ParseTuple(args, "s", &s))
    return NULL;

  r = (PyObject*)Py_BuildValue("ss", s = reverse(strdup(s)));
  free(s);

  return r;
}

// METH_VARARGS 表示参数以元组形式传入
static PyMethodDef ExtMethods[] = {
  {"Fac", Ext_Fac, METH_VARARGS}, 
  {"Reverse", Ext_Reverse, METH_VARARGS}, 
  {NULL, NULL},
};


void initExt(void) {
  Py_InitModule("Ext", ExtMethods);
}
