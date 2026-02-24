"""Completion candidate generation for slash commands."""

from __future__ import annotations

from pathlib import Path

from minicli3.commands.registry import CommandRegistry


def _split_with_trailing(text: str) -> tuple[list[str], bool]:
    trailing_space = text.endswith(" ")
    parts = text.strip().split() if text.strip() else []
    return parts, trailing_space


def command_completions(
    text: str,
    registry: CommandRegistry,
    *,
    session_ids: list[str],
    config_keys: list[str],
    tool_names: list[str],
    project_root: Path,
) -> list[str]:
    if not text.startswith("/"):
        return []

    parts, trailing = _split_with_trailing(text)
    if not parts:
        return registry.names()

    if len(parts) == 1 and not trailing:
        return [name for name in registry.names() if name.startswith(parts[0])]

    cmd = parts[0]
    spec = registry.get(cmd)
    if spec is None:
        return [name for name in registry.names() if name.startswith(cmd)]

    # Subcommand completion
    if spec.subcommands and (len(parts) == 1 or (len(parts) == 2 and not trailing)):
        needle = "" if trailing or len(parts) == 1 else parts[1]
        return [sub for sub in spec.subcommands if sub.startswith(needle)]

    # Key argument completion by command patterns
    if cmd == "/session" and len(parts) >= 2 and parts[1] in {"switch", "delete", "rename"}:
        if len(parts) == 2 and trailing:
            return session_ids
        if len(parts) == 3 and not trailing:
            return [sid for sid in session_ids if sid.startswith(parts[2])]
    if cmd == "/config" and len(parts) >= 2 and parts[1] in {"get", "set"}:
        if len(parts) == 2 and trailing:
            return config_keys
        if len(parts) == 3 and not trailing:
            return [k for k in config_keys if k.startswith(parts[2])]
    if cmd == "/tools" and len(parts) >= 2 and parts[1] in {"enable", "disable"}:
        if len(parts) == 2 and trailing:
            return tool_names
        if len(parts) == 3 and not trailing:
            return [t for t in tool_names if t.startswith(parts[2])]
    if cmd in {"/read", "/write", "/ls", "/grep"}:
        needle = ""
        if trailing:
            needle = ""
        elif len(parts) > 1:
            needle = parts[-1]
        paths = []
        for p in project_root.rglob("*"):
            if len(paths) >= 200:
                break
            rel = str(p.relative_to(project_root))
            if needle and not rel.startswith(needle):
                continue
            paths.append(rel)
        return paths

    return []
