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

#define SIZE_OF_ATTR(Struct, Attr)          sizeof(((Struct*)0)->Attr)
#define COLUMN_USERNAME_SIZE                (32)
#define COLUMN_EMAIL_SIZE                   (255)
#define TABLE_MAX_PAGES                     (100)

struct Row {
  sdb::u32_t                                id;
  char                                      username[COLUMN_USERNAME_SIZE];
  char                                      email[COLUMN_EMAIL_SIZE];

  inline void print() noexcept {
    std::cout << "(" << id << ", " << username << ", " << email << ")" << std::endl;
  }

  inline void serialize(void* destination) noexcept;
  inline void deserialize(const void* source) noexcept;
};

constexpr sdb::u32_t ID_SIZE                = SIZE_OF_ATTR(Row, id);
constexpr sdb::u32_t USERNAME_SIZE          = SIZE_OF_ATTR(Row, username);
constexpr sdb::u32_t EMAIL_SIZE             = SIZE_OF_ATTR(Row, email);
constexpr sdb::u32_t ID_OFFSET              = 0;
constexpr sdb::u32_t USERNAME_OFFSET        = ID_OFFSET + ID_SIZE;
constexpr sdb::u32_t EMAIL_OFFSET           = USERNAME_OFFSET + USERNAME_SIZE;
constexpr sdb::u32_t ROW_SIZE               = ID_SIZE + USERNAME_SIZE + EMAIL_OFFSET;
constexpr sdb::u32_t PAGE_SIZE              = 4096;
constexpr sdb::u32_t ROWS_PER_PAGE          = PAGE_SIZE / ROW_SIZE;
constexpr sdb::u32_t TABLE_MAX_ROWS         = ROWS_PER_PAGE * TABLE_MAX_PAGES;

inline void Row::serialize(void* destination) noexcept {
  memcpy((sdb::byte_t*)destination + ID_OFFSET, &id, ID_SIZE);
  memcpy((sdb::byte_t*)destination + USERNAME_OFFSET, &username, USERNAME_SIZE);
  memcpy((sdb::byte_t*)destination + EMAIL_OFFSET, &email, EMAIL_SIZE);
}

inline void Row::deserialize(const void* source) noexcept {
  memcpy(&id, (sdb::byte_t*)source + ID_OFFSET, ID_SIZE);
  memcpy(&username, (sdb::byte_t*)source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&email, (sdb::byte_t*)source + EMAIL_OFFSET, EMAIL_SIZE);
}

struct Table {
  sdb::u32_t                                num_rows{};
  void*                                     pages[TABLE_MAX_PAGES];

  Table() noexcept {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i)
      pages[i] = NULL;
  }

  ~Table() noexcept {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
      if (NULL != pages[i])
        free(pages[i]);
    }
  }

  inline void* row_slot(sdb::u32_t row_num) noexcept {
    sdb::u32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = pages[page_num];
    if (NULL == page) {
      page = pages[page_num] = malloc(PAGE_SIZE);
    }

    sdb::u32_t row_offset = row_num % ROWS_PER_PAGE;
    sdb::u32_t byte_offset = row_offset * ROW_SIZE;
    return (sdb::byte_t*)page + byte_offset;
  }
};


class SQLCompiler final : private sdb::UnCopyable {
public:
  enum ExecuteRet : int {
    SUCCESS,
    TABLE_FULL,
  };

  enum class PrepareRet : int {
    SUCCESS,
    SYNTAX_ERROR,
    UNRECONGNIZED_STATEMENT,
  };

  enum class StatementType : int {
    INSERT,
    SELECT,
  };
private:
  StatementType type_;

  inline ExecuteRet _insert(Table& table, Row& row) noexcept {
    if (table.num_rows >= TABLE_MAX_ROWS)
      return ExecuteRet::TABLE_FULL;

    row.serialize(table.row_slot(table.num_rows));
    table.num_rows += 1;

    return ExecuteRet::SUCCESS;
  }

  inline ExecuteRet _select(Table& table) noexcept {
    Row row;
    for (sdb::u32_t i = 0; i < table.num_rows; ++i) {
      row.deserialize(table.row_slot(i));
      row.print();
    }

    return ExecuteRet::SUCCESS;
  }
public:
  PrepareRet prepare(const sdb::str_t& command, Row& row) noexcept {
    if (command.starts_with("insert")) {
      type_ = StatementType::INSERT;
      int args_assigned = sscanf(command.data(), "insert %u %s %s", &row.id, row.username, row.email);
      if (args_assigned < 3)
        return PrepareRet::SYNTAX_ERROR;
      return PrepareRet::SUCCESS;
    }
    else if (command.starts_with("select")) {
      type_ = StatementType::SELECT;
      return PrepareRet::SUCCESS;
    }

    return PrepareRet::UNRECONGNIZED_STATEMENT;
  }

  void execute(Table& table, Row& row) noexcept {
    switch (type_) {
    case StatementType::INSERT: _insert(table, row); break;
    case StatementType::SELECT: _select(table); break;
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

  Table table;
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
    Row row;
    switch (compiler.prepare(command, row)) {
    case SQLCompiler::PrepareRet::SUCCESS:
      break;
    case SQLCompiler::PrepareRet::SYNTAX_ERROR:
      std::cerr << "Syntax error. Could not parse statement." << std::endl;
      continue;
    case SQLCompiler::PrepareRet::UNRECONGNIZED_STATEMENT:
      std::cerr << "Unrecognized keyword at start of `" << command << "`" << std::endl;
      continue;
    }

    compiler.execute(table, row);
    std::cout << "Executed" <<  std::endl;
  }

  return 0;
}
