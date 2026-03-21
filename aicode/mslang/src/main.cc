// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Compiler.hh"
#include "Lsp.hh"
#include "Scanner.hh"
#include "Serializer.hh"
#include "VM.hh"

static constexpr ms::sz_t kHISTORY_MAX = 1000;

static inline ms::str_t trim(const ms::str_t& s) noexcept {
  auto start = s.find_first_not_of(" \t\r\n");
  if (start == ms::str_t::npos) return "";
  auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

static bool needs_continuation(const ms::str_t& source) noexcept {
  ms::Scanner scanner(source);
  ms::TokenType last_type = ms::TokenType::TOKEN_EOF;
  int depth = 0;

  for (;;) {
    ms::Token token = scanner.scan_token();
    if (token.type == ms::TokenType::TOKEN_EOF) break;
    if (token.type == ms::TokenType::TOKEN_ERROR) return false;

    last_type = token.type;

    switch (token.type) {
    case ms::TokenType::TOKEN_LEFT_PAREN:
    case ms::TokenType::TOKEN_LEFT_BRACE:
    case ms::TokenType::TOKEN_LEFT_BRACKET:
      depth++; break;
    case ms::TokenType::TOKEN_RIGHT_PAREN:
    case ms::TokenType::TOKEN_RIGHT_BRACE:
    case ms::TokenType::TOKEN_RIGHT_BRACKET:
      depth--; break;
    default: break;
    }
  }

  if (last_type == ms::TokenType::TOKEN_EOF) return false;
  if (depth > 0) return true;
  return last_type != ms::TokenType::TOKEN_SEMICOLON;
}

static void print_help() noexcept {
  std::cout << "Maple REPL commands:\n"
            << "  @help          Show this help message\n"
            << "  @quit          Exit the REPL\n"
            << "  @load <file>   Load and execute a Maple script\n"
            << "  @history       Show command history\n"
            << "  @clear         Clear the screen\n"
            << "\n"
            << "Multi-line input is supported: unmatched brackets or\n"
            << "trailing operators/commas continue on the next line.\n";
}

static void load_file(const ms::str_t& arg) noexcept {
  ms::str_t path = trim(arg);
  // Strip surrounding quotes if present
  if (path.size() >= 2
      && ((path.front() == '"' && path.back() == '"')
          || (path.front() == '\'' && path.back() == '\''))) {
    path = path.substr(1, path.size() - 2);
  }

  if (path.empty()) {
    std::cerr << "Usage: @load <file.ms>" << std::endl;
    return;
  }

  auto source_opt = ms::ModuleLoader::read_source(path);
  if (!source_opt.has_value()) {
    std::cerr << "Could not open file \"" << path << "\"." << std::endl;
    return;
  }

  auto& vm = ms::VM::get_instance();
  vm.interpret(*source_opt, path);
}

static void print_history(const std::vector<ms::str_t>& history) noexcept {
  for (ms::sz_t i = 0; i < history.size(); ++i) {
    std::cout << "  " << (i + 1) << "  " << history[i] << "\n";
  }
}

static bool handle_meta_command(
    const ms::str_t& input, const std::vector<ms::str_t>& history) noexcept {
  ms::str_t cmd = trim(input);
  if (cmd == "@quit" || cmd == "@q") return true;

  if (cmd == "@help" || cmd == "@h") {
    print_help();
  } else if (cmd == "@history") {
    print_history(history);
  } else if (cmd == "@clear") {
    std::cout << "\033[2J\033[H" << std::flush;
  } else if (cmd.starts_with("@load ")) {
    load_file(cmd.substr(6));
  } else {
    std::cerr << "Unknown command: " << cmd << ". Type @help for help." << std::endl;
  }
  return false;
}

static void repl() noexcept {
  auto& vm = ms::VM::get_instance();
  std::vector<ms::str_t> history;

  std::cout << "Maple v1.0 -- type @help for commands, @quit to exit.\n";

  for (;;) {
    std::cout << "maple> ";

    ms::str_t line;
    if (!std::getline(std::cin, line)) {
      std::cout << std::endl;
      break;
    }

    if (trim(line).empty()) continue;

    // Meta-commands
    if (trim(line)[0] == '@') {
      if (handle_meta_command(line, history)) break;
      continue;
    }

    // Multi-line input: continue on unbalanced brackets or non-trigger line end
    ms::str_t source = line;
    while (needs_continuation(source)) {
      std::cout << "  ...> ";
      ms::str_t continuation;
      if (!std::getline(std::cin, continuation)) {
        std::cout << std::endl;
        break;
      }
      source += "\n" + continuation;
    }

    // Add to history
    if (history.size() >= kHISTORY_MAX) {
      history.erase(history.begin());
    }
    history.push_back(source);

    vm.interpret(source);
  }
}

static void run_file(const ms::str_t& path) noexcept {
  auto& vm = ms::VM::get_instance();

  if (ms::is_msc_file(path)) {
    // Execute precompiled bytecode
    ms::ObjFunction* fn = ms::deserialize(path);
    if (fn == nullptr) std::exit(65);

    ms::InterpretResult result = vm.interpret_bytecode(fn);
    if (result == ms::InterpretResult::INTERPRET_RUNTIME_ERROR) std::exit(70);
    return;
  }

  auto source_opt = ms::ModuleLoader::read_source(path);
  if (!source_opt.has_value()) {
    std::cerr << "Could not open file \"" << path << "\"." << std::endl;
    std::exit(74);
  }

  const auto& source = *source_opt;
  ms::str_t msc = ms::msc_path_for(path);

  // Try loading from cache (hash-validated)
  if (auto* cached = ms::try_load_cache(msc, source)) {
    ms::InterpretResult result = vm.interpret_bytecode(cached, source, path);
    if (result == ms::InterpretResult::INTERPRET_RUNTIME_ERROR) std::exit(70);
    return;
  }

  // Cache miss — compile, execute, write cache
  ms::ObjFunction* fn = ms::compile(source, path);
  if (fn == nullptr) std::exit(65);

  ms::serialize(fn, msc, source); // best-effort cache write

  ms::InterpretResult result = vm.interpret_bytecode(fn, source, path);
  if (result == ms::InterpretResult::INTERPRET_RUNTIME_ERROR) std::exit(70);
}

static void compile_file(const ms::str_t& path) noexcept {
  auto source_opt = ms::ModuleLoader::read_source(path);
  if (!source_opt.has_value()) {
    std::cerr << "Could not open file \"" << path << "\"." << std::endl;
    std::exit(74);
  }

  ms::ObjFunction* fn = ms::compile(*source_opt, path);
  if (fn == nullptr) {
    std::cerr << "Compilation failed." << std::endl;
    std::exit(65);
  }

  // Replace .ms extension with .msc (or append .msc)
  ms::str_t out_path = path;
  if (out_path.size() >= 3 && out_path.substr(out_path.size() - 3) == ".ms") {
    out_path += "c";
  } else {
    out_path += ".msc";
  }

  if (!ms::serialize(fn, out_path)) {
    std::cerr << "Failed to write bytecode to \"" << out_path << "\"." << std::endl;
    std::exit(74);
  }

  std::cout << "Compiled: " << path << " -> " << out_path << std::endl;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  if (argc == 1) {
    repl();
  } else if (argc == 2 && ms::str_t(argv[1]) == "--lsp") {
    ms::LspServer server;
    server.run();
  } else if (argc == 2) {
    run_file(argv[1]);
  } else if (argc == 3 && ms::str_t(argv[1]) == "--compile") {
    compile_file(argv[2]);
  } else {
    std::cerr << "Usage: mslang [script]\n"
              << "       mslang --lsp\n"
              << "       mslang --compile <script.ms>" << std::endl;
    std::exit(64);
  }

  return 0;
}
