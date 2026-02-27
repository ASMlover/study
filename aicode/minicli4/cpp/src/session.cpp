#include "minicli4/session.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace minicli4 {

std::string now_iso() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#if defined(_WIN32)
  gmtime_s(&tm, &t);
#else
  gmtime_r(&t, &tm);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  return oss.str();
}

SessionRecord SessionStore::create(const std::string& session_id) {
  std::string sid = session_id;
  if (sid.empty()) {
    sid = "session-" + std::to_string(static_cast<long long>(
                           std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count()));
  }
  SessionRecord rec{sid, sid, {}, now_iso()};
  records_[sid] = rec;
  current_id_ = sid;
  return rec;
}

SessionRecord SessionStore::load(const std::string& session_id) const {
  const auto it = records_.find(session_id);
  if (it == records_.end()) {
    throw std::runtime_error("session not found: " + session_id);
  }
  return it->second;
}

void SessionStore::save(const SessionRecord& rec) {
  SessionRecord next = rec;
  next.updated_at = now_iso();
  records_[next.session_id] = next;
}

std::vector<SessionRecord> SessionStore::list() const {
  std::vector<SessionRecord> out;
  out.reserve(records_.size());
  for (const auto& [_, rec] : records_) {
    out.push_back(rec);
  }
  std::sort(out.begin(), out.end(), [](const SessionRecord& a, const SessionRecord& b) {
    return a.updated_at > b.updated_at;
  });
  return out;
}

void SessionStore::set_current(const std::string& session_id) {
  current_id_ = session_id;
}

SessionRecord SessionStore::current() const {
  if (current_id_.empty()) {
    auto* self = const_cast<SessionStore*>(this);
    return self->create("default");
  }
  const auto it = records_.find(current_id_);
  if (it == records_.end()) {
    auto* self = const_cast<SessionStore*>(this);
    return self->create("default");
  }
  return it->second;
}

bool SessionStore::remove(const std::string& session_id) {
  return records_.erase(session_id) > 0;
}

bool SessionStore::rename(const std::string& old_id, const std::string& new_id) {
  if (records_.count(old_id) == 0 || records_.count(new_id) > 0) {
    return false;
  }
  SessionRecord rec = records_.at(old_id);
  rec.session_id = new_id;
  rec.title = new_id;
  rec.updated_at = now_iso();
  records_.erase(old_id);
  records_[new_id] = rec;
  if (current_id_ == old_id) {
    current_id_ = new_id;
  }
  return true;
}

}  // namespace minicli4
