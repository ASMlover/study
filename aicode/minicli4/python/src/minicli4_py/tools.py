from __future__ import annotations

import json
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .config import RuntimeConfig


@dataclass(slots=True)
class ToolResult:
    ok: bool
    output: str
    requires_approval: bool = False


def _within_root(project_root: Path, target: str) -> bool:
    root = project_root.resolve()
    resolved = (project_root / target).resolve()
    return resolved == root or root in resolved.parents


def _allowed_path(config: RuntimeConfig, project_root: Path, target: str) -> bool:
    resolved = (project_root / target).resolve()
    allowed = [(project_root / p).resolve() for p in config.allowed_paths]
    return any(base == resolved or base in resolved.parents for base in allowed)


def _parse_args(raw_args: Any) -> dict[str, Any]:
    if isinstance(raw_args, str):
        try:
            parsed = json.loads(raw_args)
            return parsed if isinstance(parsed, dict) else {}
        except json.JSONDecodeError:
            return {}
    if isinstance(raw_args, dict):
        return raw_args
    return {}


class ToolRegistry:
    def __init__(self, project_root: Path, config: RuntimeConfig) -> None:
        self.project_root = project_root
        self.config = config

    def definitions(self) -> list[dict[str, Any]]:
        def f(name: str, description: str, properties: dict[str, Any], required: list[str] | None = None) -> dict[str, Any]:
            parameters: dict[str, Any] = {"type": "object", "properties": properties}
            if required:
                parameters["required"] = required
            return {
                "type": "function",
                "function": {
                    "name": name,
                    "description": description,
                    "parameters": parameters,
                },
            }

        return [
            f("read_file", "Read a UTF-8 text file.", {"path": {"type": "string"}}, ["path"]),
            f(
                "write_file",
                "Write UTF-8 content to file.",
                {"path": {"type": "string"}, "content": {"type": "string"}},
                ["path", "content"],
            ),
            f("list_dir", "List directory entries.", {"path": {"type": "string"}}, ["path"]),
            f(
                "grep_text",
                "Search project text with regex.",
                {"pattern": {"type": "string"}, "path": {"type": "string"}},
                ["pattern"],
            ),
            f("run_shell", "Run shell command under policy.", {"command": {"type": "string"}}, ["command"]),
            f("project_tree", "Render project tree.", {"path": {"type": "string"}, "depth": {"type": "integer"}}),
            f("session_export", "Export session messages.", {"out": {"type": "string"}, "format": {"type": "string"}}),
        ]

    def list_tool_names(self) -> list[str]:
        return [item["function"]["name"] for item in self.definitions()]

    def execute(self, name: str, raw_args: Any, session_messages_json: str = "") -> ToolResult:
        args = _parse_args(raw_args)

        if name == "read_file":
            p = str(args.get("path", ""))
            if not p or not _within_root(self.project_root, p) or not _allowed_path(self.config, self.project_root, p):
                return ToolResult(ok=False, output="path denied")
            full = (self.project_root / p).resolve()
            if not full.exists():
                return ToolResult(ok=False, output="not found")
            return ToolResult(ok=True, output=full.read_text(encoding="utf-8"))

        if name == "write_file":
            if self.config.safe_mode == "strict":
                return ToolResult(ok=False, output="write requires explicit approval", requires_approval=True)
            p = str(args.get("path", ""))
            content = str(args.get("content", ""))
            if not p or not _within_root(self.project_root, p) or not _allowed_path(self.config, self.project_root, p):
                return ToolResult(ok=False, output="path denied")
            full = (self.project_root / p).resolve()
            full.parent.mkdir(parents=True, exist_ok=True)
            full.write_text(content, encoding="utf-8")
            return ToolResult(ok=True, output=f"wrote {p}")

        if name == "list_dir":
            p = str(args.get("path", "."))
            if not _within_root(self.project_root, p) or not _allowed_path(self.config, self.project_root, p):
                return ToolResult(ok=False, output="path denied")
            full = (self.project_root / p).resolve()
            if not full.exists():
                return ToolResult(ok=False, output="not found")
            entries = [f"{x.name}/" if x.is_dir() else x.name for x in sorted(full.iterdir(), key=lambda v: v.name)]
            return ToolResult(ok=True, output="\n".join(entries))

        if name == "grep_text":
            pattern_raw = str(args.get("pattern", ""))
            if not pattern_raw:
                return ToolResult(ok=False, output="missing pattern")
            start = str(args.get("path", "."))
            if not _within_root(self.project_root, start) or not _allowed_path(self.config, self.project_root, start):
                return ToolResult(ok=False, output="path denied")
            try:
                regex = re.compile(pattern_raw, re.IGNORECASE)
            except re.error:
                return ToolResult(ok=False, output="invalid regex")
            root = (self.project_root / start).resolve()
            files: list[Path] = []
            for path in root.rglob("*"):
                if any(skip in path.parts for skip in (".git", "node_modules", ".minicli4")):
                    continue
                if path.is_file():
                    files.append(path)
                if len(files) >= 500:
                    break
            matches: list[str] = []
            for file in files:
                try:
                    text = file.read_text(encoding="utf-8")
                except (OSError, UnicodeDecodeError):
                    continue
                for idx, line in enumerate(text.splitlines(), start=1):
                    if regex.search(line):
                        rel = file.relative_to(self.project_root).as_posix()
                        matches.append(f"{rel}:{idx}: {line}")
                        if len(matches) >= 50:
                            return ToolResult(ok=True, output="\n".join(matches))
            return ToolResult(ok=True, output="\n".join(matches) if matches else "no matches")

        if name == "run_shell":
            command = str(args.get("command", "")).strip()
            if not command:
                return ToolResult(ok=False, output="missing command")
            head = command.split()[0] if command.split() else ""
            if self.config.safe_mode == "strict" and head not in self.config.shell_allowlist:
                return ToolResult(ok=False, output=f"command blocked in strict mode: {head}", requires_approval=True)
            try:
                proc = subprocess.run(
                    command,
                    shell=True,
                    cwd=self.project_root,
                    text=True,
                    capture_output=True,
                    timeout=min(self.config.timeout_ms / 1000, 30),
                    check=False,
                )
            except subprocess.TimeoutExpired:
                return ToolResult(ok=False, output="command timeout")
            output = "\n".join([proc.stdout or "", proc.stderr or ""]).strip()
            if not output:
                output = f"(exit {proc.returncode})"
            return ToolResult(ok=proc.returncode == 0, output=output)

        if name == "project_tree":
            p = str(args.get("path", "."))
            depth = int(args.get("depth", 3))
            if not _within_root(self.project_root, p) or not _allowed_path(self.config, self.project_root, p):
                return ToolResult(ok=False, output="path denied")
            root = (self.project_root / p).resolve()
            lines: list[str] = []

            def visit(directory: Path, prefix: str, d: int) -> None:
                if d < 0:
                    return
                entries = sorted(directory.iterdir(), key=lambda v: v.name)[:50]
                for entry in entries:
                    lines.append(f"{prefix}{'[D]' if entry.is_dir() else '[F]'} {entry.name}")
                    if entry.is_dir():
                        visit(entry, f"{prefix}  ", d - 1)

            visit(root, "", depth)
            return ToolResult(ok=True, output="\n".join(lines))

        if name == "session_export":
            out = str(args.get("out", ".minicli4/sessions/export.json"))
            if not _within_root(self.project_root, out):
                return ToolResult(ok=False, output="path denied")
            full = (self.project_root / out).resolve()
            full.parent.mkdir(parents=True, exist_ok=True)
            full.write_text(session_messages_json, encoding="utf-8")
            return ToolResult(ok=True, output=f"exported {out}")

        return ToolResult(ok=False, output=f"unknown tool: {name}")
