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
#include <algorithm>
#include <cctype>
#include <format>
#include "VM.hh"

namespace ms {

bool VM::invoke_string_method(ObjString* str, ObjString* name, int arg_count) noexcept {
  strv_t sv = str->value();
  strv_t method_name = name->value();

  if (method_name == "len") {
    if (arg_count != 0) {
      runtime_error("len() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(static_cast<double>(sv.length()));
    return true;
  } else if (method_name == "slice") {
    if (arg_count != 2) {
      runtime_error("slice() takes exactly 2 arguments (start, end).");
      return false;
    }
    if (!peek(1).is_number() || !peek(0).is_number()) {
      runtime_error("slice() arguments must be numbers.");
      return false;
    }
    auto len = static_cast<i32_t>(sv.length());
    auto start = static_cast<i32_t>(peek(1).as_number());
    auto end = static_cast<i32_t>(peek(0).as_number());
    if (start < 0) start += len;
    if (end < 0) end += len;
    start = std::clamp(start, 0, len);
    end = std::clamp(end, start, len);
    ObjString* res = copy_string(sv.data() + start, static_cast<sz_t>(end - start));
    stack_top_[-arg_count - 1] = Value(static_cast<Object*>(res));
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "find") {
    if (arg_count != 1) {
      runtime_error("find() takes exactly 1 argument.");
      return false;
    }
    if (!peek(0).is_string()) {
      runtime_error("find() argument must be a string.");
      return false;
    }
    strv_t needle = as_string(peek(0))->value();
    auto pos = sv.find(needle);
    double result = (pos == str_t::npos) ? -1.0 : static_cast<double>(pos);
    stack_top_[-arg_count - 1] = Value(result);
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "replace") {
    if (arg_count != 2) {
      runtime_error("replace() takes exactly 2 arguments (old, new).");
      return false;
    }
    if (!peek(1).is_string() || !peek(0).is_string()) {
      runtime_error("replace() arguments must be strings.");
      return false;
    }
    strv_t old_str = as_string(peek(1))->value();
    strv_t new_str = as_string(peek(0))->value();
    str_t result{sv};
    auto pos = result.find(old_str);
    if (pos != str_t::npos) {
      result.replace(pos, old_str.length(), new_str);
    }
    ObjString* res = copy_string(result.data(), result.length());
    stack_top_[-arg_count - 1] = Value(static_cast<Object*>(res));
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "split") {
    if (arg_count != 1) {
      runtime_error("split() takes exactly 1 argument.");
      return false;
    }
    if (!peek(0).is_string()) {
      runtime_error("split() argument must be a string.");
      return false;
    }
    strv_t delim = as_string(peek(0))->value();
    ObjList* list = allocate<ObjList>();
    // Push list to protect from GC during string allocations
    push(Value(static_cast<Object*>(list)));
    if (delim.empty()) {
      // Split into individual characters
      for (sz_t i = 0; i < sv.length(); ++i) {
        ObjString* ch = copy_string(sv.data() + i, 1);
        list->elements().push_back(Value(static_cast<Object*>(ch)));
      }
    } else {
      sz_t start = 0;
      sz_t pos;
      while ((pos = sv.find(delim, start)) != strv_t::npos) {
        ObjString* s = copy_string(sv.data() + start, pos - start);
        list->elements().push_back(Value(static_cast<Object*>(s)));
        start = pos + delim.length();
      }
      ObjString* s = copy_string(sv.data() + start, sv.length() - start);
      list->elements().push_back(Value(static_cast<Object*>(s)));
    }
    pop(); // pop GC guard
    // Replace receiver + arg with result
    stack_top_[-arg_count - 1] = Value(static_cast<Object*>(list));
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "upper") {
    if (arg_count != 0) {
      runtime_error("upper() takes no arguments.");
      return false;
    }
    str_t result{sv};
    std::transform(result.begin(), result.end(), result.begin(),
      [](unsigned char c) { return std::toupper(c); });
    ObjString* res = copy_string(result.data(), result.length());
    stack_top_[-1] = Value(static_cast<Object*>(res));
    return true;
  } else if (method_name == "lower") {
    if (arg_count != 0) {
      runtime_error("lower() takes no arguments.");
      return false;
    }
    str_t result{sv};
    std::transform(result.begin(), result.end(), result.begin(),
      [](unsigned char c) { return std::tolower(c); });
    ObjString* res = copy_string(result.data(), result.length());
    stack_top_[-1] = Value(static_cast<Object*>(res));
    return true;
  } else if (method_name == "trim") {
    if (arg_count != 0) {
      runtime_error("trim() takes no arguments.");
      return false;
    }
    auto first = sv.find_first_not_of(" \t\n\r\f\v");
    if (first == str_t::npos) {
      ObjString* res = copy_string("", 0);
      stack_top_[-1] = Value(static_cast<Object*>(res));
    } else {
      auto last = sv.find_last_not_of(" \t\n\r\f\v");
      ObjString* res = copy_string(sv.data() + first, last - first + 1);
      stack_top_[-1] = Value(static_cast<Object*>(res));
    }
    return true;
  } else if (method_name == "starts_with") {
    if (arg_count != 1) {
      runtime_error("starts_with() takes exactly 1 argument.");
      return false;
    }
    if (!peek(0).is_string()) {
      runtime_error("starts_with() argument must be a string.");
      return false;
    }
    strv_t prefix = as_string(peek(0))->value();
    bool result = sv.starts_with(prefix);
    stack_top_[-arg_count - 1] = Value(result);
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "ends_with") {
    if (arg_count != 1) {
      runtime_error("ends_with() takes exactly 1 argument.");
      return false;
    }
    if (!peek(0).is_string()) {
      runtime_error("ends_with() argument must be a string.");
      return false;
    }
    strv_t suffix = as_string(peek(0))->value();
    bool result = sv.ends_with(suffix);
    stack_top_[-arg_count - 1] = Value(result);
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "contains") {
    if (arg_count != 1) {
      runtime_error("contains() takes exactly 1 argument.");
      return false;
    }
    if (!peek(0).is_string()) {
      runtime_error("contains() argument must be a string.");
      return false;
    }
    strv_t needle = as_string(peek(0))->value();
    bool result = sv.find(needle) != strv_t::npos;
    stack_top_[-arg_count - 1] = Value(result);
    stack_top_ -= arg_count;
    return true;
  }
  runtime_error(std::format("Undefined string method '{}'.", method_name));
  return false;
}

bool VM::invoke_list_method(ObjList* list, ObjString* name, int arg_count) noexcept {
  strv_t method_name = name->value();
  if (method_name == "len") {
    if (arg_count != 0) {
      runtime_error("len() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(static_cast<double>(list->len()));
    return true;
  } else if (method_name == "push") {
    if (arg_count != 1) {
      runtime_error("push() takes exactly 1 argument.");
      return false;
    }
    Value val = stack_top_[-1];
    list->elements().push_back(val);
    stack_top_ -= 2; // pop arg and receiver
    push(Value());   // return nil
    return true;
  } else if (method_name == "pop") {
    if (arg_count != 0) {
      runtime_error("pop() takes no arguments.");
      return false;
    }
    if (list->elements().empty()) {
      runtime_error("Cannot pop from an empty list.");
      return false;
    }
    Value val = list->elements().back();
    list->elements().pop_back();
    stack_top_[-1] = val;
    return true;
  }
  runtime_error(std::format("Undefined list method '{}'.", method_name));
  return false;
}

bool VM::invoke_tuple_method(ObjTuple* tuple, ObjString* name, int arg_count) noexcept {
  strv_t method_name = name->value();
  if (method_name == "len") {
    if (arg_count != 0) {
      runtime_error("len() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(static_cast<double>(tuple->len()));
    return true;
  }
  runtime_error(std::format("Undefined tuple method '{}'.", method_name));
  return false;
}

bool VM::invoke_map_method(ObjMap* map, ObjString* name, int arg_count) noexcept {
  strv_t method_name = name->value();
  if (method_name == "len") {
    if (arg_count != 0) {
      runtime_error("len() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(static_cast<double>(map->len()));
    return true;
  } else if (method_name == "keys") {
    if (arg_count != 0) {
      runtime_error("keys() takes no arguments.");
      return false;
    }
    ObjList* keys = allocate<ObjList>();
    for (auto& [k, v] : map->entries()) {
      keys->elements().push_back(k);
    }
    stack_top_[-1] = Value(static_cast<Object*>(keys));
    return true;
  } else if (method_name == "values") {
    if (arg_count != 0) {
      runtime_error("values() takes no arguments.");
      return false;
    }
    ObjList* vals = allocate<ObjList>();
    for (auto& [k, v] : map->entries()) {
      vals->elements().push_back(v);
    }
    stack_top_[-1] = Value(static_cast<Object*>(vals));
    return true;
  } else if (method_name == "has") {
    if (arg_count != 1) {
      runtime_error("has() takes exactly 1 argument.");
      return false;
    }
    Value key = stack_top_[-1];
    bool found = map->entries().find(key) != map->entries().end();
    stack_top_ -= 2; // pop arg and receiver
    push(Value(found));
    return true;
  } else if (method_name == "remove") {
    if (arg_count != 1) {
      runtime_error("remove() takes exactly 1 argument.");
      return false;
    }
    Value key = stack_top_[-1];
    map->entries().erase(key);
    map->mark_dirty();
    stack_top_ -= 2; // pop arg and receiver
    push(Value()); // return nil
    return true;
  }
  runtime_error(std::format("Undefined map method '{}'.", method_name));
  return false;
}

bool VM::invoke_stringbuilder_method(ObjStringBuilder* sb, ObjString* name, int arg_count) noexcept {
  strv_t method_name = name->value();

  if (method_name == "append") {
    if (arg_count != 1) {
      runtime_error("append() takes exactly 1 argument.");
      return false;
    }
    str_t s = peek(0).stringify();
    sb->append(s);
    // Return the builder itself for chaining
    stack_top_[-arg_count - 1] = Value(static_cast<Object*>(sb));
    stack_top_ -= arg_count;
    return true;
  } else if (method_name == "build") {
    if (arg_count != 0) {
      runtime_error("build() takes no arguments.");
      return false;
    }
    const str_t& buf = sb->buffer();
    ObjString* result = copy_string(buf.data(), buf.length());
    stack_top_[-1] = Value(static_cast<Object*>(result));
    return true;
  } else if (method_name == "len") {
    if (arg_count != 0) {
      runtime_error("len() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(static_cast<double>(sb->len()));
    return true;
  } else if (method_name == "clear") {
    if (arg_count != 0) {
      runtime_error("clear() takes no arguments.");
      return false;
    }
    sb->clear();
    stack_top_[-1] = Value(static_cast<Object*>(sb));
    return true;
  }
  runtime_error(std::format("Undefined StringBuilder method '{}'.", method_name));
  return false;
}

bool VM::invoke_weakref_method(ObjWeakRef* ref, ObjString* name, int arg_count) noexcept {
  strv_t method_name = name->value();

  if (method_name == "get") {
    if (arg_count != 0) {
      runtime_error("get() takes no arguments.");
      return false;
    }
    if (ref->is_alive()) {
      stack_top_[-1] = Value(ref->target());
    } else {
      stack_top_[-1] = Value(); // nil
    }
    return true;
  } else if (method_name == "alive") {
    if (arg_count != 0) {
      runtime_error("alive() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(ref->is_alive());
    return true;
  }
  runtime_error(std::format("Undefined weak_ref method '{}'.", method_name));
  return false;
}

bool VM::invoke_file_method(ObjFile* file, ObjString* name, int arg_count) noexcept {
  strv_t method_name = name->value();

  if (method_name == "read") {
    if (arg_count != 0) {
      runtime_error("read() takes no arguments.");
      return false;
    }
    if (!file->is_open()) {
      runtime_error("Cannot read from a closed file.");
      return false;
    }
    str_t content = file->read_all();
    ObjString* result = copy_string(content.data(), content.length());
    stack_top_[-1] = Value(static_cast<Object*>(result));
    return true;
  } else if (method_name == "readline") {
    if (arg_count != 0) {
      runtime_error("readline() takes no arguments.");
      return false;
    }
    if (!file->is_open()) {
      runtime_error("Cannot read from a closed file.");
      return false;
    }
    if (file->eof()) {
      stack_top_[-1] = Value(); // nil
      return true;
    }
    str_t line = file->read_line();
    if (file->eof() && line.empty()) {
      stack_top_[-1] = Value(); // nil
      return true;
    }
    ObjString* result = copy_string(line.data(), line.length());
    stack_top_[-1] = Value(static_cast<Object*>(result));
    return true;
  } else if (method_name == "write") {
    if (arg_count != 1) {
      runtime_error("write() takes exactly 1 argument.");
      return false;
    }
    if (!file->is_open()) {
      runtime_error("Cannot write to a closed file.");
      return false;
    }
    str_t data = peek(0).stringify();
    file->write(data);
    stack_top_ -= 2; // pop arg and receiver
    push(Value());   // return nil
    return true;
  } else if (method_name == "close") {
    if (arg_count != 0) {
      runtime_error("close() takes no arguments.");
      return false;
    }
    file->close();
    stack_top_[-1] = Value(); // return nil
    return true;
  } else if (method_name == "eof") {
    if (arg_count != 0) {
      runtime_error("eof() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(file->eof());
    return true;
  } else if (method_name == "is_open") {
    if (arg_count != 0) {
      runtime_error("is_open() takes no arguments.");
      return false;
    }
    stack_top_[-1] = Value(file->is_open());
    return true;
  }
  runtime_error(std::format("Undefined file method '{}'.", method_name));
  return false;
}

} // namespace ms
