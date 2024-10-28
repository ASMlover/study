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
#include "util.hh"

enum class MetaCommandResult {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND,
};

#define SIZE_OF_ATTR(Struct, Attr)                sizeof(((Struct*)0)->Attr)
#define COLUMN_USERNAME_SIZE                      (32)
#define COLUMN_EMAIL_SIZE                         (255)
#define TABLE_MAX_PAGES                           (100)

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

constexpr sdb::u32_t ID_SIZE                          = SIZE_OF_ATTR(Row, id);
constexpr sdb::u32_t USERNAME_SIZE                    = SIZE_OF_ATTR(Row, username);
constexpr sdb::u32_t EMAIL_SIZE                       = SIZE_OF_ATTR(Row, email);
constexpr sdb::u32_t ID_OFFSET                        = 0;
constexpr sdb::u32_t USERNAME_OFFSET                  = ID_OFFSET + ID_SIZE;
constexpr sdb::u32_t EMAIL_OFFSET                     = USERNAME_OFFSET + USERNAME_SIZE;
constexpr sdb::u32_t ROW_SIZE                         = ID_SIZE + USERNAME_SIZE + EMAIL_OFFSET;
constexpr sdb::u32_t PAGE_SIZE                        = 4096;

enum class NodeType {
  NODE_INTERNAL,
  NODE_LEAF,
};

// Common Node Header Layout
constexpr sdb::u32_t NODE_TYPE_SIZE                   = sizeof(sdb::u8_t);
constexpr sdb::u32_t NODE_TYPE_OFFSET                 = 0;
constexpr sdb::u32_t IS_ROOT_SIZE                     = sizeof(sdb::u8_t);
constexpr sdb::u32_t IS_ROOT_OFFSET                   = NODE_TYPE_SIZE;
constexpr sdb::u32_t PARENT_POINTER_SIZE              = sizeof(sdb::u32_t);
constexpr sdb::u32_t PARENT_POINTER_OFFSET            = IS_ROOT_OFFSET + IS_ROOT_SIZE;
constexpr sdb::u32_t COMMON_NODE_HEADER_SIZE          = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

// Leaf Node Header Layout
constexpr sdb::u32_t LEAF_NODE_NUM_CELLS_SIZE         = sizeof(sdb::u32_t);
constexpr sdb::u32_t LEAF_NODE_NUM_CELLS_OFFSET       = COMMON_NODE_HEADER_SIZE;
constexpr sdb::u32_t LEAF_NODE_HEADER_SIZE            = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

// Leaf Node Body Layout
constexpr sdb::u32_t LEAF_NODE_KEY_SIZE               = sizeof(sdb::u32_t);
constexpr sdb::u32_t LEAF_NODE_KEY_OFFSET             = 0;
constexpr sdb::u32_t LEAF_NODE_VALUE_SIZE             = ROW_SIZE;
constexpr sdb::u32_t LEAF_NODE_VALUE_OFFSET           = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
constexpr sdb::u32_t LEAF_NODE_CELL_SIZE              = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
constexpr sdb::u32_t LEAF_NODE_SPACE_FOR_CELLS        = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
constexpr sdb::u32_t LEAF_NODE_MAX_CELLS              = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

constexpr sdb::u32_t LEAF_NODE_RIGHT_SPLIT_COUNT      = (LEAF_NODE_MAX_CELLS + 1) / 2;
constexpr sdb::u32_t LEAF_NODE_LEFT_SPLIT_COUNT       = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

// Internal Node Header Layout
constexpr sdb::u32_t INTERNAL_NODE_NUM_KEYS_SIZE      = sizeof(sdb::u32_t);
constexpr sdb::u32_t INTERNAL_NODE_NUM_KEYS_OFFSET    = COMMON_NODE_HEADER_SIZE;
constexpr sdb::u32_t INTERNAL_NODE_RIGHT_CHILD_SIZE   = sizeof(sdb::u32_t);
constexpr sdb::u32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET = INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
constexpr sdb::u32_t INTERNAL_NODE_HEADER_SIZE        = COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;

// Internal Node Body Layout
constexpr sdb::u32_t INTERNAL_NODE_KEY_SIZE           = sizeof(sdb::u32_t);
constexpr sdb::u32_t INTERNAL_NODE_CHILD_SIZE         = sizeof(sdb::u32_t);
constexpr sdb::u32_t INTERNAL_NODE_CELL_SIZE          = INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;

