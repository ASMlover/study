// Copyright (c) 2024 ASMlover. All rights reserved.
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
#include "common.hh"

enum class MetaCommandResult {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND,
};

class SQLCompiler final : private sdb::UnCopyable {
public:
  enum class PrepareRet : int {
    SUCCESS,
    UNRECONGNIZED_STATEMENT,
  };

  enum class StatementType : int {
    INSERT,
    SELECT,
  };
private:
  StatementType type_;
public:
  PrepareRet prepare(const sdb::str_t& command) noexcept {
    if (command.starts_with("insert")) {
      type_ = StatementType::INSERT;
      return PrepareRet::SUCCESS;
    }
    else if (command.starts_with("select")) {
      type_ = StatementType::SELECT;
      return PrepareRet::SUCCESS;
    }

    return PrepareRet::UNRECONGNIZED_STATEMENT;
  }

  void execute() noexcept {
    switch (type_) {
    case StatementType::INSERT:
      std::cout << "This is where we would do an insert" << std::endl;
      break;
    case StatementType::SELECT:
      std::cout << "This is where we would do a select" << std::endl;
      break;
    default: break;
    }
  }
};

static MetaCommandResult do_meta_command(const sdb::str_t& command) {
  if (command == ".exit")
    exit(EXIT_SUCCESS);
  else
    return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
}

int main(int argc, char* argv[]) {
  SDB_UNUSED(argc), SDB_UNUSED(argv);

  sdb::str_t command;
  for (;;) {
    std::cout << "db > ";
    if (!std::getline(std::cin, command))
      break;

    if (command[0] == '.') {
      switch (do_meta_command(command)) {
      case MetaCommandResult::META_COMMAND_SUCCESS:
        continue;
      case MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND:
        std::cerr << "Unrecognized command `" << command << "`" << std::endl;
        continue;
      }
    }

    SQLCompiler compiler;
    switch (compiler.prepare(command)) {
    case SQLCompiler::PrepareRet::SUCCESS:
      break;
    case SQLCompiler::PrepareRet::UNRECONGNIZED_STATEMENT:
      std::cerr << "Unrecognized keyword at start of `" << command << "`" << std::endl;
      continue;
    }

    compiler.execute();
    std::cout << "Executed" <<  std::endl;
  }

  return 0;
}
