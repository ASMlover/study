// Copyright (c) 2023 ASMlover. All rights reserved.
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
#pragma once

#include <string>
#include <tuple>
#include <Python.h>

namespace pprof::pyutils {

inline std::tuple<std::string, std::string> get_from_call(PyCodeObject* funccode) noexcept {
	std::string funcname;
	if (PyObject* py_funcname = PyUnicode_AsASCIIString(funccode->co_name); py_funcname != nullptr) {
		funcname = PyBytes_AS_STRING(py_funcname);
		Py_DECREF(py_funcname);
	}

	std::string filename;
	if (PyObject* py_filename = PyUnicode_AsASCIIString(funccode->co_filename); py_filename != nullptr) {
		filename = PyBytes_AS_STRING(py_filename);
		Py_DECREF(py_filename);
	}

	return std::make_tuple(funcname, filename);
}

inline std::tuple<std::string, std::string> get_from_ccall(PyCFunctionObject* fn) noexcept {
	const char* modulename{};
	if (fn->m_self == nullptr) {
		PyObject* fn_module = fn->m_module;
		if (fn_module != nullptr && PyUnicode_Check(fn_module)) {
			Py_XINCREF(fn_module);
			if (PyObject* py_modulename = PyUnicode_AsASCIIString(fn_module); py_modulename != nullptr) {
				modulename = PyBytes_AS_STRING(py_modulename);
				Py_DECREF(py_modulename);
			}
			Py_DECREF(fn_module);
		}
		else if (fn_module != nullptr && PyModule_Check(fn_module)) {
			modulename = PyModule_GetName(fn_module);
			if (modulename == nullptr) {
				PyErr_Clear();
				modulename = "__builtin__";
			}
		}
		else {
			modulename = "__builtin__";
		}
	}
	else {
		modulename = "<built-in method>";
		if (PyObject* fn_name = PyUnicode_FromString(fn->m_ml->ml_name); fn_name != nullptr) {
			PyObject* fn_type = _PyType_Lookup(Py_TYPE(fn->m_self), fn_name);
			Py_XINCREF(fn_type);
			Py_DECREF(fn_name);

			if (fn_type != nullptr) {
				PyObject* fn_repr = PyObject_Repr(fn_type);
				Py_DECREF(fn_type);
				if (fn_repr != nullptr) {
					if (PyObject* py_repr = PyUnicode_AsASCIIString(fn_type); py_repr != nullptr) {
						modulename = PyBytes_AS_STRING(py_repr);
						Py_DECREF(py_repr);
					}
					Py_DECREF(fn_repr);
				}
			}
		}
		PyErr_Clear();
	}

	return std::make_tuple(fn->m_ml->ml_name, modulename);
}

}
