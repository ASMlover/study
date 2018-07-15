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
#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include "pysmall_object.h"
#include "pysmall_intobject.h"
#include "pysmall_strobject.h"
#include "pysmall_dictobject.h"

static std::unique_ptr<pysmall::DictObject> g_ENV(new pysmall::DictObject());

static bool __isdigit(const std::string& s) {
  for (auto c : s) {
    if (!std::isdigit(c))
      return false;
  }
  return true;
}

static pysmall::Object* symbol_as_object(const std::string& symbol) {
  auto* key = new pysmall::StrObject(symbol.c_str());
  auto* val = g_ENV->getitem(key);
  if (val == nullptr)
    std::cerr << "[ERROR]: " << symbol << " is not defined!!!" << std::endl;
  return val;
}

static void pysmall_exec_print(const std::string& symbol) {
  auto* x = symbol_as_object(symbol);
  if (x != nullptr)
    x->type->print_fn(x);
}

static void pysmall_exec_add(const std::string& t, const std::string& s) {
  if (__isdigit(s)) {
    auto* val = new pysmall::IntObject(std::atoi(s.c_str()));
    auto* key = new pysmall::StrObject(t.c_str());
    g_ENV->setitem(key, val);
  }
  else if (s.find("\"") != std::string::npos) {
    auto* val = new pysmall::StrObject(s.substr(1, s.size() - 2).c_str());
    auto* key = new pysmall::StrObject(t.c_str());
    g_ENV->setitem(key, val);
  }
  else if (auto pos = s.find(" + "); pos != std::string::npos) {
    auto* lobj = symbol_as_object(s.substr(0, pos));
    auto* robj = symbol_as_object(s.substr(pos + 3));
    if (lobj != nullptr && robj != nullptr && lobj->type == robj->type) {
      auto* val = lobj->type->plus_fn(lobj, robj);
      auto* key = new pysmall::StrObject(t.c_str());
      g_ENV->setitem(key, val);
    }
    g_ENV->type->print_fn(g_ENV.get());
  }
}

static void pysmall_exec_command(const std::string& s) {
  std::size_t pos{};
  if ((pos = s.find("print ")) != std::string::npos) {
    pysmall_exec_print(s.substr(6));
  }
  else if ((pos = s.find(" = ")) != std::string::npos) {
    auto x = s.substr(0, pos);
    auto y = s.substr(pos + 3);
    pysmall_exec_add(x, y);
  }
}

static void pysmall_exec(void) {
  static const char* info = "********* pysmall research *********";
  static const char* prompt = ">>> ";

  std::string s;

  std::cout << info << std::endl;
  std::cout << prompt;
  while (std::getline(std::cin, s)) {
    if (s.empty()) {
      std::cout << prompt;
      continue;
    }
    else if (s == "exit") {
      break;
    }
    else {
      pysmall_exec_command(s);
    }
    std::cout << prompt;
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  pysmall_exec();

  return 0;
}
