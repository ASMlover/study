from __future__ import annotations

from dataclasses import asdict
from datetime import datetime, timezone
from pathlib import Path

from minicli4_py.commands import COMMANDS, CommandContext, run_slash
from minicli4_py.config import RuntimeConfig
from minicli4_py.session import SessionMessage, SessionStore
from minicli4_py.tools import ToolRegistry


def _ctx(tmp_path: Path):
    state = tmp_path / ".minicli4"
    (state / "sessions").mkdir(parents=True, exist_ok=True)
    sessions = SessionStore(state)
    session = sessions.create("default")
    config = RuntimeConfig()
    tools = ToolRegistry(tmp_path, config)
    lines: list[str] = []
    pending = {"value": ""}

    def set_config(next_cfg: RuntimeConfig) -> None:
        nonlocal config, tools
        config = next_cfg
        tools = ToolRegistry(tmp_path, config)

    def set_session(next_session) -> None:
        nonlocal session
        session = next_session

    return {
        "state": state,
        "sessions": sessions,
        "session_get": lambda: session,
        "config_get": lambda: config,
        "tools_get": lambda: tools,
        "lines": lines,
        "context": lambda: CommandContext(
            project_root=tmp_path,
            config=config,
            session=session,
            sessions=sessions,
            tools=tools,
            set_config=set_config,
            set_session=set_session,
            out=lambda line: lines.append(line),
            set_pending_approval=lambda value: pending.__setitem__("value", value),
            clear_pending_approval=lambda: pending.__setitem__("value", ""),
            pending_approval=lambda: pending["value"],
        ),
    }


def test_command_baseline_has_30_commands() -> None:
    assert len(COMMANDS) == 30
    assert "/agents" in COMMANDS
    assert "/approve" in COMMANDS


def test_session_new_switches_session(tmp_path: Path) -> None:
    obj = _ctx(tmp_path)
    keep = run_slash(obj["context"](), "/session new demo")
    assert keep is True
    assert obj["session_get"]().session_id == "demo"
    assert "session created" in "\n".join(obj["lines"])


def test_help_shows_grouped_overview(tmp_path: Path) -> None:
    obj = _ctx(tmp_path)
    run_slash(obj["context"](), "/help")
    text = "\n".join(obj["lines"])
    assert "MiniCLI4 Help" in text
    assert "Usage: /help [command]" in text
    assert "Core:" in text
    assert "Session:" in text
    assert "Tools:" in text
    assert "System:" in text
    assert "/help" in text
    assert "/run" in text


def test_help_specific_command(tmp_path: Path) -> None:
    obj = _ctx(tmp_path)
    run_slash(obj["context"](), "/help config")
    text = "\n".join(obj["lines"])
    assert "/config - Read or update local project configuration." in text
    assert "Usage: /config <get|set|list|reset> [key] [value]" in text
    assert "Subcommands: get, set, list, reset" in text


def test_status_and_doctor_cards(tmp_path: Path) -> None:
    obj = _ctx(tmp_path)
    run_slash(obj["context"](), "/status")
    run_slash(obj["context"](), "/doctor")
    text = "\n".join(obj["lines"])
    assert "[STATUS]" in text
    assert "model       : glm-5" in text
    assert "[DOCTOR]" in text
    assert "model_lock  : ok" in text


def test_config_theme_and_motion_updates(tmp_path: Path) -> None:
    obj = _ctx(tmp_path)
    run_slash(obj["context"](), "/config set theme light")
    run_slash(obj["context"](), "/config set motion minimal")
    cfg = obj["config_get"]()
    assert cfg.theme == "light"
    assert cfg.motion == "minimal"
    assert "config updated" in "\n".join(obj["lines"])


def test_context_usage_percentage(tmp_path: Path) -> None:
    obj = _ctx(tmp_path)
    now = datetime.now(tz=timezone.utc).isoformat().replace("+00:00", "Z")
    session = obj["session_get"]()
    session.messages.extend(
        [
            SessionMessage(role="user", content="a" * 400, created_at=now),
            SessionMessage(role="assistant", content="b" * 400, created_at=now),
        ]
    )
    cfg = RuntimeConfig(**asdict(obj["config_get"]()))
    cfg.max_tokens = 1000
    run_slash(
        CommandContext(
            project_root=tmp_path,
            config=cfg,
            session=session,
            sessions=obj["sessions"],
            tools=obj["tools_get"](),
            set_config=lambda _x: None,
            set_session=lambda _x: None,
            out=lambda line: obj["lines"].append(line),
            set_pending_approval=lambda _x: None,
            clear_pending_approval=lambda: None,
            pending_approval=lambda: "",
        ),
        "/context",
    )
    text = "\n".join(obj["lines"])
    assert "messages=2" in text
    assert "tokens~=200" in text
    assert "context_usage~=20% (200/1000)" in text
