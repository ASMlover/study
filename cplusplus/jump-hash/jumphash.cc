// Copyright (c) 2018 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <cstdint>
#include <Python.h>

static std::int32_t _jump_consistent_hash(
    std::uint64_t key, std::int32_t num_buckets) {
  std::int64_t b = -1, j = 0;

  while (j < num_buckets) {
    b = j;
    key = key * 2862933555777941757ULL + 1;
    j = static_cast<std::int64_t>(
        (b + 1) * (double(1LL << 31) / double((key >> 33) + 1)));
  }
  return b;
}

static PyObject* _jumphash_hash(PyObject* self, PyObject* args) {
  std::uint64_t key;
  std::int32_t num_buckets;
  if (!PyArg_ParseTuple(args, "Ki", &key, &num_buckets))
    return nullptr;
  if (num_buckets < 1) {
    PyErr_SetString(PyExc_ValueError,
        "`num_buckets` must be a positive number");
    return nullptr;
  }

  auto h = _jump_consistent_hash(key, num_buckets);
  return PyInt_FromLong(h);
}

PyDoc_STRVAR(_jumphash_doc,
"Fast, minimal memory, consistent hash algorithm");
PyDoc_STRVAR(_jumphash_hash_doc,
"hash(key, num_buckets) -> int\n\n"
"Generate a number in the range [0, num_buckets)\n"
"This function uses C bindings for speed\n\n"
"Args:\n"
"\tkey(int): the key to hash\n"
"\tnum_buckets(int): number of buckets to use\n"
"Returns:\n"
"\tthe bucket number `key` computes to\n"
"Raises:\n"
"\tValueError: if `num_buckets` is not a positive number");

static PyMethodDef _jumphash_methods[] = {
  {"hash", _jumphash_hash, METH_VARARGS, _jumphash_hash_doc},
  {nullptr}
};

PyMODINIT_FUNC init_jumphash(void) {
  Py_InitModule3("_jumphash", _jumphash_methods, _jumphash_doc);
}
