import { RuntimeConfig, saveConfig } from "./config";
import { SessionRecord, SessionStore } from "./session";
import { ToolRegistry } from "./tools";

export interface CommandContext {
  projectRoot: string;
  config: RuntimeConfig;
  session: SessionRecord;
  sessions: SessionStore;
  tools: ToolRegistry;
  setConfig: (next: RuntimeConfig) => void;
  setSession: (next: SessionRecord) => void;
  out: (line: string) => void;
  setPendingApproval: (value: string) => void;
  clearPendingApproval: () => void;
  pendingApproval: () => string;
}

interface CommandHelp {
  name: string;
  description: string;
  usage: string;
  category: "Core" | "Session" | "Context" | "Tools" | "System";
  subcommands?: string[];
}

const COMMAND_HELP: CommandHelp[] = [
  { name: "/help", description: "Show slash command help.", usage: "/help [command]", category: "Core" },
  { name: "/exit", description: "Exit REPL.", usage: "/exit", category: "Core" },
  { name: "/clear", description: "Clear current session messages.", usage: "/clear", category: "Session" },
  { name: "/status", description: "Show runtime status summary.", usage: "/status", category: "System" },
  { name: "/version", description: "Show CLI version.", usage: "/version", category: "System" },
  { name: "/model", description: "Show current model (locked to glm-5).", usage: "/model", category: "System" },
  {
    name: "/config",
    description: "Read or update local project configuration.",
    usage: "/config <get|set|list|reset> [key] [value]",
    category: "System",
    subcommands: ["get", "set", "list", "reset"]
  },
  {
    name: "/session",
    description: "Session lifecycle operations.",
    usage: "/session <list|new|switch|delete|rename|current> [args]",
    category: "Session",
    subcommands: ["list", "new", "switch", "delete", "rename", "current"]
  },
  { name: "/history", description: "Show recent message history.", usage: "/history", category: "Session" },
  { name: "/new", description: "Create and switch to a new session.", usage: "/new [session_id]", category: "Session" },
  { name: "/switch", description: "Switch active session.", usage: "/switch <session_id>", category: "Session" },
  { name: "/rename", description: "Rename current session.", usage: "/rename <new_session_id>", category: "Session" },
  { name: "/context", description: "Show context usage estimate.", usage: "/context", category: "Context" },
  { name: "/compact", description: "Compact old context messages.", usage: "/compact", category: "Context" },
  {
    name: "/tools",
    description: "List or toggle tool state.",
    usage: "/tools <enable|disable> [tool_name]",
    category: "Tools",
    subcommands: ["enable", "disable"]
  },
  { name: "/permissions", description: "Show safety and permission settings.", usage: "/permissions", category: "System" },
  { name: "/grep", description: "Search project text by regex.", usage: "/grep <pattern> [path]", category: "Tools" },
  { name: "/tree", description: "Show project tree view.", usage: "/tree [path] [depth]", category: "Tools" },
  { name: "/run", description: "Run shell command under safety policy.", usage: "/run <command>", category: "Tools" },
  { name: "/read", description: "Read text file content.", usage: "/read <path>", category: "Tools" },
  { name: "/write", description: "Write text to file.", usage: "/write <path> <content>", category: "Tools" },
  { name: "/ls", description: "List directory entries.", usage: "/ls [path]", category: "Tools" },
  { name: "/pwd", description: "Print project root path.", usage: "/pwd", category: "Tools" },
  { name: "/export", description: "Export current session messages.", usage: "/export [output_path]", category: "Session" },
  { name: "/doctor", description: "Run runtime diagnostics.", usage: "/doctor", category: "System" },
  {
    name: "/agents",
    description: "Inspect or set agent profiles.",
    usage: "/agents <list|set> [profile]",
    category: "System",
    subcommands: ["list", "set"]
  },
  { name: "/approve", description: "Approve pending guarded command.", usage: "/approve", category: "Tools" },
  { name: "/deny", description: "Deny pending guarded command.", usage: "/deny", category: "Tools" },
  { name: "/files", description: "Show context file list (reserved).", usage: "/files", category: "Context" },
  { name: "/add", description: "Add context file (reserved).", usage: "/add <path>", category: "Context" }
];

