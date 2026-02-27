#include "minicli4/completion.hpp"

#include <algorithm>
#include <map>

namespace minicli4 {

namespace {
const std::map<std::string, std::vector<std::string>> kSubs = {
    {"/config", {"get", "set", "list", "reset"}},
    {"/session", {"list", "new", "switch", "delete", "rename", "current"}},
    {"/tools", {"enable", "disable"}},
    {"/agents", {"list", "set"}},
};

std::vector<std::string> complete_paths(const CompletionContext& ctx, const std::string& prefix) {
  std::vector<std::string> all = ctx.project_paths;
  if (all.empty()) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(ctx.project_root)) {
      const auto rel = std::filesystem::relative(entry.path(), ctx.project_root).generic_string();
      if (rel.rfind(".git/", 0) == 0 || rel.rfind("node_modules/", 0) == 0 || rel.rfind(".minicli4/", 0) == 0) {
        continue;
      }
      all.push_back(rel);
      if (all.size() > 250) {
        break;
      }
    }
  }
  std::vector<std::string> out;
  for (const auto& item : all) {
    if (item.rfind(prefix, 0) == 0) {
      out.push_back(item);
    }
  }
  std::sort(out.begin(), out.end());
  return out;
}

std::vector<std::string> filter_prefix(const std::vector<std::string>& items, const std::string& prefix, bool sorted) {
  std::vector<std::string> out;
  for (const auto& item : items) {
    if (item.rfind(prefix, 0) == 0) {
      out.push_back(item);
    }
  }
  if (sorted) {
    std::sort(out.begin(), out.end());
  }
  return out;
}
}  // namespace

std::vector<std::string> complete(const std::string& text, const CompletionContext& ctx) {
  if (text.empty() || text[0] != '/') {
    return {};
  }
  const bool trailing = !text.empty() && text.back() == ' ';
  std::vector<std::string> parts;
  std::string current;
  for (const char c : text) {
    if (c == ' ') {
      if (!current.empty()) {
        parts.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(c);
  }
  if (!current.empty()) {
    parts.push_back(current);
  }

  if (parts.size() <= 1 && !trailing) {
    const std::string needle = parts.empty() ? "/" : parts.front();
    return filter_prefix(ctx.command_names, needle, true);
  }

  const std::string cmd = parts.empty() ? "" : parts.front();
  const auto sub_it = kSubs.find(cmd);
  if (sub_it != kSubs.end()) {
    if (parts.size() == 1 && trailing) {
      return sub_it->second;
    }
    if (parts.size() == 2 && !trailing) {
      return filter_prefix(sub_it->second, parts[1], true);
    }
  }

  if (cmd == "/session" && parts.size() >= 2 && parts[1] == "switch") {
    const std::string needle = trailing ? "" : (parts.size() > 2 ? parts[2] : "");
    return filter_prefix(ctx.session_ids, needle, true);
  }
  if (cmd == "/config" && parts.size() >= 2 && (parts[1] == "set" || parts[1] == "get")) {
    const std::string needle = trailing ? "" : (parts.size() > 2 ? parts[2] : "");
    return filter_prefix(ctx.config_keys, needle, true);
  }
  if (cmd == "/tools" && parts.size() >= 2 && (parts[1] == "enable" || parts[1] == "disable")) {
    const std::string needle = trailing ? "" : (parts.size() > 2 ? parts[2] : "");
    return filter_prefix(ctx.tool_names, needle, true);
  }

  if (cmd == "/read" || cmd == "/write" || cmd == "/ls" || cmd == "/grep" || cmd == "/tree" || cmd == "/export" || cmd == "/add") {
    const std::string needle = trailing ? "" : (parts.empty() ? "" : parts.back());
    return complete_paths(ctx, needle);
  }

  return {};
}

}  // namespace minicli4
