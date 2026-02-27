#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "minicli4/agent.hpp"
#include "minicli4/cli.hpp"
#include "minicli4/commands.hpp"
#include "minicli4/completion.hpp"
#include "minicli4/config.hpp"
#include "minicli4/provider.hpp"
#include "minicli4/session.hpp"
#include "minicli4/tools.hpp"
#include "minicli4/tui.hpp"

using namespace minicli4;

namespace {

std::string strip_ansi(const std::string& input) {
  std::string out;
  out.reserve(input.size());
  bool in_escape = false;
  for (size_t i = 0; i < input.size(); i++) {
    const char c = input[i];
    if (!in_escape && c == '\x1b' && i + 1 < input.size() && input[i + 1] == '[') {
      in_escape = true;
      continue;
    }
    if (in_escape) {
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        in_escape = false;
      }
      continue;
    }
    out.push_back(c);
  }
  return out;
}

void expect(bool cond, const std::string& message) {
  if (!cond) {
    throw std::runtime_error(message);
  }
}

struct Fixture {
  std::filesystem::path root;
  RuntimeConfig config;
  SessionStore sessions;
  SessionRecord session;
  ToolRegistry tools;
  std::vector<std::string> lines;
  std::string pending;

  Fixture()
      : root(std::filesystem::current_path()),
        config(),
        sessions(),
        session(sessions.create("default")),
        tools(root, config) {}

  CommandContext context() {
    return CommandContext{
        root,
        &config,
        &session,
        &sessions,
        &tools,
        [&](const RuntimeConfig& next) {
          config = next;
          tools = ToolRegistry(root, config);
        },
        [&](const SessionRecord& next) { session = next; },
        [&](const std::string& line) { lines.push_back(line); },
        [&](const std::string& value) { pending = value; },
        [&]() { pending.clear(); },
        [&]() { return pending; },
    };
  }
};

class FakeProvider : public IProvider {
 public:
  explicit FakeProvider(std::vector<ProviderReply> replies) : replies_(std::move(replies)) {}
  ProviderReply chat(const ProviderRequest&) override {
    if (replies_.empty()) {
      throw std::runtime_error("no more fake replies");
    }
    const auto reply = replies_.front();
    replies_.erase(replies_.begin());
    return reply;
  }

 private:
  std::vector<ProviderReply> replies_;
};

void test_commands_baseline() {
  expect(commands().size() == 30, "command baseline must have 30 commands");
  expect(std::find(commands().begin(), commands().end(), "/agents") != commands().end(), "missing /agents");
  expect(std::find(commands().begin(), commands().end(), "/approve") != commands().end(), "missing /approve");
}

void test_help_and_session() {
  Fixture fx;
  auto ctx = fx.context();
  run_slash(ctx, "/help");
  std::ostringstream joined;
  for (const auto& line : fx.lines) {
    joined << line << "\n";
  }
  const auto txt = joined.str();
  expect(txt.find("MiniCLI4 Help") != std::string::npos, "missing help header");
  expect(txt.find("Core:") != std::string::npos, "missing Core category");

  fx.lines.clear();
  run_slash(ctx, "/session new demo");
  expect(fx.session.session_id == "demo", "session should switch after /session new");
}

void test_context_usage() {
  Fixture fx;
  fx.config.max_tokens = 1000;
  fx.session.messages.push_back({"user", std::string(400, 'a'), now_iso(), "", ""});
  fx.session.messages.push_back({"assistant", std::string(400, 'b'), now_iso(), "", ""});
  auto ctx = fx.context();
  run_slash(ctx, "/context");
  std::ostringstream joined;
  for (const auto& line : fx.lines) {
    joined << line << "\n";
  }
  const auto txt = joined.str();
  expect(txt.find("messages=2") != std::string::npos, "context should show messages");
  expect(txt.find("tokens~=200") != std::string::npos, "context should estimate tokens");
  expect(txt.find("context_usage~=20% (200/1000)") != std::string::npos, "context should show usage percent");
}

void test_completion_vectors() {
  CompletionContext ctx{
      commands(),
      {"default", "demo", "feature-x"},
      {"api_key", "timeout_ms", "max_tokens", "safe_mode"},
      {"read_file", "write_file", "run_shell", "grep_text"},
      std::filesystem::current_path(),
      {"README.md", "src/main.ts", "src/repl.ts", "spec/config.schema.json", "tests/unit/repl.test.ts"}};

  expect(complete("/co", ctx) == std::vector<std::string>({"/compact", "/config", "/context"}), "prefix completion mismatch");
  expect(complete("/config ", ctx) == std::vector<std::string>({"get", "set", "list", "reset"}), "subcommand completion mismatch");
  expect(complete("/session switch d", ctx) == std::vector<std::string>({"default", "demo"}), "dynamic session completion mismatch");
  expect(complete("/read src/", ctx) == std::vector<std::string>({"src/main.ts", "src/repl.ts"}), "path completion mismatch");
}

