#pragma once

#include <map>
#include <string>
#include <vector>

namespace minicli4 {

struct SessionMessage {
  std::string role;
  std::string content;
  std::string created_at;
  std::string tool_call_id;
  std::string name;
};

struct SessionRecord {
  std::string session_id;
  std::string title;
  std::vector<SessionMessage> messages;
  std::string updated_at;
};

class SessionStore {
 public:
  SessionRecord create(const std::string& session_id = "");
  SessionRecord load(const std::string& session_id) const;
  void save(const SessionRecord& rec);
  std::vector<SessionRecord> list() const;
  void set_current(const std::string& session_id);
  SessionRecord current() const;
  bool remove(const std::string& session_id);
  bool rename(const std::string& old_id, const std::string& new_id);

 private:
  std::map<std::string, SessionRecord> records_;
  std::string current_id_;
};

std::string now_iso();

}  // namespace minicli4
