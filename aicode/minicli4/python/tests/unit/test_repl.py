from __future__ import annotations

import pytest

from minicli4_py.repl import SlashCompleter, _handle_ctrl_l, _handle_escape, _handle_f1
from minicli4_py.tui import PaneState

prompt_toolkit = pytest.importorskip("prompt_toolkit")
from prompt_toolkit.document import Document


def test_slash_completer_replaces_current_token() -> None:
    completer = SlashCompleter(lambda _text: ["max_tokens", "model"])
    doc = Document(text="/config set m", cursor_position=len("/config set m"))
    items = list(completer.get_completions(doc, None))
    assert [item.text for item in items] == ["max_tokens", "model"]
    assert all(item.start_position == -1 for item in items)


class _FakeTui:
    def __init__(self) -> None:
        self.events: list[str] = []
        self.status: list[PaneState] = []
        self.inputs: list[str] = []

    def print_event(self, line: str) -> None:
        self.events.append(line)

    def print_status(self, pane: PaneState) -> None:
        self.status.append(pane)

    def announce_input(self, text: str) -> None:
        self.inputs.append(text)


def test_ctrl_l_hotkey_matches_typescript_behavior() -> None:
    tui = _FakeTui()
    pane = PaneState(
        session_id="default",
        model="glm-5",
        stage="idle",
        tools="read_file",
        mode="strict",
        pending_approval="none",
    )
    lines: list[str] = []
    flush_count = {"n": 0}
    _handle_ctrl_l(True, tui, pane, lambda line: lines.append(line), lambda: flush_count.__setitem__("n", flush_count["n"] + 1))
    assert tui.events == ["[ui] append-only mode; showing latest status"]
    assert len(tui.status) == 1
    assert flush_count["n"] == 1
    assert lines == []


def test_f1_hotkey_announces_help_and_prints_lines() -> None:
    tui = _FakeTui()
    chunks: list[list[str]] = []
    flush_count = {"n": 0}
    _handle_f1(
        True,
        tui,
        lambda lines: chunks.append(lines),
        lambda: ["MiniCLI4 Help", "Usage: /help [command]"],
        lambda: flush_count.__setitem__("n", flush_count["n"] + 1),
    )
    assert tui.inputs == ["/help"]
    assert chunks == [["MiniCLI4 Help", "Usage: /help [command]"]]
    assert flush_count["n"] == 1


def test_escape_hotkey_prints_dismissed_message() -> None:
    lines: list[str] = []
    _handle_escape(lambda line: lines.append(line))
    assert lines == ["[completion] dismissed"]
