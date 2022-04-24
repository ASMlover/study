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
#include <iostream>
#include <memory>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <unordered_map>
#include "../common/common.hh"
#include "../common/python_helper.hh"

////////////////////////////////////////////////////////////////////////////////////////////////////
// [C++ Profiler] Helper
inline std::uint64_t time_ns() noexcept {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
}

class CachedSourceLocation final : public common::Singleton<CachedSourceLocation> {
  static constexpr std::string kEmptyLocation = "";
  std::unordered_map<std::uint32_t, std::string> cached_locations_;

  inline std::uint32_t calc_hash(const char* s, std::size_t n) noexcept {
    std::uint32_t hashcode = 2166136261u;
    for (std::size_t i = 0; i < n; ++i) {
      hashcode ^= s[i];
      hashcode *= 16777619;
    }
    return hashcode;
  }
public:
  std::uint32_t create_source_location(
      const std::string& funcname, const std::string& filename, int lineno) noexcept {
    std::stringstream ss;
    ss << funcname << " (" << filename << ":" << lineno << ")";
    std::string source_location = ss.str();

    std::uint32_t hashcode = calc_hash(source_location.c_str(), source_location.size());
    if (auto it = cached_locations_.find(hashcode); it == cached_locations_.end())
      cached_locations_.insert({hashcode, source_location});

    return hashcode;
  }

  const std::string& get_location(std::uint32_t hashcode) const noexcept {
    if (auto it = cached_locations_.find(hashcode); it != cached_locations_.end())
      return it->second;
    return kEmptyLocation;
  }
};

struct FunctionMark {
  std::uint32_t hashcode;
  std::uint64_t timestamp;

  FunctionMark(std::uint32_t h, std::uint64_t t) noexcept : hashcode{h}, timestamp{t} {}
};

class FunctionStatistics final : private common::UnCopyable {
  std::uint32_t hashcode_;
  std::vector<std::uint64_t> executed_times_;
public:
  FunctionStatistics(std::uint32_t hashcode) noexcept : hashcode_{hashcode} {}

  inline const std::uint32_t get_hashcode() const noexcept { return hashcode_; }
  inline const std::vector<std::uint64_t>& get_executed_times() const noexcept { return executed_times_; }
  inline void append_executed_ns(std::uint64_t executed_ns) noexcept { executed_times_.push_back(executed_ns); }
  inline std::size_t get_total_count() const noexcept { return executed_times_.size(); }

  inline std::uint64_t get_total_ns() const noexcept {
    return std::accumulate(executed_times_.begin(), executed_times_.end(), 0);
  }

  inline std::uint64_t get_max_ns() const noexcept {
    return *std::max_element(executed_times_.begin(), executed_times_.end());
  }
};
using FunctionStatisticsPtr = std::shared_ptr<FunctionStatistics>;

struct SortedMark {
  std::uint32_t hashcode;
  std::uint64_t total_ns;
  std::size_t total_count;
  std::uint64_t max_ns;

  SortedMark(std::uint32_t h, std::uint64_t total, std::size_t count, std::uint64_t max) noexcept
    : hashcode{h}, total_ns{total}, total_count{count}, max_ns{max} {
  }
};
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// [C++ Profiler] Implementation
class Profiler final : public common::Singleton<Profiler> {
  std::unordered_map<std::uint32_t, FunctionStatisticsPtr> pprofile_stats_;
  std::list<FunctionMark> pprofile_deque_;
public:
  inline void reset_event() noexcept {
    pprofile_stats_.clear();
    pprofile_deque_.clear();
  }

  void push_event(const std::string& funcname, const std::string& filename, int lineno) noexcept {
    std::uint32_t hashcode =
      CachedSourceLocation::get_instance().create_source_location(funcname, filename, lineno);
    pprofile_deque_.push_back({hashcode, time_ns()});
  }

