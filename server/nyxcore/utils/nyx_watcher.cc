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
#include <typeinfo>
#include <boost/algorithm/string/predicate.hpp>
#include "nyx_watcher.h"

namespace nyx {

void Watcher::record_alloc(const WatcherObject* w) {
  std::unique_lock<std::mutex> g(lock_);
  objects_.emplace(w, _ObjectData(w, std::string()));
}

std::vector<_ObjectData> Watcher::find_by_typename(const char* name) {
  std::vector<_ObjectData> r;
  std::unique_lock<std::mutex> g(lock_);
  for (auto& o : objects_) {
    auto* type_name = typeid(*o.second._ins).name();
    if (boost::ends_with(type_name, name))
      r.emplace_back(o.second);
  }
  return r;
}

std::vector<_ObjectData> Watcher::find_by_typeinfo(const std::type_info& ti) {
  std::vector<_ObjectData> r;
  std::unique_lock<std::mutex> g(lock_);
  for (auto& o : objects_) {
    if (ti == typeid(*o.second._ins))
      r.emplace_back(o.second);
  }
  return r;
}

std::uint32_t Watcher::count_by_typename(const char* name) {
  std::uint32_t count{};
  std::unique_lock<std::mutex> g(lock_);
  for (auto& o : objects_) {
    auto* type_name = typeid(*o.second._ins).name();
    if (boost::ends_with(type_name, name))
      ++count;
  }
  return count;
}

std::uint32_t Watcher::count_by_typeinfo(const std::type_info& ti) {
  std::uint32_t count{};
  std::unique_lock<std::mutex> g(lock_);
  for (auto& o : objects_) {
    if (ti == typeid(*o.second._ins))
      ++count;
  }
  return count;
}

#if defined(_NYXCORE_ENABLE_WATCHER)
WatcherObject::WatcherObject(void) {
  Watcher::instance().record_alloc(this);
}

WatcherObject::~WatcherObject(void) {
  Watcher::instance().record_dealloc(this);
}
#endif

static PyObject* _watcher_findbytype(PyObject* self, PyObject* args) {
  const char* name;
  if (!PyArg_ParseTuple(args, "s", &name)) {
    return nullptr;
  }

  auto objects = Watcher::instance().find_by_typename(name);
  auto* r = PyTuple_New(objects.size());
  for (auto i = 0u; i < objects.size(); ++i) {
    const auto& o = objects[i];
    PyTuple_SET_ITEM(r, i, Py_BuildValue("ls", o._ins, o._bt.c_str()));
  }
  return r;
}

static PyObject* _watcher_gettypecounts(PyObject* self) {
  Watcher::instance().lock();

  const auto& all_objects = Watcher::instance().get_all_objects();
  struct _TypeData {
    const char* name;
    std::uint32_t count{1};

    _TypeData(const char* n)
      : name(n) {
    }
  };
  std::unordered_map<std::size_t, _TypeData> type_datas;
  for (auto& o : all_objects) {
    auto& ti = typeid(*o.second._ins);
    auto hash_code = ti.hash_code();
    auto it = type_datas.find(hash_code);
    if (it != type_datas.end())
      ++it->second.count;
    else
      type_datas.emplace(hash_code, _TypeData(ti.name()));
  }

  PyObject* r = PyTuple_New(type_datas.size());
  std::size_t i{};
  for (auto& t : type_datas) {
    PyTuple_SET_ITEM(r, i, Py_BuildValue("si", t.second.name, t.second.count));
    ++i;
  }
  Watcher::instance().unlock();
  return r;
}

PyDoc_STRVAR(__findbytype_doc,
"W.findbytype(name) -> tuple -- return all instances of name ");
PyDoc_STRVAR(__gettypecounts_doc,
"W.gettypecounts() -> tuple -- return instance count of all type");

void nyx_watcher_wrap(PyObject* m) {
  static PyMethodDef _watcher_methods[] = {
    {"findbytype", (PyCFunction)_watcher_findbytype, METH_VARARGS, __findbytype_doc},
    {"gettypecounts", (PyCFunction)_watcher_gettypecounts, METH_NOARGS, __gettypecounts_doc},
    {nullptr},
  };

  auto* mod = Py_InitModule3("_nyxcore.watcher",
      _watcher_methods, "Module _nyxcore.watcher");
  Py_INCREF(mod);
  PyModule_AddObject(m, "watcher", mod);
}

}
