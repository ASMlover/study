from __future__ import annotations

import json
from dataclasses import asdict
from pathlib import Path

from minicli4_py.completion import CompletionContext, complete
from minicli4_py.commands import COMMANDS
from minicli4_py.config import RuntimeConfig


def test_completion_command_prefix() -> None:
    out = complete(
        "/co",
        CompletionContext(
            command_names=["/config", "/compact", "/context", "/help"],
            session_ids=["default"],
            config_keys=["api_key", "timeout_ms"],
            tool_names=["read_file", "run_shell"],
            project_root=Path.cwd(),
        ),
    )
    assert out == ["/compact", "/config", "/context"]


def test_completion_dynamic_config_keys() -> None:
    out = complete(
        "/config set ",
        CompletionContext(
            command_names=["/config"],
            session_ids=["default"],
            config_keys=["api_key", "max_tokens"],
            tool_names=[],
            project_root=Path.cwd(),
        ),
    )
    assert out == ["api_key", "max_tokens"]


def test_completion_vectors_against_spec() -> None:
    repo_root = Path(__file__).resolve().parents[3]
    vectors = [
        repo_root / "spec" / "test-vectors" / "completion.commands.json",
        repo_root / "spec" / "test-vectors" / "completion.subcommands.json",
        repo_root / "spec" / "test-vectors" / "completion.dynamic.json",
        repo_root / "spec" / "test-vectors" / "completion.paths.json",
    ]
    for vector_file in vectors:
        payload = json.loads(vector_file.read_text(encoding="utf-8-sig"))
        for case in payload["cases"]:
            context = case.get("context", {})
            result = complete(
                case["input"],
                CompletionContext(
                    command_names=COMMANDS,
                    session_ids=context.get("session_ids", ["default"]),
                    config_keys=context.get("config_keys", list(asdict(RuntimeConfig()).keys())),
                    tool_names=context.get("tool_names", ["read_file", "write_file", "run_shell", "grep_text"]),
                    project_root=repo_root,
                    project_paths=context.get("project_paths"),
                ),
            )
            assert result == case["expected"], f"{vector_file.name}:{case['id']}"