inline NodeType get_node_type(void* node) noexcept {
  sdb::u8_t value = *((sdb::u8_t*)node + NODE_TYPE_OFFSET);
  return (NodeType)value;
}

inline void set_node_type(void* node, NodeType type) noexcept {
  sdb::u8_t value = (sdb::u8_t)type;
  *((sdb::u8_t*)node + NODE_TYPE_OFFSET) = value;
}

void print_constants() noexcept {
  std::cout << "========= [Constants] =========" << std::endl;
  std::cout << "ROW_SIZE: " << ROW_SIZE << std::endl;
  std::cout << "COMMON_NODE_HEADER_SIZE: " << COMMON_NODE_HEADER_SIZE << std::endl;
  std::cout << "LEAF_NODE_HEADER_SIZE: " << LEAF_NODE_HEADER_SIZE << std::endl;
  std::cout << "LEAF_NODE_CELL_SIZE: " << LEAF_NODE_CELL_SIZE << std::endl;
  std::cout << "LEAF_NODE_SPACE_FOR_CELLS: " << LEAF_NODE_SPACE_FOR_CELLS << std::endl;
  std::cout << "LEAF_NODE_MAX_CELLS: " << LEAF_NODE_MAX_CELLS << std::endl;
}

inline sdb::u32_t* leaf_node_num_cells(void* node) noexcept {
  return (sdb::u32_t*)((sdb::byte_t*)node + LEAF_NODE_NUM_CELLS_OFFSET);
}