export const COMMANDS: string[] = COMMAND_HELP.map((item) => item.name);

function showCommandHelp(ctx: CommandContext, command: string): void {
  const key = command.startsWith("/") ? command : `/${command}`;
  const item = COMMAND_HELP.find((entry) => entry.name === key);
  if (!item) {
    ctx.out(`Unknown command: ${command}`);
    ctx.out("Use /help to list all commands.");
    return;
  }

  ctx.out(`${item.name} - ${item.description}`);
  ctx.out(`Usage: ${item.usage}`);
  if (item.subcommands && item.subcommands.length > 0) {
    ctx.out(`Subcommands: ${item.subcommands.join(", ")}`);
  }
}

function showHelp(ctx: CommandContext): void {
  const categories: Array<CommandHelp["category"]> = ["Core", "Session", "Context", "Tools", "System"];
  ctx.out("MiniCLI4 Help");
  ctx.out("Usage: /help [command]");
  ctx.out("");

  for (const category of categories) {
    const items = COMMAND_HELP.filter((item) => item.category === category);
    if (items.length === 0) {
      continue;
    }
    ctx.out(`${category}:`);
    for (const item of items) {
      ctx.out(`  ${item.name.padEnd(13)} ${item.description}`);
    }
    ctx.out("");
  }

  ctx.out("Tips:");
  ctx.out("  Tab completes commands and arguments.");
  ctx.out("  Use /help <command> for command details.");
}

