// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <fstream>
#include <iostream>
#include <sstream>
#include "compiler.hh"
#include "vm.hh"

static std::string _s_root_dir;

inline std::string read_file(const std::string& path) {
  std::ifstream fp(path);
  if (fp.is_open()) {
    std::stringstream ss;
    ss << fp.rdbuf();

    return ss.str();
  }
  return "";
}

inline std::string file_path(const std::string& name) {
  return _s_root_dir + name + ".wren";
}

static std::string read_module(wrencc::WrenVM& vm, const std::string& module) {
  // first try to load the module with a `.wren` extension
  std::string module_path = file_path(module);
  std::string module_contents = read_file(module_path);

  // if no contents could be loaded treat the module name as specifying a
  // package and try to load the `__init__.wren` file in the package

  if (module_contents.empty()) {
    std::string package_module = module + "/__init__";
    std::string package_module_path = file_path(package_module);
    module_contents = read_file(package_module_path);
  }
  return module_contents;
}

static void report_error(wrencc::WrenVM& vm, wrencc::WrenError type,
    const std::string& module, int line, const std::string& message) {
  switch (type) {
  case wrencc::WrenError::COMPILE:
    std::cerr
      << "[`" << module << "` LINE: " << line << "] " << message
      << std::endl;
    break;
  case wrencc::WrenError::RUNTIME:
    std::cerr << message << std::endl;
    break;
  case wrencc::WrenError::SATCK_TRACE:
    std::cerr
      << "[`" << module << "` LINE: " << line << "] in " << message
      << std::endl;
    break;
  }
}

static std::string resolve_module(wrencc::WrenVM& vm,
    const std::string& importer, const std::string& name) {
  if (name.size() > 2 &&
      ((name[0] == '.' && name[1] == '/') ||
       (name[0] == '.' && name[1] == '.' && name[2] == '/'))) {
    std::string resolved = "";
    auto pos = importer.find_last_of('/');
    if (pos != std::string::npos)
      resolved = importer.substr(0, pos);

    if (resolved.size() > 0 && resolved.back() != '/')
      resolved.push_back('/');
    resolved += name;

    std::size_t off = 0;
    for (;;) {
      pos = resolved.find_first_of('/', off);
      if (pos != std::string::npos) {
        std::string temp = resolved.substr(off, pos);
        if (temp.size() == 1 && temp == ".")
          resolved = resolved.substr(pos + 1);
        else
          off += pos + 1;
      }
      else {
        break;
      }
    }

    return resolved;
  }
  return name;
}

static void eval_with_file(const std::string& fname) {
  auto pos = fname.find_last_of('\\');
  if (pos != std::string::npos)
    _s_root_dir = fname.substr(0, pos + 1);

  std::ifstream fp(fname);
  if (fp.is_open()) {
    std::stringstream ss;
    ss << fp.rdbuf();

    wrencc::WrenVM vm;
    vm.set_load_fn(read_module);
    vm.set_error_fn(report_error);
    vm.set_res_module_fn(resolve_module);
    vm.interpret("main", ss.str());
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  std::cout << "W R E N - C C" << std::endl;

  eval_with_file(argv[1]);

  return 0;
}
