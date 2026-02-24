"""Built-in slash commands for MiniCLI3."""

from __future__ import annotations

from rich.table import Table

from minicli3.commands.registry import CommandRegistry, CommandSpec


def build_registry() -> CommandRegistry:
    reg = CommandRegistry()

    def cmd_exit(app, _args):
        app.session_manager.save()
        app.console.print("[green]Bye[/green]")
        return False

    def cmd_help(app, args):
        if len(args) > 1:
            target = app.commands.get(args[1])
            if target is None:
                app.console.print(f"[yellow]Unknown command: {args[1]}[/yellow]")
                return None
            app.console.print(f"[cyan]{target.name}[/cyan] - {target.description}")
            app.console.print(f"Usage: {target.usage}")
            if target.subcommands:
                app.console.print(f"Subcommands: {', '.join(target.subcommands)}")
            return None

        table = Table(title="Slash Commands")
        table.add_column("Command", style="cyan")
        table.add_column("Description")
        for spec in app.commands.list():
            table.add_row(spec.name, spec.description)
        app.console.print(table)
        return None

    def cmd_clear(app, _args):
        app.session.clear()
        app.session_manager.save()
        app.console.print("[green]Session cleared.[/green]")

    def cmd_status(app, _args):
        app.console.print(f"session: [cyan]{app.session.session_id}[/cyan]")
        app.console.print(f"model: [cyan]{app.config.model}[/cyan]")
        app.console.print(f"tools: [cyan]{', '.join(app.tools.enabled_tools())}[/cyan]")

    def cmd_version(app, _args):
        app.console.print("minicli3 0.1.0")

    def cmd_echo(app, args):
        app.console.print(" ".join(args[1:]))

    def cmd_session(app, args):
        if len(args) == 1 or args[1] == "list":
            rows = app.session_manager.list()
            table = Table(title="Sessions")
            table.add_column("ID", style="cyan")
            table.add_column("Messages")
            table.add_column("Updated")
            for row in rows:
                marker = " *" if row["session_id"] == app.session.session_id else ""
                table.add_row(row["session_id"] + marker, str(row["message_count"]), row["updated_at"][:19])
            app.console.print(table)
            return None

        sub = args[1]
        if sub == "new":
            sid = args[2] if len(args) > 2 else None
            app.session = app.session_manager.create(sid)
            app.console.print(f"[green]Created session: {app.session.session_id}[/green]")
        elif sub == "switch" and len(args) > 2:
            app.session = app.session_manager.set_current(args[2])
            app.console.print(f"[green]Switched session: {app.session.session_id}[/green]")
        elif sub == "delete" and len(args) > 2:
            ok = app.session_manager.delete(args[2])
            app.console.print("[green]Deleted.[/green]" if ok else "[yellow]Session not found.[/yellow]")
        elif sub == "rename" and len(args) > 3:
            ok = app.session_manager.rename(args[2], args[3])
            app.console.print("[green]Renamed.[/green]" if ok else "[yellow]Rename failed.[/yellow]")
        elif sub == "current":
            app.console.print(f"[cyan]{app.session.session_id}[/cyan]")
        else:
            app.console.print("[yellow]Usage: /session [list|new|switch|delete|rename|current][/yellow]")

    def cmd_sessions(app, _args):
        return cmd_session(app, ["/session", "list"])

    def cmd_new(app, args):
        session_id = args[1] if len(args) > 1 else None
        tail = ["new"] + ([session_id] if session_id else [])
        return cmd_session(app, ["/session", *tail])

    def cmd_switch(app, args):
        if len(args) < 2:
            app.console.print("[yellow]Usage: /switch <session_id>[/yellow]")
            return None
        return cmd_session(app, ["/session", "switch", args[1]])

    def cmd_delete(app, args):
        if len(args) < 2:
            app.console.print("[yellow]Usage: /delete <session_id>[/yellow]")
            return None
        return cmd_session(app, ["/session", "delete", args[1]])

    def cmd_rename(app, args):
        if len(args) < 3:
            app.console.print("[yellow]Usage: /rename <old_id> <new_id>[/yellow]")
            return None
        return cmd_session(app, ["/session", "rename", args[1], args[2]])

    def cmd_current(app, _args):
        return cmd_session(app, ["/session", "current"])

    def cmd_history(app, args):
        limit = 20
        if len(args) > 2 and args[1] == "--limit":
            limit = int(args[2])
        for msg in app.session.messages[-limit:]:
            app.console.print(f"[bold]{msg.role}[/bold]: {msg.content[:200]}")

    def cmd_context(app, _args):
        estimated = sum(len(m.content) // 4 for m in app.session.messages)
        app.console.print(f"messages={len(app.session.messages)}")
        app.console.print(f"tokens~={estimated}")
        app.console.print("limit=128000")

    def cmd_compact(app, _args):
        if len(app.session.messages) <= 20:
            app.console.print("[dim]No compaction needed.[/dim]")
            return None
        kept = app.session.messages[-20:]
        app.session.messages = kept
        app.session.add_message("system", "Conversation compacted to keep recent context.")
        app.session_manager.save()
        app.console.print("[green]Compacted session context.[/green]")

    def cmd_model(app, _args):
        app.console.print("glm-5")

    def cmd_config(app, args):
        if len(args) == 1 or args[1] == "list":
            for k, v in app.config.__dict__.items():
                app.console.print(f"{k}={v}")
            return None
        sub = args[1]
        if sub == "get" and len(args) > 2:
            value = app.config_manager.get(args[2])
            app.console.print(f"{args[2]}={value}")
        elif sub == "set" and len(args) > 3:
            app.config_manager.set(args[2], args[3])
            app.config = app.config_manager.config
            app.tools.config = app.config
            app.console.print("[green]Config updated.[/green]")
        elif sub == "reset":
            app.config_manager.reset()
            app.config = app.config_manager.config
            app.console.print("[green]Config reset.[/green]")
        elif sub == "path":
            app.console.print(str(app.config_manager.config_path))
        else:
            app.console.print("[yellow]Usage: /config [list|get|set|reset|path][/yellow]")

    def cmd_tools(app, args):
        if len(args) == 1:
            rows = app.tools.list_tools()
            table = Table(title="Tools")
            table.add_column("Tool", style="cyan")
            table.add_column("Enabled")
            table.add_column("Description")
            for row in rows:
                table.add_row(row["name"], "yes" if row["enabled"] else "no", row["description"])
            app.console.print(table)
            return None

        sub = args[1]
        if sub == "enable" and len(args) > 2:
            app.console.print("[green]Enabled.[/green]" if app.tools.enable(args[2]) else "[yellow]Unknown tool.[/yellow]")
        elif sub == "disable" and len(args) > 2:
            app.console.print("[green]Disabled.[/green]" if app.tools.disable(args[2]) else "[yellow]Unknown tool.[/yellow]")
        else:
            app.console.print("[yellow]Usage: /tools [enable|disable <name>][/yellow]")

    def cmd_permissions(app, _args):
        app.console.print(f"safe_mode={app.config.safe_mode}")
        app.console.print(f"allowed_paths={app.config.allowed_paths}")
        app.console.print(f"shell_allowlist={app.config.shell_allowlist}")

    def cmd_grep(app, args):
        if len(args) < 2:
            app.console.print("[yellow]Usage: /grep <pattern> [path] [glob][/yellow]")
            return None
        pattern = args[1]
        path = args[2] if len(args) > 2 else "."
        glob = args[3] if len(args) > 3 else "*"
        app.console.print(app.tools.execute("grep", {"pattern": pattern, "path": path, "file_glob": glob}))

    def cmd_pwd(app, _args):
        app.console.print(str(app.project_root))

    def cmd_ls(app, args):
        path = args[1] if len(args) > 1 else "."
        app.console.print(app.tools.execute("list_dir", {"path": path}))

    def cmd_read(app, args):
        if len(args) < 2:
            app.console.print("[yellow]Usage: /read <path>[/yellow]")
            return None
        app.console.print(app.tools.execute("read", {"path": args[1]}))

    def cmd_write(app, args):
        if len(args) < 3:
            app.console.print("[yellow]Usage: /write <path> <content>[/yellow]")
            return None
        app.console.print(app.tools.execute("write", {"path": args[1], "content": " ".join(args[2:])}))

    def cmd_run(app, args):
        if len(args) < 2:
            app.console.print("[yellow]Usage: /run <command>[/yellow]")
            return None
        app.console.print(app.tools.execute("run_shell", {"command": " ".join(args[1:])}))

    commands: list[CommandSpec] = [
        CommandSpec("/exit", "Exit REPL", "/exit", cmd_exit),
        CommandSpec("/help", "Show help", "/help [command]", cmd_help),
        CommandSpec("/clear", "Clear current session", "/clear", cmd_clear),
        CommandSpec("/status", "Show runtime status", "/status", cmd_status),
        CommandSpec("/version", "Show version", "/version", cmd_version),
        CommandSpec("/echo", "Echo text", "/echo <text>", cmd_echo),
        CommandSpec("/session", "Session management", "/session [list|new|switch|delete|rename|current]", cmd_session, ["list", "new", "switch", "delete", "rename", "current"]),
        CommandSpec("/sessions", "List sessions", "/sessions", cmd_sessions),
        CommandSpec("/new", "Create and switch session", "/new [session_id]", cmd_new),
        CommandSpec("/switch", "Switch current session", "/switch <session_id>", cmd_switch),
        CommandSpec("/delete", "Delete session", "/delete <session_id>", cmd_delete),
        CommandSpec("/rename", "Rename session", "/rename <old_id> <new_id>", cmd_rename),
        CommandSpec("/current", "Show current session id", "/current", cmd_current),
        CommandSpec("/history", "Show message history", "/history [--limit N]", cmd_history),
        CommandSpec("/context", "Show context usage", "/context", cmd_context),
        CommandSpec("/compact", "Compact old context", "/compact", cmd_compact),
        CommandSpec("/model", "Show model (locked)", "/model", cmd_model),
        CommandSpec("/config", "Config management", "/config [list|get|set|reset|path]", cmd_config, ["list", "get", "set", "reset", "path"]),
        CommandSpec("/tools", "List/enable/disable tools", "/tools [enable|disable <name>]", cmd_tools, ["enable", "disable"]),
        CommandSpec("/permissions", "Show safety policy", "/permissions", cmd_permissions),
        CommandSpec("/grep", "Search text by regex", "/grep <pattern> [path] [glob]", cmd_grep),
        CommandSpec("/pwd", "Show project root path", "/pwd", cmd_pwd),
        CommandSpec("/ls", "List directory entries", "/ls [path]", cmd_ls),
        CommandSpec("/read", "Read file content", "/read <path>", cmd_read),
        CommandSpec("/write", "Write file content", "/write <path> <content>", cmd_write),
        CommandSpec("/run", "Run shell command via allowlist", "/run <command>", cmd_run),
    ]
    for spec in commands:
        reg.register(spec)
    return reg