export function runSlash(ctx: CommandContext, input: string): boolean {
  const parts = input.trim().split(/\s+/);
  const cmd = parts[0] ?? "";

  if (!COMMANDS.includes(cmd)) {
    ctx.out(`Unknown command: ${cmd}`);
    return true;
  }

  if (cmd === "/exit") {
    return false;
  }

  if (cmd === "/help") {
    const target = parts[1];
    if (target) {
      showCommandHelp(ctx, target);
    } else {
      showHelp(ctx);
    }
    return true;
  }

  if (cmd === "/status") {
    ctx.out(`session=${ctx.session.session_id}`);
    ctx.out(`model=${ctx.config.model}`);
    ctx.out(`safe_mode=${ctx.config.safe_mode}`);
    return true;
  }

  if (cmd === "/version") {
    ctx.out("minicli4-ts 0.1.0");
    return true;
  }

  if (cmd === "/model") {
    ctx.out("glm-5");
    return true;
  }

  if (cmd === "/clear") {
    ctx.session.messages = [];
    ctx.sessions.save(ctx.session);
    ctx.out("session cleared");
    return true;
  }

  if (cmd === "/config") {
    const sub = parts[1] ?? "list";
    if (sub === "list") {
      for (const [k, v] of Object.entries(ctx.config)) {
        ctx.out(`${k}=${Array.isArray(v) ? v.join(",") : String(v)}`);
      }
      return true;
    }
    if (sub === "get") {
      const key = parts[2] as keyof RuntimeConfig;
      if (!key) {
        ctx.out("usage: /config get <key>");
      } else {
        ctx.out(`${String(key)}=${String(ctx.config[key])}`);
      }
      return true;
    }
    if (sub === "set") {
      const key = parts[2] as keyof RuntimeConfig;
      const value = parts.slice(3).join(" ");
      if (!key || value.length === 0) {
        ctx.out("usage: /config set <key> <value>");
        return true;
      }
      const next = { ...ctx.config };
      if (key === "model") {
        next.model = "glm-5";
      } else if (key === "stream") {
        next.stream = value === "true";
      } else if (key === "timeout_ms" || key === "max_retries" || key === "max_tokens" || key === "agent_max_rounds") {
        (next as Record<string, unknown>)[key] = Number(value);
      } else if (key === "temperature") {
        next.temperature = Number(value);
      } else if (key === "safe_mode") {
        next.safe_mode = value === "balanced" ? "balanced" : "strict";
      } else if (key === "allowed_paths" || key === "shell_allowlist") {
        (next as Record<string, unknown>)[key] = value.split(",").map((x) => x.trim()).filter(Boolean);
      } else {
        (next as Record<string, unknown>)[key] = value;
      }
      ctx.setConfig(next);
      saveConfig(ctx.projectRoot, next);
      ctx.out("config updated");
      return true;
    }
    if (sub === "reset") {
      const reset = {
        ...ctx.config,
        model: "glm-5" as const
      };
      ctx.setConfig(reset);
      saveConfig(ctx.projectRoot, reset);
      ctx.out("config reset (model remains glm-5)");
      return true;
    }
    ctx.out("usage: /config <get|set|list|reset>");
    return true;
  }

  if (cmd === "/session") {
    const sub = parts[1] ?? "list";
    if (sub === "list") {
      for (const rec of ctx.sessions.list()) {
        const marker = rec.session_id === ctx.session.session_id ? "*" : " ";
        ctx.out(`${marker} ${rec.session_id} (${rec.messages.length})`);
      }
      return true;
    }
    if (sub === "new") {
      const next = ctx.sessions.create(parts[2]);
      ctx.setSession(next);
      ctx.out(`session created: ${next.session_id}`);
      return true;
    }
    if (sub === "switch") {
      const target = parts[2];
      if (!target) {
        ctx.out("usage: /session switch <id>");
        return true;
      }
      const next = ctx.sessions.load(target);
      ctx.sessions.setCurrent(target);
      ctx.setSession(next);
      ctx.out(`session switched: ${next.session_id}`);
      return true;
    }
    if (sub === "delete") {
      const target = parts[2];
      if (!target) {
        ctx.out("usage: /session delete <id>");
        return true;
      }
      ctx.out(ctx.sessions.delete(target) ? "deleted" : "not found");
      return true;
    }
    if (sub === "rename") {
      const oldId = parts[2];
      const newId = parts[3];
      if (!oldId || !newId) {
        ctx.out("usage: /session rename <old> <new>");
        return true;
      }
      ctx.out(ctx.sessions.rename(oldId, newId) ? "renamed" : "rename failed");
      return true;
    }
    if (sub === "current") {
      ctx.out(ctx.session.session_id);
      return true;
    }
    ctx.out("usage: /session <list|new|switch|delete|rename|current>");
    return true;
  }

  if (cmd === "/new") {
    const next = ctx.sessions.create(parts[1]);
    ctx.setSession(next);
    ctx.out(`session created: ${next.session_id}`);
    return true;
  }

  if (cmd === "/switch") {
    const id = parts[1];
    if (!id) {
      ctx.out("usage: /switch <id>");
      return true;
    }
    const next = ctx.sessions.load(id);
    ctx.sessions.setCurrent(id);
    ctx.setSession(next);
    ctx.out(`session switched: ${id}`);
    return true;
  }

  if (cmd === "/rename") {
    const newId = parts[1];
    if (!newId) {
      ctx.out("usage: /rename <new>");
      return true;
    }
    const ok = ctx.sessions.rename(ctx.session.session_id, newId);
    if (ok) {
      const next = ctx.sessions.load(newId);
      ctx.setSession(next);
    }
    ctx.out(ok ? "renamed" : "rename failed");
    return true;
  }

  if (cmd === "/history") {
    for (const msg of ctx.session.messages.slice(-20)) {
      ctx.out(`${msg.role}: ${msg.content.slice(0, 120)}`);
    }
    return true;
  }

  if (cmd === "/context") {
    const chars = ctx.session.messages.reduce((n, m) => n + m.content.length, 0);
    ctx.out(`messages=${ctx.session.messages.length}`);
    ctx.out(`tokens~=${Math.floor(chars / 4)}`);
    return true;
  }

  if (cmd === "/compact") {
    if (ctx.session.messages.length > 24) {
      ctx.session.messages = ctx.session.messages.slice(-24);
      ctx.sessions.save(ctx.session);
      ctx.out("session compacted");
    } else {
      ctx.out("no compaction needed");
    }
    return true;
  }

  if (cmd === "/tools") {
    const sub = parts[1] ?? "list";
    if (sub === "list") {
      for (const n of ctx.tools.listToolNames()) {
        ctx.out(n);
      }
      return true;
    }
    if (sub === "enable" || sub === "disable") {
      ctx.out(`${sub} acknowledged`);
      return true;
    }
    ctx.out("usage: /tools <enable|disable>");
    return true;
  }

  if (cmd === "/permissions") {
    ctx.out(`safe_mode=${ctx.config.safe_mode}`);
    ctx.out(`allowed_paths=${ctx.config.allowed_paths.join(",")}`);
    ctx.out(`shell_allowlist=${ctx.config.shell_allowlist.join(",")}`);
    return true;
  }

  if (cmd === "/grep") {
    const pattern = parts[1];
    if (!pattern) {
      ctx.out("usage: /grep <pattern> [path]");
      return true;
    }
    const result = ctx.tools.execute("grep_text", { pattern, path: parts[2] ?? "." });
    ctx.out(result.output);
    return true;
  }

  if (cmd === "/tree") {
    const result = ctx.tools.execute("project_tree", { path: parts[1] ?? ".", depth: Number(parts[2] ?? 3) });
    ctx.out(result.output);
    return true;
  }

  if (cmd === "/run") {
    const command = parts.slice(1).join(" ");
    if (!command) {
      ctx.out("usage: /run <command>");
      return true;
    }
    const result = ctx.tools.execute("run_shell", { command });
    if (result.requiresApproval) {
      ctx.setPendingApproval(command);
      ctx.out(`pending approval: /approve or /deny (${command})`);
      return true;
    }
    ctx.out(result.output);
    return true;
  }

  if (cmd === "/approve") {
    const pending = ctx.pendingApproval();
    if (!pending) {
      ctx.out("no pending command");
      return true;
    }
    const result = ctx.tools.execute("run_shell", { command: pending.replace(/^\s+|\s+$/g, "") });
    ctx.out(result.output);
    ctx.clearPendingApproval();
    return true;
  }

  if (cmd === "/deny") {
    const pending = ctx.pendingApproval();
    if (!pending) {
      ctx.out("no pending command");
      return true;
    }
    ctx.clearPendingApproval();
    ctx.out("pending command denied");
    return true;
  }

  if (cmd === "/read") {
    const p = parts[1];
    if (!p) {
      ctx.out("usage: /read <path>");
      return true;
    }
    ctx.out(ctx.tools.execute("read_file", { path: p }).output);
    return true;
  }

  if (cmd === "/write") {
    const p = parts[1];
    const content = parts.slice(2).join(" ");
    if (!p || !content) {
      ctx.out("usage: /write <path> <content>");
      return true;
    }
    ctx.out(ctx.tools.execute("write_file", { path: p, content }).output);
    return true;
  }

  if (cmd === "/ls") {
    ctx.out(ctx.tools.execute("list_dir", { path: parts[1] ?? "." }).output);
    return true;
  }

  if (cmd === "/pwd") {
    ctx.out(ctx.projectRoot);
    return true;
  }

  if (cmd === "/export") {
    const out = parts[1] ?? ".minicli4/sessions/export.json";
    ctx.out(ctx.tools.execute("session_export", { out }, JSON.stringify(ctx.session.messages, null, 2)).output);
    return true;
  }

  if (cmd === "/doctor") {
    ctx.out("doctor: runtime reachable");
    return true;
  }

  if (cmd === "/agents") {
    const sub = parts[1] ?? "list";
    if (sub === "list") {
      ctx.out("orchestrator");
      ctx.out("planner");
      ctx.out("coder");
      ctx.out("reviewer");
      ctx.out("safety");
      return true;
    }
    if (sub === "set") {
      ctx.out("agent profile set (reserved)");
      return true;
    }
    ctx.out("usage: /agents <list|set>");
    return true;
  }

  if (cmd === "/files") {
    ctx.out("context files feature reserved");
    return true;
  }

  if (cmd === "/add") {
    ctx.out("context add feature reserved");
    return true;
  }

  return true;
}