void test_provider_sse_parser() {
  const auto chunks = GLMProvider::parse_sse_lines({
      "data: {\"choices\":[{\"delta\":{\"content\":\"Hel\"}}]}",
      "data: {\"choices\":[{\"delta\":{\"content\":\"lo\"}}]}",
      "data: [DONE]",
  });
  expect(chunks.size() == 2, "parser should return 2 chunks");
  expect(chunks[0].choices[0].delta_content + chunks[1].choices[0].delta_content == "Hello", "stream parse mismatch");
}

void test_agent_round() {
  RuntimeConfig cfg;
  cfg.stream = false;
  ToolCall tool_call{"c1", "list_dir", "{}"};
  ProviderReply planner{false, ChatResponse{{ChatChoice{"plan", "", {}}}}, {}};
  ProviderReply coder{false, ChatResponse{{ChatChoice{"need tool", "", {tool_call}}}}, {}};
  ProviderReply reviewer{false, ChatResponse{{ChatChoice{"review", "", {}}}}, {}};
  ProviderReply final_reply{false, ChatResponse{{ChatChoice{"final answer", "", {}}}}, {}};
  FakeProvider provider({planner, coder, reviewer, final_reply});

  ToolRegistry tools(std::filesystem::current_path(), cfg);
  std::vector<std::string> stages;
  std::vector<std::string> deltas;
  std::vector<std::string> tool_names;
  const auto result = run_multi_agent_round(
      "hi",
      {SessionMessage{"user", "start", now_iso(), "", ""}},
      provider,
      tools,
      cfg,
      AgentCallbacks{
          [&](const std::string& stage, const std::string&) { stages.push_back(stage); },
          [&](const std::string& name, const std::string&) { tool_names.push_back(name); },
          [&](const std::string& delta) { deltas.push_back(delta); }});

  expect(result.final == "final answer", "agent final mismatch");
  expect(stages == std::vector<std::string>({"planner", "coder", "reviewer", "orchestrator"}), "stage order mismatch");
  expect(tool_names == std::vector<std::string>({"list_dir"}), "tool callback mismatch");
  expect(deltas == std::vector<std::string>({"final answer"}), "delta callback mismatch");
}

void test_tui_append_only() {
  std::ostringstream out;
  TwoPaneTui tui(out, true);
  const PaneState pane{"default", "glm-5", "idle", "run_shell,read_file,list_dir", "strict", "none"};
  tui.start(pane);
  tui.announce_input("/help");
  tui.print_event("MiniCLI4 Help");
  tui.update_status(PaneState{"default", "glm-5", "planner", "", "strict", "none"});
  tui.print_status(PaneState{"default", "glm-5", "planner", "", "strict", "none"});
  tui.start_thinking();
  tui.tick_thinking();
  tui.stop_thinking();
  tui.start_assistant_stream();
  tui.append_assistant_chunk("Hello");
  tui.append_assistant_chunk(" world");
  tui.end_assistant_stream();

  const auto text = strip_ansi(out.str());
  expect(text.find("MiniCLI4 C++ Agent CLI") != std::string::npos, "missing tui header");
  expect(text.find("[YOU 01] /help") != std::string::npos, "missing input row");
  expect(text.find("MiniCLI4 Help") != std::string::npos, "missing event output");
  expect(text.find("[STATUS] session=default model=glm-5 stage=planner mode=strict approval=none") != std::string::npos, "missing status line");
  expect(text.find("Hello world") != std::string::npos, "missing stream content");
}

void test_cli_backend_check_command() {
  std::ostringstream captured;
  auto* old = std::cout.rdbuf(captured.rdbuf());
  const int code = run_cli({"backend"});
  std::cout.rdbuf(old);
  expect(code == 0, "backend command should return 0");
  const auto out = captured.str();
  expect(out.find("tui_backend=") != std::string::npos, "backend output should include active backend");
  expect(out.find("ftxui_available=") != std::string::npos, "backend output should include ftxui availability");
}

}  // namespace

int main() {
  try {
    test_commands_baseline();
    test_help_and_session();
    test_context_usage();
    test_completion_vectors();
    test_provider_sse_parser();
    test_agent_round();
    test_tui_append_only();
    test_cli_backend_check_command();
    std::cout << "all tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "test failure: " << ex.what() << "\n";
    return 1;
  }
}
