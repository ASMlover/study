#include "minicli4/tools.hpp"

#include <algorithm>
#include <functional>
#include <fstream>
#include <regex>
#include <sstream>

namespace minicli4 {

ToolRegistry::ToolRegistry(std::filesystem::path project_root, RuntimeConfig config)
    : project_root_(std::move(project_root)), config_(std::move(config)) {}

std::vector<std::string> ToolRegistry::list_tool_names() const {
  return {"read_file", "write_file", "list_dir", "grep_text", "run_shell", "project_tree", "session_export"};
}

bool ToolRegistry::within_root(const std::string& target) const {
  const auto root = std::filesystem::weakly_canonical(project_root_);
  const auto resolved = std::filesystem::weakly_canonical(project_root_ / target);
  return resolved == root || resolved.string().rfind(root.string(), 0) == 0;
}

bool ToolRegistry::allowed_path(const std::string& target) const {
  const auto resolved = std::filesystem::weakly_canonical(project_root_ / target);
  for (const auto& p : config_.allowed_paths) {
    const auto allowed = std::filesystem::weakly_canonical(project_root_ / p);
    if (resolved == allowed || resolved.string().rfind(allowed.string(), 0) == 0) {
      return true;
    }
  }
  return false;
}

ToolResult ToolRegistry::execute(
    const std::string& name,
    const std::map<std::string, std::string>& args,
    const std::string& session_messages_json) const {
  if (name == "read_file") {
    const auto it = args.find("path");
    const std::string path = it == args.end() ? "" : it->second;
    if (path.empty() || !within_root(path) || !allowed_path(path)) {
      return {false, "path denied", false};
    }
    const auto full = std::filesystem::weakly_canonical(project_root_ / path);
    if (!std::filesystem::exists(full)) {
      return {false, "not found", false};
    }
    std::ifstream in(full, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return {true, ss.str(), false};
  }

  if (name == "write_file") {
    if (config_.safe_mode == "strict") {
      return {false, "write requires explicit approval", true};
    }
    const std::string path = args.count("path") ? args.at("path") : "";
    const std::string content = args.count("content") ? args.at("content") : "";
    if (path.empty() || !within_root(path) || !allowed_path(path)) {
      return {false, "path denied", false};
    }
    const auto full = std::filesystem::weakly_canonical(project_root_ / path);
    std::filesystem::create_directories(full.parent_path());
    std::ofstream out(full, std::ios::binary);
    out << content;
    return {true, "wrote " + path, false};
  }

  if (name == "list_dir") {
    const std::string path = args.count("path") ? args.at("path") : ".";
    if (!within_root(path) || !allowed_path(path)) {
      return {false, "path denied", false};
    }
    const auto full = std::filesystem::weakly_canonical(project_root_ / path);
    if (!std::filesystem::exists(full)) {
      return {false, "not found", false};
    }
    std::vector<std::string> entries;
    for (const auto& entry : std::filesystem::directory_iterator(full)) {
      entries.push_back(entry.path().filename().string() + (entry.is_directory() ? "/" : ""));
    }
    std::sort(entries.begin(), entries.end());
    std::ostringstream out;
    for (size_t i = 0; i < entries.size(); i++) {
      out << entries[i];
      if (i + 1 < entries.size()) {
        out << "\n";
      }
    }
    return {true, out.str(), false};
  }

  if (name == "grep_text") {
    const std::string pattern = args.count("pattern") ? args.at("pattern") : "";
    const std::string start = args.count("path") ? args.at("path") : ".";
    if (pattern.empty()) {
      return {false, "missing pattern", false};
    }
    if (!within_root(start) || !allowed_path(start)) {
      return {false, "path denied", false};
    }
    std::regex regex;
    try {
      regex = std::regex(pattern, std::regex::icase);
    } catch (...) {
      return {false, "invalid regex", false};
    }
    std::vector<std::string> matches;
    const auto root = std::filesystem::weakly_canonical(project_root_ / start);
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
      const auto rel = std::filesystem::relative(entry.path(), project_root_).generic_string();
      if (rel.find(".git/") == 0 || rel.find("node_modules/") == 0 || rel.find(".minicli4/") == 0) {
        continue;
      }
      if (!entry.is_regular_file()) {
        continue;
      }
      std::ifstream in(entry.path());
      if (!in.is_open()) {
        continue;
      }
      std::string line;
      int line_no = 0;
      while (std::getline(in, line)) {
        line_no += 1;
        if (std::regex_search(line, regex)) {
          std::ostringstream one;
          one << rel << ":" << line_no << ": " << line;
          matches.push_back(one.str());
          if (matches.size() >= 50) {
            break;
          }
        }
      }
      if (matches.size() >= 50) {
        break;
      }
    }
    if (matches.empty()) {
      return {true, "no matches", false};
    }
    std::ostringstream out;
    for (size_t i = 0; i < matches.size(); i++) {
      out << matches[i];
      if (i + 1 < matches.size()) {
        out << "\n";
      }
    }
    return {true, out.str(), false};
  }

  if (name == "run_shell") {
    const std::string cmd = args.count("command") ? args.at("command") : "";
    if (cmd.empty()) {
      return {false, "missing command", false};
    }
    const auto space = cmd.find(' ');
    const std::string head = cmd.substr(0, space == std::string::npos ? cmd.size() : space);
    if (config_.safe_mode == "strict") {
      if (std::find(config_.shell_allowlist.begin(), config_.shell_allowlist.end(), head) == config_.shell_allowlist.end()) {
        return {false, "command blocked in strict mode: " + head, true};
      }
    }
    return {true, "(execution omitted in tests)", false};
  }

  if (name == "project_tree") {
    const std::string path = args.count("path") ? args.at("path") : ".";
    int depth = 3;
    if (args.count("depth")) {
      depth = std::max(0, std::stoi(args.at("depth")));
    }
    if (!within_root(path) || !allowed_path(path)) {
      return {false, "path denied", false};
    }
    const auto root = std::filesystem::weakly_canonical(project_root_ / path);
    std::ostringstream out;
    std::function<void(const std::filesystem::path&, int, const std::string&)> walk =
        [&](const std::filesystem::path& dir, int d, const std::string& prefix) {
          if (d < 0) {
            return;
          }
          std::vector<std::filesystem::path> items;
          for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            items.push_back(entry.path());
          }
          std::sort(items.begin(), items.end());
          for (const auto& p : items) {
            const bool is_dir = std::filesystem::is_directory(p);
            out << prefix << (is_dir ? "[D] " : "[F] ") << p.filename().string() << "\n";
            if (is_dir) {
              walk(p, d - 1, prefix + "  ");
            }
          }
        };
    walk(root, depth, "");
    return {true, out.str(), false};
  }

  if (name == "session_export") {
    const std::string out_path = args.count("out") ? args.at("out") : ".minicli4/sessions/export.json";
    if (!within_root(out_path)) {
      return {false, "path denied", false};
    }
    const auto full = std::filesystem::weakly_canonical(project_root_ / out_path);
    std::filesystem::create_directories(full.parent_path());
    std::ofstream out(full, std::ios::binary);
    out << session_messages_json;
    return {true, "exported " + out_path, false};
  }

  return {false, "unknown tool: " + name, false};
}

}  // namespace minicli4
