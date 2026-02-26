from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(slots=True)
class CompletionContext:
    command_names: list[str]
    session_ids: list[str]
    config_keys: list[str]
    tool_names: list[str]
    project_root: Path
    project_paths: list[str] | None = None


SUBS: dict[str, list[str]] = {
    "/config": ["get", "set", "list", "reset"],
    "/session": ["list", "new", "switch", "delete", "rename", "current"],
    "/tools": ["enable", "disable"],
    "/agents": ["list", "set"],
}


def complete(text: str, ctx: CompletionContext) -> list[str]:
    if not text.startswith("/"):
        return []
    trailing = text.endswith(" ")
    parts = text.strip().split() if text.strip() else []
    if len(parts) <= 1 and not trailing:
        needle = parts[0] if parts else "/"
        return sorted([name for name in ctx.command_names if name.startswith(needle)])

    cmd = parts[0] if parts else ""
    subs = SUBS.get(cmd)
    if subs:
        if len(parts) == 1 and trailing:
            return subs[:]
        if len(parts) == 2 and not trailing:
            return sorted([sub for sub in subs if sub.startswith(parts[1])])

    if cmd == "/session" and len(parts) > 1 and parts[1] == "switch":
        needle = "" if trailing else (parts[2] if len(parts) > 2 else "")
        return [sid for sid in ctx.session_ids if sid.startswith(needle)]
    if cmd == "/config" and len(parts) > 1 and parts[1] in {"set", "get"}:
        needle = "" if trailing else (parts[2] if len(parts) > 2 else "")
        return [key for key in ctx.config_keys if key.startswith(needle)]
    if cmd == "/tools" and len(parts) > 1 and parts[1] in {"enable", "disable"}:
        needle = "" if trailing else (parts[2] if len(parts) > 2 else "")
        return [name for name in ctx.tool_names if name.startswith(needle)]

    if cmd in {"/read", "/write", "/ls", "/grep", "/tree", "/export", "/add"}:
        needle = "" if trailing else (parts[-1] if parts else "")
        return _complete_paths(ctx.project_root, needle, ctx.project_paths)
    return []


def _complete_paths(project_root: Path, prefix: str, project_paths: list[str] | None) -> list[str]:
    if project_paths is not None:
        return sorted([item for item in project_paths if item.startswith(prefix)])
    out: list[str] = []
    try:
        for path in project_root.rglob("*"):
            if any(skip in path.parts for skip in (".git", "node_modules", ".minicli4")):
                continue
            rel = path.relative_to(project_root).as_posix()
            out.append(rel)
            if len(rel.split("/")) < 6 and path.is_dir():
                continue
            if len(out) > 250:
                break
    except OSError:
        return []
    return sorted([item for item in out if item.startswith(prefix)])
