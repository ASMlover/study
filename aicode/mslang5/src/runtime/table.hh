#pragma once

#include <string>
#include <unordered_map>

#include "runtime/value.hh"

namespace ms {

class Table {
 public:
  bool Set(const std::string& key, Value value);
  bool Get(const std::string& key, Value* out) const;
  bool Contains(const std::string& key) const;
  std::size_t Size() const;
  const std::unordered_map<std::string, Value>& Data() const;

 private:
  std::unordered_map<std::string, Value> data_;
};

}  // namespace ms

