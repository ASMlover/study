#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace minicli4 {

struct CompletionContext {
  std::vector<std::string> command_names;
  std::vector<std::string> session_ids;
  std::vector<std::string> config_keys;
  std::vector<std::string> tool_names;
  std::filesystem::path project_root;
  std::vector<std::string> project_paths;
};

std::vector<std::string> complete(const std::string& text, const CompletionContext& ctx);

}  // namespace minicli4
