#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "minicli4/config.hpp"

namespace minicli4 {

struct ToolResult {
  bool ok{false};
  std::string output;
  bool requires_approval{false};
};

class ToolRegistry {
 public:
  ToolRegistry(std::filesystem::path project_root, RuntimeConfig config);

  std::vector<std::string> list_tool_names() const;
  ToolResult execute(
      const std::string& name,
      const std::map<std::string, std::string>& args,
      const std::string& session_messages_json = "") const;

 private:
  std::filesystem::path project_root_;
  RuntimeConfig config_;

  bool within_root(const std::string& target) const;
  bool allowed_path(const std::string& target) const;
};

}  // namespace minicli4