inline void* leaf_node_cell(void* node, sdb::u32_t cell_num) noexcept {
  return (sdb::byte_t*)node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

inline sdb::u32_t* leaf_node_key(void* node, sdb::u32_t cell_num) noexcept {
  return (sdb::u32_t*)leaf_node_cell(node, cell_num);
}

inline void* leaf_node_value(void* node, sdb::u32_t cell_num) noexcept {
  return (sdb::byte_t*)leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

inline void indent(sdb::u32_t level) noexcept {
  for (sdb::u32_t i = 0; i < level; ++i)
    std::cout << "  ";
}

inline sdb::u32_t* internal_node_num_keys(void* node) noexcept {
  return (sdb::u32_t*)((sdb::u8_t*)node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

inline sdb::u32_t* internal_node_right_child(void* node) noexcept {
  return (sdb::u32_t*)((sdb::u8_t*)node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

inline sdb::u32_t* internal_node_cell(void* node, sdb::u32_t cell_num) noexcept {
  return (sdb::u32_t*)((sdb::u8_t*)node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

inline sdb::u32_t* internal_node_child(void* node, sdb::u32_t child_num) noexcept {
  sdb::u32_t num_keys = *internal_node_num_keys(node);
  if (child_num > num_keys) {
    std::cout << "Tried to access child_num " << child_num << " > " << num_keys << std::endl;
    std::exit(EXIT_FAILURE);
  }
  else if (child_num == num_keys) {
    return internal_node_right_child(node);
  }
  else {
    return internal_node_cell(node, child_num);
  }
}

inline sdb::u32_t* internal_node_key(void* node, sdb::u32_t key_num) noexcept {
  return (sdb::u32_t*)((sdb::u8_t*)internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE);
}

inline sdb::u32_t get_node_max_key(void* node) noexcept {
  switch (get_node_type(node)) {
  case NodeType::NODE_INTERNAL:
    return *internal_node_key(node, *internal_node_num_keys(node) - 1);
  case NodeType::NODE_LEAF:
    return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
  default: break;
  }
  return 0;
}

inline bool is_node_root(void* node) noexcept {
  sdb::u8_t value = *((sdb::u8_t*)node + IS_ROOT_OFFSET);
  return sdb::as_type<bool>(value);
}

inline void set_node_root(void* node, bool is_root) noexcept {
  sdb::u8_t value = sdb::as_type<bool>(is_root);
  *((sdb::u8_t*)node + IS_ROOT_OFFSET) = value;
}

inline void initialize_leaf_node(void* node) noexcept {
  set_node_type(node, NodeType::NODE_LEAF);
  set_node_root(node, false);
  *leaf_node_num_cells(node) = 0;
}

inline void initialize_internal_node(void* node) noexcept {
  set_node_type(node, NodeType::NODE_INTERNAL);
  set_node_root(node, false);
  *internal_node_num_keys(node) = 0;
}

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
  int                                       _file_descriptor;
  sdb::u32_t                                _file_length;
  sdb::u32_t                                _num_pages;
  void*                                     _pages[TABLE_MAX_PAGES];

  Pager(int fd, sdb::u32_t flen) noexcept
    : _file_descriptor{fd}, _file_length{flen} {
    _num_pages = _file_length / PAGE_SIZE;
    if (0 != _file_length % PAGE_SIZE) {
      std::cerr << "DB file is not a whole number of pages. Corrpt file." << std::endl;
      std::exit(EXIT_FAILURE);
    }

    for (int i = 0; i < TABLE_MAX_PAGES; ++i)
      _pages[i] = NULL;
  }

  ~Pager() noexcept {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i)
      reclaim_page(i);
  }

  inline void* acquire_page() noexcept {
    return malloc(PAGE_SIZE);
  }

  inline void reclaim_page(int i) noexcept {
    if (nullptr != _pages[i]) {
      free(_pages[i]);
      _pages[i] = nullptr;
    }
  }

  inline bool is_page_valid(int i) const noexcept { return nullptr != _pages[i]; }
  inline int file_descriptor() const noexcept { return _file_descriptor; }
  inline sdb::u32_t file_length() const noexcept { return _file_length; }
  inline sdb::u32_t num_pages() const noexcept { return _num_pages; }
  inline sdb::u32_t get_unused_page_num() const noexcept { return _num_pages; }

  void flush(sdb::u32_t page_num) noexcept {
    if (NULL == _pages[page_num]) {
      std::cerr << "Tried to flush null page" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    off_t offset = sdb::lseek(_file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    if (-1 == offset) {
      std::cerr << "Error seeling: " << errno << std::endl;
      std::exit(EXIT_FAILURE);
    }

    sdb::ssz_t bytes_written = sdb::write(_file_descriptor, _pages[page_num], PAGE_SIZE);
    if (-1 == bytes_written) {
      std::cerr << "Error writing: " << errno << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  void* get_page(sdb::u32_t page_num) noexcept {
    if (page_num > TABLE_MAX_PAGES) {
      std::cerr << "Tried to fetch page number out of bounds. " << TABLE_MAX_PAGES << std::endl;
      std::exit(EXIT_FAILURE);
    }

    if (NULL == _pages[page_num]) {
      void* page = acquire_page();
      sdb::u32_t num_pages = _file_length / PAGE_SIZE;

      if (_file_length % PAGE_SIZE)
        num_pages += 1;

      if (page_num <= num_pages) {
        sdb::lseek(_file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
        sdb::ssz_t bytes_read = sdb::read(_file_descriptor, page, PAGE_SIZE);
        if (-1 == bytes_read) {
          std::cerr << "Error reading file: " << errno << std::endl;
          std::exit(EXIT_FAILURE);
        }
      }
      _pages[page_num] = page;

      if (page_num >= _num_pages)
        _num_pages = page_num + 1;
    }

    return _pages[page_num];
  }

  void print_tree(sdb::u32_t page_num, sdb::u32_t indentation_level) noexcept {
    void* node = get_page(page_num);
    sdb::u32_t num_keys, child;

    switch (get_node_type(node)) {
    case NodeType::NODE_LEAF:
      num_keys = *leaf_node_num_cells(node);
      indent(indentation_level);
      std::cout << "- leaf (size " << num_keys << ")" << std::endl;
      for (sdb::u32_t i = 0; i < num_keys; ++i) {
        indent(indentation_level + 1);
        std::cout << "- " << *leaf_node_key(node, i) << std::endl;
      }
      break;
    case NodeType::NODE_INTERNAL:
      num_keys = *internal_node_num_keys(node);
      indent(indentation_level);
      std::cout << "- internal (size " << num_keys << ")" << std::endl;
      for (sdb::u32_t i = 0; i < num_keys; ++i) {
        child = *internal_node_child(node, i);
        print_tree(child, indentation_level + 1);

        indent(indentation_level + 1);
        std::cout << "- key " << *internal_node_key(node, i) << std::endl;
      }
      child = *internal_node_right_child(node);
      print_tree(child, indentation_level + 1);
      break;
    default: break;
    }
  }

  static Pager* pager_open(const char* filename) noexcept {
    int fd = sdb::open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (-1 == fd) {
      std::cerr << "Unable to open file" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    off_t file_length = sdb::lseek(fd, 0, SEEK_END);
    return new Pager(fd, sdb::as_type<sdb::u32_t>(file_length));
  }
};

struct Cursor;
struct Table {
  sdb::u32_t                                _root_page_num{};
  Pager*                                    _pager;

  Table(Pager* p, sdb::u32_t root_page_num = 0) noexcept
    : _pager{p}, _root_page_num{root_page_num} {
  }

  ~Table() noexcept {}

  inline sdb::u32_t root_page_num() const noexcept { return _root_page_num; }
  inline Pager* pager() const noexcept { return _pager; }

  Cursor* leaf_node_find(sdb::u32_t page_num, sdb::u32_t key) noexcept;
  Cursor* find(sdb::u32_t key) noexcept;
  Cursor* internal_node_find(sdb::u32_t page_num, sdb::u32_t key) noexcept;

  static Table* db_open(const char* filename) noexcept {
    Pager* pager = Pager::pager_open(filename);

    Table* table = new Table{pager, 0};
    if (0 == pager->_num_pages) {
      void* root_node = pager->get_page(0);
      initialize_leaf_node(root_node);
      set_node_root(root_node, true);
    }
    return table;
  }

  static void db_close(Table* table) noexcept {
    Pager* pager = table->pager();

    for (sdb::u32_t i = 0; i < pager->num_pages(); ++i) {
      if (!pager->is_page_valid(i))
        continue;

      pager->flush(i);
      pager->reclaim_page(i);
    }

    int result = sdb::close(pager->file_descriptor());
    if (-1 == result) {
      std::cerr << "Error closing db file" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    for (sdb::u32_t i = 0; i < TABLE_MAX_PAGES; ++i)
      pager->reclaim_page(i);
    delete pager;
    delete table;
  }

  bool insert(Row& row) noexcept;
  void select() noexcept;
  void create_new_root(sdb::u32_t right_child_page_num) noexcept;
};

struct Cursor {
  Table*                                    _table;
  sdb::u32_t                                _page_num;
  sdb::u32_t                                _cell_num;
  bool                                      _end_of_table;

  Cursor(Table* tb, sdb::u32_t page_num, sdb::u32_t cell_num, bool is_end) noexcept
    : _table{tb}, _page_num{page_num}, _cell_num{cell_num}, _end_of_table{is_end} {
  }

  static Cursor* start(Table* table) noexcept {
    void* root_node = table->pager()->get_page(table->_root_page_num);
    sdb::u32_t num_cells = *leaf_node_num_cells(root_node);
    return new Cursor(table, table->root_page_num(), 0, 0 == num_cells);
  }

  static Cursor* end(Table* table) noexcept {
    void* root_node = table->pager()->get_page(table->root_page_num());
    sdb::u32_t num_cells = *leaf_node_num_cells(root_node);
    return new Cursor(table, table->root_page_num(), num_cells, true);
  }

  static Cursor* make_cursor(Table* table, sdb::u32_t page_num) noexcept {
    return new Cursor(table, page_num, 0, false);
  }

  static void reclaim(Cursor* cursor) noexcept {
    if (nullptr != cursor)
      delete cursor;
  }

  inline void set_cell_num(sdb::u32_t cell_num) noexcept {
    _cell_num = cell_num;
  }

  inline bool end_of_table() const noexcept {
    return _end_of_table;
  }

  inline void advance() noexcept {
    sdb::u32_t page_num = _page_num;
    void* node = _table->pager()->get_page(page_num);
    _cell_num += 1;
    if (_cell_num >= (*leaf_node_num_cells(node)))
      _end_of_table = true;
  }

  void* value() noexcept {
    sdb::u32_t page_num = _page_num;
    void* page = _table->pager()->get_page(page_num);
    return leaf_node_value(page, _cell_num);
  }

  void leaf_node_insert(sdb::u32_t key, Row* value) noexcept {
    void* node = _table->pager()->get_page(_page_num);
    sdb::u32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS) {
      leaf_node_split_and_insert(key, value);
      return;
    }

    if (_cell_num < num_cells) {
      for (auto i = num_cells; i > _cell_num; --i)
        std::memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
    }
    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, _cell_num)) = key;
    value->serialize(leaf_node_value(node, _cell_num));
  }

  void leaf_node_split_and_insert(sdb::u32_t key, Row* value) noexcept {
    void* old_node = _table->pager()->get_page(_page_num);
    sdb::u32_t new_page_num = _table->pager()->get_unused_page_num();
    void* new_node = _table->pager()->get_page(new_page_num);
    initialize_leaf_node(new_node);

    for (sdb::u32_t i = LEAF_NODE_MAX_CELLS; i >= 0; --i) {
      void* destination_node;
      if (i >= LEAF_NODE_LEFT_SPLIT_COUNT)
        destination_node = new_node;
      else
        destination_node = old_node;
      sdb::u32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
      void* destination = leaf_node_cell(destination_node, index_within_node);

      if (i == _cell_num)
        value->serialize(destination);
      else if (i > _cell_num)
        std::memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
      else
        std::memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }

    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node)) {
      return _table->create_new_root(new_page_num);
    }
    else {
      std::cout << "Need to implement updating parent after split" << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
};

Cursor* Table::leaf_node_find(sdb::u32_t page_num, sdb::u32_t key) noexcept {
  void* node = _pager->get_page(page_num);
  sdb::u32_t num_cells = *leaf_node_num_cells(node);

  Cursor* cursor = Cursor::make_cursor(this, page_num);
  sdb::u32_t min_index = 0;
  sdb::u32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    sdb::u32_t index = (min_index + one_past_max_index) / 2;
    sdb::u32_t key_at_index = *leaf_node_key(node, key);
    if (key == key_at_index) {
      cursor->set_cell_num(index);
      return cursor;
    }
    if (key < key_at_index)
      one_past_max_index = index;
    else
      min_index = index + 1;
  }

  cursor->set_cell_num(min_index);
  return cursor;
}

Cursor* Table::find(sdb::u32_t key) noexcept {
  sdb::u32_t root_page_num = _root_page_num;
  void* root_node = _pager->get_page(root_page_num);
  if (NodeType::NODE_LEAF == get_node_type(root_node)) {
    return leaf_node_find(root_page_num, key);
  }
  else {
    std::cerr << "Need to implement searching an internal node" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return nullptr;
}

bool Table::insert(Row& row) noexcept {
  void* node = _pager->get_page(_root_page_num);
  sdb::u32_t num_cells = *leaf_node_num_cells(node);

  Cursor* cursor = Cursor::end(this);
  cursor->leaf_node_insert(row.id, &row);
  Cursor::reclaim(cursor);

  return true;
}

void Table::select() noexcept {
  Cursor* cursor = Cursor::start(this);

  Row row;
  while (!(cursor->end_of_table())) {
    row.deserialize(cursor->value());
    row.print();
    cursor->advance();
  }
  Cursor::reclaim(cursor);
}

void Table::create_new_root(sdb::u32_t right_child_page_num) noexcept {
  void* root = _pager->get_page(_root_page_num);
  void* right_child = _pager->get_page(right_child_page_num);
  sdb::u32_t left_child_page_num = _pager->get_unused_page_num();
  void* left_child = _pager->get_page(left_child_page_num);

  std::memcpy(left_child, root, PAGE_SIZE);
  set_node_root(left_child, false);

  initialize_internal_node(root);
  set_node_root(root, true);

  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;
  sdb::u32_t left_child_max_key = get_node_max_key(left_child);
  *internal_node_key(root, 0) = left_child_max_key;
  *internal_node_right_child(root) = right_child_page_num;
}

Cursor* Table::internal_node_find(sdb::u32_t page_num, sdb::u32_t key) noexcept {
  return nullptr;
}


class SQLCompiler final : private sdb::UnCopyable {
public:
  enum ExecuteRet : int {
    SUCCESS,
    DUPLICATE_KEY,
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

static MetaCommandResult do_meta_command(const sdb::str_t& command, Table* table) {
  if (command == ".exit") {
    Table::db_close(table);
    std::exit(EXIT_SUCCESS);
  }
  else if (command == ".btree") {
    table->pager()->print_tree(0, 0);
    return MetaCommandResult::META_COMMAND_SUCCESS;
  }
  else if (command == ".constants") {
    print_constants();
    return MetaCommandResult::META_COMMAND_SUCCESS;
  }
  else {
    return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

int main(int argc, char* argv[]) {
  SDB_UNUSED(argc), SDB_UNUSED(argv);

  if (argc < 2) {
    std::cerr << "Must supply a database filename" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
  Table* table = Table::db_open(filename);

  sdb::str_t command;
  for (;;) {
    std::cout << "db > ";
    if (!std::getline(std::cin, command))
      break;

    if (command[0] == '.') {
      switch (do_meta_command(command, table)) {
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

    compiler.execute(*table, row);
    std::cout << "Executed" <<  std::endl;
  }

  return 0;
}
