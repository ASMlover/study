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
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "common.hh"

#if defined(SDB_WINDOWS)
# include <io.h>
#else
# include <unistd.h>
#endif

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
  char                                      username[COLUMN_USERNAME_SIZE + 1];
  char                                      email[COLUMN_EMAIL_SIZE + 1];

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

struct Pager {
  int                                       file_descriptor;
  sdb::u32_t                                file_length;
  void*                                     pages[TABLE_MAX_PAGES];

  Pager(int fd, sdb::u32_t flen) noexcept
    : file_descriptor{fd}, file_length{flen} {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i)
      pages[i] = NULL;
  }

  ~Pager() noexcept {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
      if (NULL != pages[i])
        free(pages[i]);
    }
  }

  static Pager* pager_open(const char* filename) noexcept {
#if defined(SDB_WINDOWS)
    int fd = _open(filename, _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);
#else
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
#endif
    if (-1 == fd) {
      std::cerr << "Unable to open file" << std::endl;
      std::exit(EXIT_FAILURE);
    }

#if defined(SDB_WINDOWS)
    off_t file_length = _lseek(fd, 0, SEEK_END);
#else
    off_t file_length = lseek(fd, 0, SEEK_END);
#endif

    return new Pager(fd, sdb::as_type<sdb::u32_t>(file_length));
  }
};

struct Table {
  sdb::u32_t                                num_rows{};
  Pager*                                    pager;

  Table(sdb::u32_t rows, Pager* p) noexcept
    : num_rows{rows}, pager{p} {
  }

  ~Table() noexcept {}

  static Table* db_open(const char* filename) noexcept {
    Pager* pager = Pager::pager_open(filename);
    sdb::u32_t num_rows = pager->file_length / ROW_SIZE;

    return new Table(num_rows, pager);
  }

  static void db_close(Table* table) noexcept {
  }

  void* get_page(Pager* pager, sdb::u32_t page_num) noexcept {
    if (page_num > TABLE_MAX_PAGES) {
      std::cerr << "Tried to fetch page number out of bounds. " << TABLE_MAX_PAGES << std::endl;
      std::exit(EXIT_FAILURE);
    }

    if (NULL == pager->pages[page_num]) {
      void* page = malloc(PAGE_SIZE);
      sdb::u32_t num_pages = pager->file_length / PAGE_SIZE;

      if (pager->file_length % PAGE_SIZE)
        num_pages += 1;

      if (page_num <= num_pages) {
#if defined(SDB_WINDOWS)
        _lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
        sdb::ssz_t bytes_read = _read(pager->file_descriptor, page, PAGE_SIZE);
#else
        lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
        sdb::ssz_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
#endif
        if (-1 == bytes_read) {
          std::cerr << "Error reading file: " << errno << std::endl;
          std::exit(EXIT_FAILURE);
        }
      }
      pager->pages[page_num] = page;
    }

    return pager->pages[page_num];
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

  inline bool insert(Row& row) noexcept {
    if (num_rows >= TABLE_MAX_ROWS)
      return false;

    row.serialize(row_slot(num_rows));
    num_rows += 1;
    return true;
  }

  inline void select() noexcept {
    Row row;
    for (sdb::u32_t i = 0; i < num_rows; ++i) {
      row.deserialize(row_slot(i));
      row.print();
    }
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
    NEGATIVE_ID,
    STRING_TOO_LONG,
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
    return (!table.insert(row)) ? ExecuteRet::TABLE_FULL : ExecuteRet::SUCCESS;
  }

  inline ExecuteRet _select(Table& table) noexcept {
    table.select();
    return ExecuteRet::SUCCESS;
  }

  PrepareRet prepare_insert(const sdb::str_t& command, Row& row) noexcept {
    type_ = StatementType::INSERT;

    std::vector<sdb::str_t> tokens;
    sdb::string_split(command, " ", 4, tokens);

    if (4 != tokens.size())
      return PrepareRet::SYNTAX_ERROR;

    sdb::str_t keyword = tokens[0];
    sdb::str_t id_string = tokens[1];
    sdb::str_t username = tokens[2];
    sdb::str_t email = tokens[3];

    int id = std::atoi(id_string.data());
    if (id < 0)
      return PrepareRet::NEGATIVE_ID;
    if (username.size() > COLUMN_USERNAME_SIZE)
      return PrepareRet::STRING_TOO_LONG;
    if (email.size() > COLUMN_EMAIL_SIZE)
      return PrepareRet::STRING_TOO_LONG;

    row.id = sdb::as_type<sdb::u32_t>(id);
    std::strncpy(row.username, username.data(), username.size() + 1);
    std::strncpy(row.email, email.data(), email.size() + 1);

    std::cout << id << " " << username << " " << email << std::endl;

    return PrepareRet::SUCCESS;
  }

  void split(const sdb::str_t& s, std::vector<sdb::str_t>& tokens) noexcept {
    tokens.clear();

    std::istringstream iss(s);
    std::copy(std::istream_iterator<sdb::str_t>(iss), std::istream_iterator<sdb::str_t>(), std::back_inserter(tokens));
  }
public:
  PrepareRet prepare(const sdb::str_t& command, Row& row) noexcept {
    if (command.starts_with("insert")) {
      return prepare_insert(command, row);
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
    case SQLCompiler::PrepareRet::STRING_TOO_LONG:
      std::cerr << "String is too long." << std::endl;
      continue;
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