  void pop_event() noexcept {
    if (pprofile_deque_.empty())
      return;

    FunctionMark fn_mark = pprofile_deque_.back();
    pprofile_deque_.pop_back();

    std::uint32_t hashcode = hashcode = fn_mark.hashcode;
    std::uint64_t executed_ns = time_ns() - fn_mark.timestamp;

    if (auto it = pprofile_stats_.find(hashcode); it != pprofile_stats_.end()) {
      it->second->append_executed_ns(executed_ns);
    }
    else {
      auto statistics = std::make_shared<FunctionStatistics>(hashcode);
      pprofile_stats_.insert({hashcode, statistics});
      statistics->append_executed_ns(executed_ns);
    }
  }

  void print_stats() noexcept {
    std::vector<SortedMark> sorted_stats;
    sorted_stats.reserve(pprofile_stats_.size());
    for (auto it : pprofile_stats_) {
      sorted_stats.push_back({
          it.second->get_hashcode(),
          it.second->get_total_ns(),
          it.second->get_total_count(),
          it.second->get_max_ns()});
    }
    std::sort(sorted_stats.begin(), sorted_stats.end(), [](const SortedMark& a, const SortedMark& b) {
          return a.max_ns > b.max_ns;
        });

    for (auto x : sorted_stats) {
      std::cout
        << CachedSourceLocation::get_instance().get_location(x.hashcode) << " | "
        << "TOTAL(ns):" << x.total_ns << " | "
        << "COUNT:" << x.total_count << " | "
        << "MAX(ns):" << x.max_ns
        << std::endl;
    }
  }
};
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// [C++ Profiler] Python Binding
static int _pprofile_tracefunc(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg) {
  switch (what) {
  case PyTrace_CALL:
    {
      PyCodeObject* funccode = frame->f_code;
      auto [funcname, filename] = python_helper::get_from_call(funccode);
      Profiler::get_instance().push_event(funcname, filename, funccode->co_firstlineno);
    } break;
  case PyTrace_C_CALL:
    if (PyCFunction_Check(arg)) {
      PyCFunctionObject* fn = reinterpret_cast<PyCFunctionObject*>(arg);
      auto [funcname, filename] = python_helper::get_from_ccall(fn);
      Profiler::get_instance().push_event(funcname, filename, 0);
    } break;
  case PyTrace_RETURN:
  case PyTrace_C_RETURN:
    Profiler::get_instance().pop_event();
    break;
  case PyTrace_C_EXCEPTION:
    if (PyCFunction_Check(arg))
      Profiler::get_instance().pop_event();
    break;
  default: break;
  }
  return 0;
}

static PyObject* _pprofile_enable(PyObject* obj, PyObject* args) {
  (void)obj, (void)args;

  Profiler::get_instance().reset_event();
  PyEval_SetProfile(_pprofile_tracefunc, nullptr);
  Py_RETURN_NONE;
}

static PyObject* _pprofile_disable(PyObject* obj, PyObject* args) {
  (void)obj, (void)args;

  PyEval_SetProfile(nullptr, nullptr);
  Py_RETURN_NONE;
}

static PyObject* _pprofile_print_stats(PyObject* obj, PyObject* args) {
  (void)obj, (void)args;

  PyEval_SetProfile(nullptr, nullptr);
  Profiler::get_instance().print_stats();
  Py_RETURN_NONE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

PyMODINIT_FUNC PyInit_cpprofile() {
  static PyMethodDef _pprofile_methods[] = {
    {"enable", _pprofile_enable, METH_NOARGS, "cpprofile.enable() -> None"},
    {"disable", _pprofile_disable, METH_NOARGS, "cpprofile.disable() -> None"},
    {"print_stats", _pprofile_print_stats, METH_NOARGS, "cpprofile.print_stats() -> None"},
    {nullptr},
  };
  static PyModuleDef _pprofile_module = {
    PyModuleDef_HEAD_INIT,
    "cpprofile", // m_name
    "CXX version of profile", // m_doc
    -1, // m_size
    _pprofile_methods, // m_methods
    nullptr, // m_reload
    nullptr, // m_traverse
    nullptr, // m_clear
    nullptr, // m_free
  };

  return PyModule_Create(&_pprofile_module);
}
