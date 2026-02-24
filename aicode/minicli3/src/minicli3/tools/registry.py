"""Tool registry and execution policy."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable

from minicli3.config import AppConfig
from minicli3.tools.file_tools import grep_files, list_dir, read_file, write_file
from minicli3.tools.shell_tool import run_shell


ToolHandler = Callable[..., str]


@dataclass
class ToolSpec:
    name: str
    description: str
    handler: ToolHandler


class ToolRegistry:
    def __init__(self, project_root: Path, config: AppConfig):
        self.project_root = project_root
        self.config = config
        self.allowed_roots = [(project_root / p).resolve() if not Path(p).is_absolute() else Path(p).resolve() for p in config.allowed_paths]
        self._tools: dict[str, ToolSpec] = {
            "read": ToolSpec("read", "Read a text file", self._read),
            "write": ToolSpec("write", "Write text file content", self._write),
            "grep": ToolSpec("grep", "Search files by regex", self._grep),
            "list_dir": ToolSpec("list_dir", "List directory entries", self._list_dir),
            "run_shell": ToolSpec("run_shell", "Run shell command via allowlist", self._run_shell),
        }
        self._enabled = set(config.enabled_tools) & set(self._tools.keys())

    def _read(self, path: str) -> str:
        return read_file(path, root=self.project_root, allowed_roots=self.allowed_roots)

    def _write(self, path: str, content: str) -> str:
        return write_file(path, content, root=self.project_root, allowed_roots=self.allowed_roots)

    def _grep(self, pattern: str, path: str = ".", file_glob: str = "*") -> str:
        return grep_files(pattern, path, file_glob, root=self.project_root, allowed_roots=self.allowed_roots)

    def _list_dir(self, path: str = ".") -> str:
        return list_dir(path, root=self.project_root, allowed_roots=self.allowed_roots)

    def _run_shell(self, command: str) -> str:
        return run_shell(command, root=self.project_root, allowlist=self.config.shell_allowlist)

    def list_tools(self) -> list[dict[str, Any]]:
        return [
            {"name": spec.name, "description": spec.description, "enabled": name in self._enabled}
            for name, spec in sorted(self._tools.items())
        ]

    def enable(self, name: str) -> bool:
        if name not in self._tools:
            return False
        self._enabled.add(name)
        return True

    def disable(self, name: str) -> bool:
        if name not in self._tools:
            return False
        self._enabled.discard(name)
        return True

    def enabled_tools(self) -> list[str]:
        return sorted(self._enabled)

    def execute(self, name: str, arguments: dict[str, Any]) -> str:
        if name not in self._tools:
            return f"Error: unknown tool '{name}'"
        if name not in self._enabled:
            return f"Error: tool '{name}' is disabled"
        try:
            return self._tools[name].handler(**arguments)
        except Exception as exc:  # pragma: no cover - defensive path
            return f"Error executing tool '{name}': {exc}"

    def tool_definitions(self) -> list[dict[str, Any]]:
        """OpenAI-compatible tool schema for enabled tools."""
        defs: dict[str, dict[str, Any]] = {
            "read": {
                "type": "function",
                "function": {
                    "name": "read",
                    "description": "Read UTF-8 file content from allowed paths.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "path": {"type": "string"},
                        },
                        "required": ["path"],
                    },
                },
            },
            "write": {
                "type": "function",
                "function": {
                    "name": "write",
                    "description": "Write UTF-8 content to a file in allowed paths.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "path": {"type": "string"},
                            "content": {"type": "string"},
                        },
                        "required": ["path", "content"],
                    },
                },
            },
            "grep": {
                "type": "function",
                "function": {
                    "name": "grep",
                    "description": "Search files by regex pattern.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "pattern": {"type": "string"},
                            "path": {"type": "string"},
                            "file_glob": {"type": "string"},
                        },
                        "required": ["pattern"],
                    },
                },
            },
            "list_dir": {
                "type": "function",
                "function": {
                    "name": "list_dir",
                    "description": "List entries in a directory.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "path": {"type": "string"},
                        },
                    },
                },
            },
            "run_shell": {
                "type": "function",
                "function": {
                    "name": "run_shell",
                    "description": "Run an allowlisted shell command in restricted mode.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "command": {"type": "string"},
                        },
                        "required": ["command"],
                    },
                },
            },
        }
        return [defs[name] for name in self.enabled_tools() if name in defs]
