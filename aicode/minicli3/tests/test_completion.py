from pathlib import Path
from uuid import uuid4

from minicli3.commands.builtin import build_registry
from minicli3.completion.engine import command_completions


def test_command_and_subcommand_completion():
    reg = build_registry()
    project_root = Path.cwd()
    items = command_completions(
        "/se",
        reg,
        session_ids=["default", "demo"],
        config_keys=["api_key", "model"],
        tool_names=["read", "grep"],
        project_root=project_root,
    )
    assert "/session" in items

    sub = command_completions(
        "/session sw",
        reg,
        session_ids=["default", "demo"],
        config_keys=["api_key", "model"],
        tool_names=["read", "grep"],
        project_root=project_root,
    )
    assert "switch" in sub


def test_dynamic_argument_completion():
    reg = build_registry()
    project_root = Path("tests/.sandbox") / str(uuid4())
    project_root.mkdir(parents=True, exist_ok=True)

    items = command_completions(
        "/config set ",
        reg,
        session_ids=["default"],
        config_keys=["api_key", "timeout_ms"],
        tool_names=["read", "grep"],
        project_root=project_root,
    )
    assert "api_key" in items
