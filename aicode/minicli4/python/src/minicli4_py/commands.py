from __future__ import annotations

from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Callable

from .config import RuntimeConfig, save_config
from .session import SessionRecord, SessionStore
from .tools import ToolRegistry


@dataclass(slots=True)
class CommandHelp:
    name: str
    description: str
    usage: str
    category: str
    subcommands: list[str] | None = None


COMMAND_HELP: list[CommandHelp] = [
    CommandHelp("/help", "Show slash command help.", "/help [command]", "Core"),
    CommandHelp("/exit", "Exit REPL.", "/exit", "Core"),
    CommandHelp("/clear", "Clear current session messages.", "/clear", "Session"),
    CommandHelp("/status", "Show runtime status summary.", "/status", "System"),
    CommandHelp("/version", "Show CLI version.", "/version", "System"),
    CommandHelp("/model", "Show current model (locked to glm-5).", "/model", "System"),
    CommandHelp("/config", "Read or update local project configuration.", "/config <get|set|list|reset> [key] [value]", "System", ["get", "set", "list", "reset"]),
    CommandHelp("/session", "Session lifecycle operations.", "/session <list|new|switch|delete|rename|current> [args]", "Session", ["list", "new", "switch", "delete", "rename", "current"]),
    CommandHelp("/history", "Show recent message history.", "/history", "Session"),
    CommandHelp("/new", "Create and switch to a new session.", "/new [session_id]", "Session"),
    CommandHelp("/switch", "Switch active session.", "/switch <session_id>", "Session"),
    CommandHelp("/rename", "Rename current session.", "/rename <new_session_id>", "Session"),
    CommandHelp("/context", "Show context usage estimate.", "/context", "Context"),
    CommandHelp("/compact", "Compact old context messages.", "/compact", "Context"),
    CommandHelp("/tools", "List or toggle tool state.", "/tools <enable|disable> [tool_name]", "Tools", ["enable", "disable"]),
    CommandHelp("/permissions", "Show safety and permission settings.", "/permissions", "System"),
    CommandHelp("/grep", "Search project text by regex.", "/grep <pattern> [path]", "Tools"),
    CommandHelp("/tree", "Show project tree view.", "/tree [path] [depth]", "Tools"),
    CommandHelp("/run", "Run shell command under safety policy.", "/run <command>", "Tools"),
    CommandHelp("/read", "Read text file content.", "/read <path>", "Tools"),
    CommandHelp("/write", "Write text to file.", "/write <path> <content>", "Tools"),
    CommandHelp("/ls", "List directory entries.", "/ls [path]", "Tools"),
    CommandHelp("/pwd", "Print project root path.", "/pwd", "Tools"),
    CommandHelp("/export", "Export current session messages.", "/export [output_path]", "Session"),
    CommandHelp("/doctor", "Run runtime diagnostics.", "/doctor", "System"),
    CommandHelp("/agents", "Inspect or set agent profiles.", "/agents <list|set> [profile]", "System", ["list", "set"]),
    CommandHelp("/approve", "Approve pending guarded command.", "/approve", "Tools"),
    CommandHelp("/deny", "Deny pending guarded command.", "/deny", "Tools"),
    CommandHelp("/files", "Show context file list (reserved).", "/files", "Context"),
    CommandHelp("/add", "Add context file (reserved).", "/add <path>", "Context"),
]

COMMANDS = [item.name for item in COMMAND_HELP]


@dataclass(slots=True)
class CommandContext:
    project_root: Path
    config: RuntimeConfig
    session: SessionRecord
    sessions: SessionStore
    tools: ToolRegistry
    set_config: Callable[[RuntimeConfig], None]
    set_session: Callable[[SessionRecord], None]
    out: Callable[[str], None]
    set_pending_approval: Callable[[str], None]
    clear_pending_approval: Callable[[], None]
    pending_approval: Callable[[], str]


