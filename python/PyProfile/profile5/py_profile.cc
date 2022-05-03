// Copyright (c) 2022 ASMlover. All rights reserved.
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
#include <string>
#include "optick/optick.h"
#include "../common/common.hh"
#include "../common/python_helper.hh"

class OptickProfiler final : public common::Singleton<OptickProfiler> {
public:
  inline void startup(short listen_port = 8099) noexcept {
    Optick::SetDefaultPort(listen_port);
  }

  inline void push_event(const std::string& funcname, const std::string& filename, int lineno) noexcept {
    Optick::EventDescription* desc = Optick::CreateDescription(funcname.c_str(), filename.c_str(), lineno);
    if (desc != nullptr)
      desc->color = static_cast<std::uint32_t>(Optick::Color::GoldenRod);
    Optick::Event::Push(*desc);
  }

  inline void pop_event() noexcept { Optick::Event::Pop(); }
  inline void frame_event() noexcept { OPTICK_FRAME("Optick.Python"); }
};

static int _pprofile_tracefunc(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg) {
  switch (what) {
  case PyTrace_CALL:
    {
      PyCodeObject* funccode = frame->f_code;
      auto [funcname, filename] = python_helper::get_from_call(funccode);
      OptickProfiler::get_instance().push_event(funcname, filename, funccode->co_firstlineno);
    } break;
  case PyTrace_C_CALL:
    if (PyCFunction_Check(arg)) {
      PyCFunctionObject* fn = reinterpret_cast<PyCFunctionObject*>(arg);
      auto [funcname, filename] = python_helper::get_from_ccall(fn);
      OptickProfiler::get_instance().push_event(funcname, filename, 0);
    } break;
  case PyTrace_RETURN:
  case PyTrace_C_RETURN:
    OptickProfiler::get_instance().pop_event();
    break;
  case PyTrace_C_EXCEPTION:
    if (PyCFunction_Check(arg))
      OptickProfiler::get_instance().pop_event();
    break;
  default: break;
  }
  return 0;
}

static PyObject* _pprofile_startup(PyObject* obj, PyObject* args, PyObject* kwds) {
  (void)obj;

  static char* kwdslist[] = {(char*)"port", nullptr};

  int listen_port = 8099;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:startup", kwdslist, &listen_port))
    return nullptr;

  OptickProfiler::get_instance().startup(static_cast<short>(listen_port));
  Py_RETURN_NONE;
}

static PyObject* _pprofile_enable(PyObject* obj,  PyObject* args) {
  (void)obj, (void)args;

  PyEval_SetProfile(_pprofile_tracefunc, nullptr);
  Py_RETURN_NONE;
}

static PyObject* _pprofile_disable(PyObject* obj, PyObject* args) {
  (void)obj, (void)args;

  PyEval_SetProfile(nullptr, nullptr);
  Py_RETURN_NONE;
}

static PyObject* _pprofile_frame(PyObject* obj, PyObject* args) {
  (void)obj, (void)args;

  OptickProfiler::get_instance().frame_event();
  Py_RETURN_NONE;
}

PyMODINIT_FUNC PyInit_cpprofile() {
  static PyMethodDef _pprofile_methods[] = {
    {"startup", (PyCFunction)_pprofile_startup, METH_VARARGS | METH_KEYWORDS, "cpprofile.startup(port: int = 8099) -> None"},
    {"enable", _pprofile_enable, METH_NOARGS, "cpprofile.enable() -> None"},
    {"disable", _pprofile_disable, METH_NOARGS, "cpprofile.disable() -> None"},
    {"frame", _pprofile_frame, METH_NOARGS, "cpprofile.frame() -> None"},
    {nullptr},
  };
  static PyModuleDef _pprofile_module = {
    PyModuleDef_HEAD_INIT,
    "cpprofile", // m_name
    "CXX profile with Optick binding", // m_doc
    -1, // m_size
    _pprofile_methods, // m_methods
    nullptr, // m_reload
    nullptr, // m_traverse
    nullptr, // m_clear
    nullptr, // m_free
  };

  return PyModule_Create(&_pprofile_module);
}
