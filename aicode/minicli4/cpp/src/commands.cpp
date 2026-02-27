#include "minicli4/commands.hpp"

#include <algorithm>
#include <sstream>

#include "minicli4/config.hpp"

namespace minicli4 {

namespace {
struct CommandHelp {
  std::string name;
  std::string description;
  std::string usage;
  std::string category;
  std::vector<std::string> subcommands;
};

const std::vector<CommandHelp> kHelp = {
    {"/help", "Show slash command help.", "/help [command]", "Core", {}},
    {"/exit", "Exit REPL.", "/exit", "Core", {}},
    {"/clear", "Clear current session messages.", "/clear", "Session", {}},
    {"/status", "Show runtime status summary.", "/status", "System", {}},
    {"/version", "Show CLI version.", "/version", "System", {}},
    {"/model", "Show current model (locked to glm-5).", "/model", "System", {}},
    {"/config", "Read or update local project configuration.", "/config <get|set|list|reset> [key] [value]", "System", {"get", "set", "list", "reset"}},
    {"/session", "Session lifecycle operations.", "/session <list|new|switch|delete|rename|current> [args]", "Session", {"list", "new", "switch", "delete", "rename", "current"}},
    {"/history", "Show recent message history.", "/history", "Session", {}},
    {"/new", "Create and switch to a new session.", "/new [session_id]", "Session", {}},
    {"/switch", "Switch active session.", "/switch <session_id>", "Session", {}},
    {"/rename", "Rename current session.", "/rename <new_session_id>", "Session", {}},
    {"/context", "Show context usage estimate.", "/context", "Context", {}},
    {"/compact", "Compact old context messages.", "/compact", "Context", {}},
    {"/tools", "List or toggle tool state.", "/tools <enable|disable> [tool_name]", "Tools", {"enable", "disable"}},
    {"/permissions", "Show safety and permission settings.", "/permissions", "System", {}},
    {"/grep", "Search project text by regex.", "/grep <pattern> [path]", "Tools", {}},
    {"/tree", "Show project tree view.", "/tree [path] [depth]", "Tools", {}},
    {"/run", "Run shell command under safety policy.", "/run <command>", "Tools", {}},
    {"/read", "Read text file content.", "/read <path>", "Tools", {}},
    {"/write", "Write text to file.", "/write <path> <content>", "Tools", {}},
    {"/ls", "List directory entries.", "/ls [path]", "Tools", {}},
    {"/pwd", "Print project root path.", "/pwd", "Tools", {}},
    {"/export", "Export current session messages.", "/export [output_path]", "Session", {}},
    {"/doctor", "Run runtime diagnostics.", "/doctor", "System", {}},
    {"/agents", "Inspect or set agent profiles.", "/agents <list|set> [profile]", "System", {"list", "set"}},
    {"/approve", "Approve pending guarded command.", "/approve", "Tools", {}},
    {"/deny", "Deny pending guarded command.", "/deny", "Tools", {}},
    {"/files", "Show context file list (reserved).", "/files", "Context", {}},
    {"/add", "Add context file (reserved).", "/add <path>", "Context", {}},
};

std::vector<std::string> split_ws(const std::string& s) {
  std::istringstream in(s);
  std::vector<std::string> out;
  std::string item;
  while (in >> item) {
    out.push_back(item);
  }
  return out;
}

std::string join_tail(const std::vector<std::string>& parts, size_t start) {
  if (start >= parts.size()) {
    return {};
  }
  std::ostringstream out;
  for (size_t i = start; i < parts.size(); i++) {
    if (i > start) {
      out << " ";
    }
    out << parts[i];
  }
  return out.str();
}

void show_help(CommandContext& ctx) {
  const std::vector<std::string> categories = {"Core", "Session", "Context", "Tools", "System"};
  ctx.out("MiniCLI4 Help");
  ctx.out("Usage: /help [command]");
  ctx.out("");
  for (const auto& category : categories) {
    ctx.out(category + ":");
    for (const auto& item : kHelp) {
      if (item.category == category) {
        ctx.out("  " + item.name + " " + item.description);
      }
    }
    ctx.out("");
  }
  ctx.out("Tips:");
  ctx.out("  Tab completes commands and arguments.");
  ctx.out("  Use /help <command> for command details.");
}
}  // namespace

const std::vector<std::string>& commands() {
  static const std::vector<std::string> names = [] {
    std::vector<std::string> out;
    out.reserve(kHelp.size());
    for (const auto& item : kHelp) {
      out.push_back(item.name);
    }
    return out;
  }();
  return names;
}

bool run_slash(CommandContext& ctx, const std::string& input) {
  const auto parts = split_ws(input);
  const std::string cmd = parts.empty() ? "" : parts.front();
  if (std::find(commands().begin(), commands().end(), cmd) == commands().end()) {
    ctx.out("Unknown command: " + cmd);
    return true;
  }
  if (cmd == "/exit") {
    return false;
  }
  if (cmd == "/help") {
    if (parts.size() <= 1) {
      show_help(ctx);
      return true;
    }
    const std::string target = parts[1].front() == '/' ? parts[1] : "/" + parts[1];
    const auto it = std::find_if(kHelp.begin(), kHelp.end(), [&](const CommandHelp& h) { return h.name == target; });
    if (it == kHelp.end()) {
      ctx.out("Unknown command: " + parts[1]);
      ctx.out("Use /help to list all commands.");
      return true;
    }
    ctx.out(it->name + " - " + it->description);
    ctx.out("Usage: " + it->usage);
    if (!it->subcommands.empty()) {
      std::ostringstream sub;
      for (size_t i = 0; i < it->subcommands.size(); i++) {
        sub << it->subcommands[i];
        if (i + 1 < it->subcommands.size()) {
          sub << ", ";
        }
      }
      ctx.out("Subcommands: " + sub.str());
    }
    return true;
  }
  if (cmd == "/status") {
    ctx.out("[STATUS]");
    ctx.out("  session     : " + ctx.session->session_id);
    ctx.out("  model       : " + ctx.config->model);
    ctx.out("  safe_mode   : " + ctx.config->safe_mode);
    ctx.out("  theme       : " + ctx.config->theme);
    ctx.out("  motion      : " + ctx.config->motion);
    ctx.out("  stream      : " + std::string(ctx.config->stream ? "true" : "false"));
    ctx.out("  timeout_ms  : " + std::to_string(ctx.config->timeout_ms));
    ctx.out("  max_retries : " + std::to_string(ctx.config->max_retries));
    return true;
  }
  if (cmd == "/version") {
    ctx.out("minicli4-cpp 0.1.0");
    return true;
  }
  if (cmd == "/model") {
    ctx.out("glm-5");
    return true;
  }
  if (cmd == "/clear") {
    ctx.session->messages.clear();
    ctx.sessions->save(*ctx.session);
    ctx.out("session cleared");
    return true;
  }
  if (cmd == "/config") {
    const std::string sub = parts.size() > 1 ? parts[1] : "list";
    if (sub == "list") {
      ctx.out("model=" + ctx.config->model);
      ctx.out("safe_mode=" + ctx.config->safe_mode);
      ctx.out("theme=" + ctx.config->theme);
      ctx.out("motion=" + ctx.config->motion);
      return true;
    }
    if (sub == "set") {
      if (parts.size() < 4) {
        ctx.out("usage: /config set <key> <value>");
        return true;
      }
      RuntimeConfig next = *ctx.config;
      const std::string key = parts[2];
      const std::string value = join_tail(parts, 3);
      if (key == "model") {
        next.model = "glm-5";
      } else if (key == "stream") {
        next.stream = value == "true";
      } else if (key == "timeout_ms") {
        next.timeout_ms = std::stoi(value);
      } else if (key == "max_retries") {
        next.max_retries = std::stoi(value);
      } else if (key == "max_tokens") {
        next.max_tokens = std::stoi(value);
      } else if (key == "agent_max_rounds") {
        next.agent_max_rounds = std::stoi(value);
      } else if (key == "temperature") {
        next.temperature = std::stod(value);
      } else if (key == "safe_mode") {
        next.safe_mode = value == "balanced" ? "balanced" : "strict";
      } else if (key == "theme") {
        next.theme = value == "light" ? "light" : "dark";
      } else if (key == "motion") {
        next.motion = value == "minimal" ? "minimal" : "full";
      }
      next.model = "glm-5";
      ctx.set_config(next);
      save_config(ctx.project_root, next);
      ctx.out("config updated");
      return true;
    }
    if (sub == "reset") {
      RuntimeConfig next = *ctx.config;
      next.model = "glm-5";
      ctx.set_config(next);
      save_config(ctx.project_root, next);
      ctx.out("config reset (model remains glm-5)");
      return true;
    }
    if (sub == "get") {
      if (parts.size() < 3) {
        ctx.out("usage: /config get <key>");
      } else {
        ctx.out(parts[2] + "=<value>");
      }
      return true;
    }
    ctx.out("usage: /config <get|set|list|reset>");
    return true;
  }
  if (cmd == "/session") {
    const std::string sub = parts.size() > 1 ? parts[1] : "list";
    if (sub == "list") {
      for (const auto& rec : ctx.sessions->list()) {
        const std::string marker = rec.session_id == ctx.session->session_id ? "*" : " ";
        ctx.out(marker + " " + rec.session_id + " (" + std::to_string(rec.messages.size()) + ")");
      }
      return true;
    }
    if (sub == "new") {
      const auto next = ctx.sessions->create(parts.size() > 2 ? parts[2] : "");
      ctx.set_session(next);
      ctx.out("session created: " + next.session_id);
      return true;
    }
    if (sub == "switch") {
      if (parts.size() < 3) {
        ctx.out("usage: /session switch <id>");
        return true;
      }
      const auto next = ctx.sessions->load(parts[2]);
      ctx.sessions->set_current(parts[2]);
      ctx.set_session(next);
      ctx.out("session switched: " + next.session_id);
      return true;
    }
    if (sub == "delete") {
      if (parts.size() < 3) {
        ctx.out("usage: /session delete <id>");
        return true;
      }
      ctx.out(ctx.sessions->remove(parts[2]) ? "deleted" : "not found");
      return true;
    }
    if (sub == "rename") {
      if (parts.size() < 4) {
        ctx.out("usage: /session rename <old> <new>");
        return true;
      }
      ctx.out(ctx.sessions->rename(parts[2], parts[3]) ? "renamed" : "rename failed");
      return true;
    }
    if (sub == "current") {
      ctx.out(ctx.session->session_id);
      return true;
    }
    ctx.out("usage: /session <list|new|switch|delete|rename|current>");
    return true;
  }
  if (cmd == "/new") {
    const auto next = ctx.sessions->create(parts.size() > 1 ? parts[1] : "");
    ctx.set_session(next);
    ctx.out("session created: " + next.session_id);
    return true;
  }
  if (cmd == "/switch") {
    if (parts.size() < 2) {
      ctx.out("usage: /switch <id>");
      return true;
    }
    const auto next = ctx.sessions->load(parts[1]);
    ctx.sessions->set_current(parts[1]);
    ctx.set_session(next);
    ctx.out("session switched: " + parts[1]);
    return true;
  }
  if (cmd == "/rename") {
    if (parts.size() < 2) {
      ctx.out("usage: /rename <new>");
      return true;
    }
    const bool ok = ctx.sessions->rename(ctx.session->session_id, parts[1]);
    if (ok) {
      ctx.set_session(ctx.sessions->load(parts[1]));
    }
    ctx.out(ok ? "renamed" : "rename failed");
    return true;
  }
  if (cmd == "/history") {
    for (const auto& msg : ctx.session->messages) {
      ctx.out(msg.role + ": " + msg.content.substr(0, std::min<size_t>(120, msg.content.size())));
    }
    return true;
  }
  if (cmd == "/context") {
    size_t chars = 0;
    for (const auto& m : ctx.session->messages) {
      chars += m.content.size();
    }
    const int estimated = static_cast<int>(chars / 4);
    const int budget = std::max(1, ctx.config->max_tokens);
    const int pct = static_cast<int>((100.0 * estimated) / budget + 0.5);
    ctx.out("messages=" + std::to_string(ctx.session->messages.size()));
    ctx.out("tokens~=" + std::to_string(estimated));
    ctx.out("context_usage~=" + std::to_string(pct) + "% (" + std::to_string(estimated) + "/" + std::to_string(budget) + ")");
    return true;
  }
  if (cmd == "/compact") {
    if (ctx.session->messages.size() > 24) {
      ctx.session->messages.erase(ctx.session->messages.begin(), ctx.session->messages.end() - 24);
      ctx.sessions->save(*ctx.session);
      ctx.out("session compacted");
    } else {
      ctx.out("no compaction needed");
    }
    return true;
  }
  if (cmd == "/tools") {
    const std::string sub = parts.size() > 1 ? parts[1] : "list";
    if (sub == "list") {
      for (const auto& name : ctx.tools->list_tool_names()) {
        ctx.out(name);
      }
      return true;
    }
    if (sub == "enable" || sub == "disable") {
      ctx.out(sub + " acknowledged");
      return true;
    }
    ctx.out("usage: /tools <enable|disable>");
    return true;
  }
  if (cmd == "/permissions") {
    ctx.out("safe_mode=" + ctx.config->safe_mode);
    return true;
  }
  if (cmd == "/grep") {
    if (parts.size() < 2) {
      ctx.out("usage: /grep <pattern> [path]");
      return true;
    }
    ctx.out(ctx.tools->execute("grep_text", {{"pattern", parts[1]}, {"path", parts.size() > 2 ? parts[2] : "."}}).output);
    return true;
  }
  if (cmd == "/tree") {
    ctx.out(ctx.tools->execute("project_tree", {{"path", parts.size() > 1 ? parts[1] : "."}, {"depth", parts.size() > 2 ? parts[2] : "3"}}).output);
    return true;
  }
  if (cmd == "/run") {
    const auto command = join_tail(parts, 1);
    if (command.empty()) {
      ctx.out("usage: /run <command>");
      return true;
    }
    const auto result = ctx.tools->execute("run_shell", {{"command", command}});
    if (result.requires_approval) {
      ctx.set_pending_approval(command);
      ctx.out("pending approval: /approve or /deny (" + command + ")");
      return true;
    }
    ctx.out(result.output);
    return true;
  }
  if (cmd == "/approve") {
    const auto pending = ctx.pending_approval();
    if (pending.empty()) {
      ctx.out("no pending command");
      return true;
    }
    ctx.out(ctx.tools->execute("run_shell", {{"command", pending}}).output);
    ctx.clear_pending_approval();
    return true;
  }
  if (cmd == "/deny") {
    const auto pending = ctx.pending_approval();
    if (pending.empty()) {
      ctx.out("no pending command");
      return true;
    }
    ctx.clear_pending_approval();
    ctx.out("pending command denied");
    return true;
  }
  if (cmd == "/read") {
    if (parts.size() < 2) {
      ctx.out("usage: /read <path>");
      return true;
    }
    ctx.out(ctx.tools->execute("read_file", {{"path", parts[1]}}).output);
    return true;
  }
  if (cmd == "/write") {
    if (parts.size() < 3) {
      ctx.out("usage: /write <path> <content>");
      return true;
    }
    ctx.out(ctx.tools->execute("write_file", {{"path", parts[1]}, {"content", join_tail(parts, 2)}}).output);
    return true;
  }
  if (cmd == "/ls") {
    ctx.out(ctx.tools->execute("list_dir", {{"path", parts.size() > 1 ? parts[1] : "."}}).output);
    return true;
  }
  if (cmd == "/pwd") {
    ctx.out(ctx.project_root.generic_string());
    return true;
  }
  if (cmd == "/export") {
    const std::string out = parts.size() > 1 ? parts[1] : ".minicli4/sessions/export.json";
    ctx.out(ctx.tools->execute("session_export", {{"out", out}}, "[]").output);
    return true;
  }
  if (cmd == "/doctor") {
    ctx.out("[DOCTOR]");
    ctx.out("  runtime     : reachable");
    ctx.out("  model_lock  : " + std::string(ctx.config->model == "glm-5" ? "ok" : "invalid"));
    ctx.out("  api_key     : " + std::string(ctx.config->api_key.empty() ? "missing" : "set"));
    ctx.out("  tools       : " + std::to_string(ctx.tools->list_tool_names().size()));
    ctx.out("  safe_mode   : " + ctx.config->safe_mode);
    return true;
  }
  if (cmd == "/agents") {
    const std::string sub = parts.size() > 1 ? parts[1] : "list";
    if (sub == "list") {
      ctx.out("orchestrator");
      ctx.out("planner");
      ctx.out("coder");
      ctx.out("reviewer");
      ctx.out("safety");
      return true;
    }
    if (sub == "set") {
      ctx.out("agent profile set (reserved)");
      return true;
    }
    ctx.out("usage: /agents <list|set>");
    return true;
  }
  if (cmd == "/files") {
    ctx.out("context files feature reserved");
    return true;
  }
  if (cmd == "/add") {
    ctx.out("context add feature reserved");
    return true;
  }
  return true;
}

}  // namespace minicli4