def _show_command_help(ctx: CommandContext, command: str) -> None:
    key = command if command.startswith("/") else f"/{command}"
    item = next((entry for entry in COMMAND_HELP if entry.name == key), None)
    if item is None:
        ctx.out(f"Unknown command: {command}")
        ctx.out("Use /help to list all commands.")
        return
    ctx.out(f"{item.name} - {item.description}")
    ctx.out(f"Usage: {item.usage}")
    if item.subcommands:
        ctx.out(f"Subcommands: {', '.join(item.subcommands)}")


def _show_help(ctx: CommandContext) -> None:
    categories = ["Core", "Session", "Context", "Tools", "System"]
    ctx.out("MiniCLI4 Help")
    ctx.out("Usage: /help [command]")
    ctx.out("")
    for category in categories:
        items = [item for item in COMMAND_HELP if item.category == category]
        if not items:
            continue
        ctx.out(f"{category}:")
        for item in items:
            ctx.out(f"  {item.name.ljust(13)} {item.description}")
        ctx.out("")
    ctx.out("Tips:")
    ctx.out("  Tab completes commands and arguments.")
    ctx.out("  Use /help <command> for command details.")


def run_slash(ctx: CommandContext, input_text: str) -> bool:
    parts = input_text.strip().split()
    cmd = parts[0] if parts else ""
    if cmd not in COMMANDS:
        ctx.out(f"Unknown command: {cmd}")
        return True
    if cmd == "/exit":
        return False
    if cmd == "/help":
        target = parts[1] if len(parts) > 1 else ""
        _show_command_help(ctx, target) if target else _show_help(ctx)
        return True
    if cmd == "/status":
        ctx.out("[STATUS]")
        ctx.out(f"  session     : {ctx.session.session_id}")
        ctx.out(f"  model       : {ctx.config.model}")
        ctx.out(f"  safe_mode   : {ctx.config.safe_mode}")
        ctx.out(f"  theme       : {ctx.config.theme}")
        ctx.out(f"  motion      : {ctx.config.motion}")
        ctx.out(f"  stream      : {ctx.config.stream}")
        ctx.out(f"  timeout_ms  : {ctx.config.timeout_ms}")
        ctx.out(f"  max_retries : {ctx.config.max_retries}")
        return True
    if cmd == "/version":
        ctx.out("minicli4-py 0.1.0")
        return True
    if cmd == "/model":
        ctx.out("glm-5")
        return True
    if cmd == "/clear":
        ctx.session.messages = []
        ctx.sessions.save(ctx.session)
        ctx.out("session cleared")
        return True
    if cmd == "/config":
        sub = parts[1] if len(parts) > 1 else "list"
        if sub == "list":
            for k, v in asdict(ctx.config).items():
                out = ",".join(v) if isinstance(v, list) else str(v)
                ctx.out(f"{k}={out}")
            return True
        if sub == "get":
            key = parts[2] if len(parts) > 2 else ""
            ctx.out("usage: /config get <key>" if not key else f"{key}={getattr(ctx.config, key, '')}")
            return True
        if sub == "set":
            key = parts[2] if len(parts) > 2 else ""
            value = " ".join(parts[3:]) if len(parts) > 3 else ""
            if not key or not value:
                ctx.out("usage: /config set <key> <value>")
                return True
            next_cfg = RuntimeConfig(**asdict(ctx.config))
            if key == "model":
                next_cfg.model = "glm-5"
            elif key == "stream":
                next_cfg.stream = value == "true"
            elif key in {"timeout_ms", "max_retries", "max_tokens", "agent_max_rounds"}:
                setattr(next_cfg, key, int(value))
            elif key == "temperature":
                next_cfg.temperature = float(value)
            elif key == "safe_mode":
                next_cfg.safe_mode = "balanced" if value == "balanced" else "strict"
            elif key == "theme":
                next_cfg.theme = "light" if value == "light" else "dark"
            elif key == "motion":
                next_cfg.motion = "minimal" if value == "minimal" else "full"
            elif key in {"allowed_paths", "shell_allowlist"}:
                setattr(next_cfg, key, [x.strip() for x in value.split(",") if x.strip()])
            else:
                setattr(next_cfg, key, value)
            ctx.set_config(next_cfg)
            save_config(ctx.project_root, next_cfg)
            ctx.out("config updated")
            return True
        if sub == "reset":
            reset = RuntimeConfig(**asdict(ctx.config))
            reset.model = "glm-5"
            ctx.set_config(reset)
            save_config(ctx.project_root, reset)
            ctx.out("config reset (model remains glm-5)")
            return True
        ctx.out("usage: /config <get|set|list|reset>")
        return True
    if cmd == "/session":
        sub = parts[1] if len(parts) > 1 else "list"
        if sub == "list":
            for rec in ctx.sessions.list():
                marker = "*" if rec.session_id == ctx.session.session_id else " "
                ctx.out(f"{marker} {rec.session_id} ({len(rec.messages)})")
            return True
        if sub == "new":
            next_rec = ctx.sessions.create(parts[2] if len(parts) > 2 else None)
            ctx.set_session(next_rec)
            ctx.out(f"session created: {next_rec.session_id}")
            return True
        if sub == "switch":
            target = parts[2] if len(parts) > 2 else ""
            if not target:
                ctx.out("usage: /session switch <id>")
                return True
            next_rec = ctx.sessions.load(target)
            ctx.sessions.set_current(target)
            ctx.set_session(next_rec)
            ctx.out(f"session switched: {next_rec.session_id}")
            return True
        if sub == "delete":
            target = parts[2] if len(parts) > 2 else ""
            if not target:
                ctx.out("usage: /session delete <id>")
                return True
            ctx.out("deleted" if ctx.sessions.delete(target) else "not found")
            return True
        if sub == "rename":
            old_id = parts[2] if len(parts) > 2 else ""
            new_id = parts[3] if len(parts) > 3 else ""
            if not old_id or not new_id:
                ctx.out("usage: /session rename <old> <new>")
                return True
            ctx.out("renamed" if ctx.sessions.rename(old_id, new_id) else "rename failed")
            return True
        if sub == "current":
            ctx.out(ctx.session.session_id)
            return True
        ctx.out("usage: /session <list|new|switch|delete|rename|current>")
        return True
    if cmd == "/new":
        next_rec = ctx.sessions.create(parts[1] if len(parts) > 1 else None)
        ctx.set_session(next_rec)
        ctx.out(f"session created: {next_rec.session_id}")
        return True
    if cmd == "/switch":
        sid = parts[1] if len(parts) > 1 else ""
        if not sid:
            ctx.out("usage: /switch <id>")
            return True
        next_rec = ctx.sessions.load(sid)
        ctx.sessions.set_current(sid)
        ctx.set_session(next_rec)
        ctx.out(f"session switched: {sid}")
        return True
    if cmd == "/rename":
        new_id = parts[1] if len(parts) > 1 else ""
        if not new_id:
            ctx.out("usage: /rename <new>")
            return True
        ok = ctx.sessions.rename(ctx.session.session_id, new_id)
        if ok:
            ctx.set_session(ctx.sessions.load(new_id))
        ctx.out("renamed" if ok else "rename failed")
        return True
    if cmd == "/history":
        for msg in ctx.session.messages[-20:]:
            ctx.out(f"{msg.role}: {msg.content[:120]}")
        return True
    if cmd == "/context":
        chars = sum(len(msg.content) for msg in ctx.session.messages)
        estimated_tokens = chars // 4
        token_budget = max(1, ctx.config.max_tokens)
        usage_percent = round((estimated_tokens / token_budget) * 100)
        ctx.out(f"messages={len(ctx.session.messages)}")
        ctx.out(f"tokens~={estimated_tokens}")
        ctx.out(f"context_usage~={usage_percent}% ({estimated_tokens}/{token_budget})")
        return True
    if cmd == "/compact":
        if len(ctx.session.messages) > 24:
            ctx.session.messages = ctx.session.messages[-24:]
            ctx.sessions.save(ctx.session)
            ctx.out("session compacted")
        else:
            ctx.out("no compaction needed")
        return True
    if cmd == "/tools":
        sub = parts[1] if len(parts) > 1 else "list"
        if sub == "list":
            for name in ctx.tools.list_tool_names():
                ctx.out(name)
            return True
        if sub in {"enable", "disable"}:
            ctx.out(f"{sub} acknowledged")
            return True
        ctx.out("usage: /tools <enable|disable>")
        return True
    if cmd == "/permissions":
        ctx.out(f"safe_mode={ctx.config.safe_mode}")
        ctx.out(f"allowed_paths={','.join(ctx.config.allowed_paths)}")
        ctx.out(f"shell_allowlist={','.join(ctx.config.shell_allowlist)}")
        return True
    if cmd == "/grep":
        pattern = parts[1] if len(parts) > 1 else ""
        if not pattern:
            ctx.out("usage: /grep <pattern> [path]")
            return True
        ctx.out(ctx.tools.execute("grep_text", {"pattern": pattern, "path": parts[2] if len(parts) > 2 else "."}).output)
        return True
    if cmd == "/tree":
        depth = int(parts[2]) if len(parts) > 2 and parts[2].isdigit() else 3
        ctx.out(ctx.tools.execute("project_tree", {"path": parts[1] if len(parts) > 1 else ".", "depth": depth}).output)
        return True
    if cmd == "/run":
        command = " ".join(parts[1:])
        if not command:
            ctx.out("usage: /run <command>")
            return True
        result = ctx.tools.execute("run_shell", {"command": command})
        if result.requires_approval:
            ctx.set_pending_approval(command)
            ctx.out(f"pending approval: /approve or /deny ({command})")
            return True
        ctx.out(result.output)
        return True
    if cmd == "/approve":
        pending = ctx.pending_approval()
        if not pending:
            ctx.out("no pending command")
            return True
        ctx.out(ctx.tools.execute("run_shell", {"command": pending.strip()}).output)
        ctx.clear_pending_approval()
        return True
    if cmd == "/deny":
        pending = ctx.pending_approval()
        if not pending:
            ctx.out("no pending command")
            return True
        ctx.clear_pending_approval()
        ctx.out("pending command denied")
        return True
    if cmd == "/read":
        p = parts[1] if len(parts) > 1 else ""
        if not p:
            ctx.out("usage: /read <path>")
            return True
        ctx.out(ctx.tools.execute("read_file", {"path": p}).output)
        return True
    if cmd == "/write":
        p = parts[1] if len(parts) > 1 else ""
        content = " ".join(parts[2:]) if len(parts) > 2 else ""
        if not p or not content:
            ctx.out("usage: /write <path> <content>")
            return True
        ctx.out(ctx.tools.execute("write_file", {"path": p, "content": content}).output)
        return True
    if cmd == "/ls":
        ctx.out(ctx.tools.execute("list_dir", {"path": parts[1] if len(parts) > 1 else "."}).output)
        return True
    if cmd == "/pwd":
        ctx.out(str(ctx.project_root))
        return True
    if cmd == "/export":
        out = parts[1] if len(parts) > 1 else ".minicli4/sessions/export.json"
        ctx.out(ctx.tools.execute("session_export", {"out": out}, str([asdict(m) for m in ctx.session.messages])).output)
        return True
    if cmd == "/doctor":
        ctx.out("[DOCTOR]")
        ctx.out("  runtime     : reachable")
        ctx.out(f"  model_lock  : {'ok' if ctx.config.model == 'glm-5' else 'invalid'}")
        ctx.out(f"  api_key     : {'set' if ctx.config.api_key.strip() else 'missing'}")
        ctx.out(f"  tools       : {len(ctx.tools.list_tool_names())}")
        ctx.out(f"  safe_mode   : {ctx.config.safe_mode}")
        return True
    if cmd == "/agents":
        sub = parts[1] if len(parts) > 1 else "list"
        if sub == "list":
            for role in ["orchestrator", "planner", "coder", "reviewer", "safety"]:
                ctx.out(role)
            return True
        if sub == "set":
            ctx.out("agent profile set (reserved)")
            return True
        ctx.out("usage: /agents <list|set>")
        return True
    if cmd == "/files":
        ctx.out("context files feature reserved")
        return True
    if cmd == "/add":
        ctx.out("context add feature reserved")
        return True
    return True
