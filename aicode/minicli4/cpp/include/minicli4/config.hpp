#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace minicli4 {

struct RuntimeConfig {
  std::string api_key{};
  std::string base_url{"https://open.bigmodel.cn/api/paas/v4"};
  std::string model{"glm-5"};
  bool stream{true};
  int timeout_ms{120000};
  int max_retries{3};
  double temperature{0.7};
  int max_tokens{4096};
  int agent_max_rounds{6};
  std::string safe_mode{"strict"};
  std::string theme{"dark"};
  std::string motion{"full"};
  std::vector<std::string> allowed_paths{"."};
  std::vector<std::string> shell_allowlist{"pwd", "ls", "dir", "cat", "type", "rg"};
  std::string slash_completion_mode{"contextual"};
};

struct LoadedConfig {
  RuntimeConfig config;
  std::filesystem::path config_path;
  std::filesystem::path state_dir;
};

std::filesystem::path state_root(const std::filesystem::path& project_root);
LoadedConfig load_config(const std::filesystem::path& project_root);
void save_config(const std::filesystem::path& project_root, const RuntimeConfig& config);

}  // namespace minicli4
