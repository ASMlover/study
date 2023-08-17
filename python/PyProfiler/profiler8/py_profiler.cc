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
#include <iostream>
#include <memory>
#include <string>
#include <list>
#include "./tracy/tracy/Tracy.hpp"
#include "./tracy/client/TracyProfiler.hpp"
#include "./tracy/client/TracyScoped.hpp"
#include "../common/common.hh"
#include "../common/python_utils.hh"

namespace pprof {

class TracyProfiler final : public common::Singleton<TracyProfiler> {
	using ScopedZonePtr = std::shared_ptr<tracy::ScopedZone>;

	bool is_started_{};
	std::list<ScopedZonePtr> scoped_zones_;
public:
	inline void startup(std::uint32_t listen_port = 8099) noexcept {
		if (!is_started_) {
			tracy::StartupProfiler(listen_port);
			is_started_ = true;
		}
	}

	inline void push_event(const std::string& funcname, const std::string& filename, int lineno) noexcept {
		scoped_zones_.push_back(std::make_shared<tracy::ScopedZone>(
					lineno, filename.c_str(), filename.size(), funcname.c_str(), funcname.size(),
					nullptr, 0, TRACY_CALLSTACK, true));
	}

	inline void pop_event() noexcept {
		if (!scoped_zones_.empty())
			scoped_zones_.pop_back();
	}

	inline void frame_event() noexcept { FrameMarkNamed("Tracy.Python"); }
};

static int _profiler_tracefunc(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg) {
	switch (what) {
	case PyTrace_CALL:
		{
			PyCodeObject* funccode = PyFrame_GetCode(frame);
			auto [funcname, filename] = pyutils::get_from_call(funccode);
			TracyProfiler::get_instance().push_event(funcname, filename, funccode->co_firstlineno);
		} break;
	case PyTrace_RETURN:
		TracyProfiler::get_instance().pop_event();
		break;
	case PyTrace_C_CALL:
		if (PyCFunction_Check(arg)) {
			PyCFunctionObject* fn = reinterpret_cast<PyCFunctionObject*>(arg);
			auto [funcname, filename] = pyutils::get_from_ccall(fn);
			TracyProfiler::get_instance().push_event(funcname, filename, 0);
		} break;
	case PyTrace_C_RETURN:
	case PyTrace_C_EXCEPTION:
		if (PyCFunction_Check(arg))
			TracyProfiler::get_instance().pop_event();
		break;
	default: break;
	}
	return 0;
}

static PyObject* _profiler_startup(PyObject* obj, PyObject* args, PyObject* kwds) {
	(void)obj;

	static char* kwdslist[] = {(char*)"port", nullptr};

	int listen_port = 8099;
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:startup", kwdslist, &listen_port))
		return nullptr;

	TracyProfiler::get_instance().startup(static_cast<std::uint32_t>(listen_port));
	Py_RETURN_NONE;
}

static PyObject* _profiler_start(PyObject* obj, PyObject* args) {
	(void)obj, (void)args;

	PyEval_SetProfile(_profiler_tracefunc, nullptr);
	Py_RETURN_NONE;
}

static PyObject* _profiler_stop(PyObject* obj, PyObject* args) {
	(void)obj, (void)args;

	PyEval_SetProfile(nullptr, nullptr);
	Py_RETURN_NONE;
}

static PyObject* _profiler_frame(PyObject* obj, PyObject* args) {
	(void)obj, (void)args;

	TracyProfiler::get_instance().frame_event();
	Py_RETURN_NONE;
}

}

PyMODINIT_FUNC PyInit_cpprof() {
	static PyMethodDef _profiler_methods[] = {
		{"startup", (PyCFunction)pprof::_profiler_startup, METH_VARARGS | METH_KEYWORDS, "cpprof.startup(port: int = 8099) -> None"},
		{"start", pprof::_profiler_start, METH_NOARGS, "cpprof.start() -> None"},
		{"stop", pprof::_profiler_stop, METH_NOARGS, "cpprof.stop() -> None"},
		{"frame", pprof::_profiler_frame, METH_NOARGS, "cpprof.frame() -> None"},
		{nullptr},
	};
	static PyModuleDef _profiler_module = {
		PyModuleDef_HEAD_INIT,
		"cpprof", // m_name
		"CXX version of PyProfiler with Tracy binding", // m_doc
		-1, // m_size
		_profiler_methods, // m_methods
		nullptr, // m_reload
		nullptr, // m_traverse
		nullptr, // m_clear
		nullptr, // m_free
	};

	return PyModule_Create(&_profiler_module);
}
