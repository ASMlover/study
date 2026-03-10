#pragma once

#include <string>
#include <unordered_map>

#include "runtime/value.hh"

namespace ms {

class Table {
 public:
  bool set(const std::string& key, Value value);
  bool get(const std::string& key, Value* out) const;
  bool contains(const std::string& key) const;
  std::size_t size() const;
  const std::unordered_map<std::string, Value>& data() const;

 private:
  std::unordered_map<std::string, Value> data_;
};

}  // namespace ms
