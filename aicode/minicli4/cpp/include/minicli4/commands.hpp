#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "minicli4/config.hpp"
#include "minicli4/session.hpp"
#include "minicli4/tools.hpp"

namespace minicli4 {

struct CommandContext {
  std::filesystem::path project_root;
  RuntimeConfig* config;
  SessionRecord* session;
  SessionStore* sessions;
  ToolRegistry* tools;
  std::function<void(const RuntimeConfig&)> set_config;
  std::function<void(const SessionRecord&)> set_session;
  std::function<void(const std::string&)> out;
  std::function<void(const std::string&)> set_pending_approval;
  std::function<void()> clear_pending_approval;
  std::function<std::string()> pending_approval;
};

const std::vector<std::string>& commands();
bool run_slash(CommandContext& ctx, const std::string& input);

}  // namespace minicli4
