#include "runtime/table.hh"

namespace ms {

bool Table::Set(const std::string& key, Value value) {
  auto [it, inserted] = data_.insert_or_assign(key, std::move(value));
  (void)it;
  return inserted;
}

bool Table::Get(const std::string& key, Value* out) const {
  const auto it = data_.find(key);
  if (it == data_.end()) {
    return false;
  }
  if (out != nullptr) {
    *out = it->second;
  }
  return true;
}

bool Table::Contains(const std::string& key) const {
  return data_.contains(key);
}

std::size_t Table::Size() const { return data_.size(); }

const std::unordered_map<std::string, Value>& Table::Data() const {
  return data_;
}

}  // namespace ms

