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
  inline std::size_t size() const noexcept { return data_.size(); }
  inline const std::unordered_map<std::string, Value>& data() const noexcept {
    return data_;
  }

 private:
  std::unordered_map<std::string, Value> data_;
};

}  // namespace ms
