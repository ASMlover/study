#include "minicli4/agent.hpp"

#include <map>

namespace minicli4 {

namespace {
std::string first_message_content(const ProviderReply& reply) {
  if (reply.response.choices.empty()) {
    return {};
  }
  return reply.response.choices.front().message_content;
}
}  // namespace

AgentResult run_multi_agent_round(
    const std::string& input_text,
    const std::vector<SessionMessage>& session_messages,
    IProvider& provider,
    const ToolRegistry& tools,
    const RuntimeConfig& config,
    const AgentCallbacks& callbacks) {
  (void)session_messages;
  std::vector<std::string> stages;

  callbacks.on_stage("planner", "planning");
  stages.push_back("planner");
  const auto planner = provider.chat(ProviderRequest{{{"user", input_text}}, false, config.temperature, 512});
  const std::string plan = first_message_content(planner);

  callbacks.on_stage("coder", "tool planning and execution");
  stages.push_back("coder");
  auto coding = provider.chat(ProviderRequest{{{"system", "planner_notes:\n" + plan}, {"user", input_text}}, false, config.temperature, config.max_tokens});

  if (!coding.response.choices.empty()) {
    for (const auto& call : coding.response.choices.front().tool_calls) {
      const auto result = tools.execute(call.name, std::map<std::string, std::string>{}, "");
      callbacks.on_tool(call.name, result.output);
    }
  }

  callbacks.on_stage("reviewer", "quality review");
  stages.push_back("reviewer");
  provider.chat(ProviderRequest{{{"system", "review coder output"}}, false, config.temperature, 1024});

  callbacks.on_stage("orchestrator", "final streaming reply");
  stages.push_back("orchestrator");
  const auto final_reply = provider.chat(ProviderRequest{{{"user", input_text}}, config.stream, config.temperature, config.max_tokens});

  std::string final;
  if (final_reply.streamed) {
    for (const auto& chunk : final_reply.stream_chunks) {
      if (chunk.choices.empty()) {
        continue;
      }
      final += chunk.choices.front().delta_content;
      callbacks.on_delta(chunk.choices.front().delta_content);
    }
  } else {
    final = first_message_content(final_reply);
    callbacks.on_delta(final);
  }

  return AgentResult{final, stages};
}

}  // namespace minicli4
