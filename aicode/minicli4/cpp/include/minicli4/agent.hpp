#pragma once

#include <functional>
#include <string>
#include <vector>

#include "minicli4/config.hpp"
#include "minicli4/provider.hpp"
#include "minicli4/session.hpp"
#include "minicli4/tools.hpp"

namespace minicli4 {

struct AgentCallbacks {
  std::function<void(const std::string&, const std::string&)> on_stage;
  std::function<void(const std::string&, const std::string&)> on_tool;
  std::function<void(const std::string&)> on_delta;
};

struct AgentResult {
  std::string final;
  std::vector<std::string> stages;
};

AgentResult run_multi_agent_round(
    const std::string& input_text,
    const std::vector<SessionMessage>& session_messages,
    IProvider& provider,
    const ToolRegistry& tools,
    const RuntimeConfig& config,
    const AgentCallbacks& callbacks);

}  // namespace minicli4
