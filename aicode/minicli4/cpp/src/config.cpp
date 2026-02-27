#include "minicli4/config.hpp"

#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>

namespace minicli4 {

namespace {
std::filesystem::path config_path_for(const std::filesystem::path& project_root) {
  return state_root(project_root) / "config.json";
}

void ensure_state(const std::filesystem::path& project_root) {
  const auto root = state_root(project_root);
  std::filesystem::create_directories(root / "sessions");
  std::filesystem::create_directories(root / "logs");
  std::filesystem::create_directories(root / "cache");
}

std::string read_file_text(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream out;
  out << in.rdbuf();
  return out.str();
}

std::string extract_string(const std::string& json, const std::string& key) {
  const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
  std::smatch m;
  if (std::regex_search(json, m, pattern) && m.size() > 1) {
    return m[1].str();
  }
  return {};
}

bool extract_bool(const std::string& json, const std::string& key, bool fallback) {
  const std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)");
  std::smatch m;
  if (std::regex_search(json, m, pattern) && m.size() > 1) {
    return m[1].str() == "true";
  }
  return fallback;
}

int extract_int(const std::string& json, const std::string& key, int fallback) {
  const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?[0-9]+)");
  std::smatch m;
  if (std::regex_search(json, m, pattern) && m.size() > 1) {
    try {
      return std::stoi(m[1].str());
    } catch (...) {
      return fallback;
    }
  }
  return fallback;
}

double extract_double(const std::string& json, const std::string& key, double fallback) {
  const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)");
  std::smatch m;
  if (std::regex_search(json, m, pattern) && m.size() > 1) {
    try {
      return std::stod(m[1].str());
    } catch (...) {
      return fallback;
    }
  }
  return fallback;
}
}  // namespace

std::filesystem::path state_root(const std::filesystem::path& project_root) {
  return project_root / ".minicli4";
}

LoadedConfig load_config(const std::filesystem::path& project_root) {
  ensure_state(project_root);
  RuntimeConfig cfg{};
  const auto cfg_path = config_path_for(project_root);
  if (!std::filesystem::exists(cfg_path)) {
    save_config(project_root, cfg);
  }

  const std::string json = read_file_text(cfg_path);
  if (!json.empty()) {
    const auto api_key = extract_string(json, "api_key");
    if (!api_key.empty()) {
      cfg.api_key = api_key;
    }
    const auto base_url = extract_string(json, "base_url");
    if (!base_url.empty()) {
      cfg.base_url = base_url;
    }
    cfg.stream = extract_bool(json, "stream", cfg.stream);
    cfg.timeout_ms = extract_int(json, "timeout_ms", cfg.timeout_ms);
    cfg.max_retries = extract_int(json, "max_retries", cfg.max_retries);
    cfg.temperature = extract_double(json, "temperature", cfg.temperature);
    cfg.max_tokens = extract_int(json, "max_tokens", cfg.max_tokens);
    cfg.agent_max_rounds = extract_int(json, "agent_max_rounds", cfg.agent_max_rounds);

    const auto safe_mode = extract_string(json, "safe_mode");
    if (!safe_mode.empty()) {
      cfg.safe_mode = safe_mode == "balanced" ? "balanced" : "strict";
    }
    const auto theme = extract_string(json, "theme");
    if (!theme.empty()) {
      cfg.theme = theme == "light" ? "light" : "dark";
    }
    const auto motion = extract_string(json, "motion");
    if (!motion.empty()) {
      cfg.motion = motion == "minimal" ? "minimal" : "full";
    }
  }

  if (const char* env_key = std::getenv("GLM_API_KEY")) {
    if (*env_key != '\0') {
      cfg.api_key = env_key;
    }
  }

  cfg.model = "glm-5";
  return LoadedConfig{cfg, cfg_path, state_root(project_root)};
}

void save_config(const std::filesystem::path& project_root, const RuntimeConfig& config) {
  ensure_state(project_root);
  RuntimeConfig normalized = config;
  normalized.model = "glm-5";
  std::ofstream out(config_path_for(project_root), std::ios::binary);
  out << "{\n"
      << "  \"api_key\": \"" << normalized.api_key << "\",\n"
      << "  \"base_url\": \"" << normalized.base_url << "\",\n"
      << "  \"model\": \"glm-5\",\n"
      << "  \"stream\": " << (normalized.stream ? "true" : "false") << ",\n"
      << "  \"timeout_ms\": " << normalized.timeout_ms << ",\n"
      << "  \"max_retries\": " << normalized.max_retries << ",\n"
      << "  \"temperature\": " << normalized.temperature << ",\n"
      << "  \"max_tokens\": " << normalized.max_tokens << ",\n"
      << "  \"agent_max_rounds\": " << normalized.agent_max_rounds << ",\n"
      << "  \"safe_mode\": \"" << normalized.safe_mode << "\",\n"
      << "  \"theme\": \"" << normalized.theme << "\",\n"
      << "  \"motion\": \"" << normalized.motion << "\"\n"
      << "}\n";
}

}  // namespace minicli4
