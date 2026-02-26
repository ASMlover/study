from __future__ import annotations

import io
import time

from minicli4_py.ansi import strip_ansi
from minicli4_py.tui import PaneState, TwoPaneTui


def _pane(**overrides) -> PaneState:
    data = {
        "session_id": "default",
        "model": "glm-5",
        "stage": "idle",
        "tools": "run_shell,read_file,list_dir,write_file,session_export,project_tree,grep_text",
        "mode": "strict",
        "pending_approval": "none",
    }
    data.update(overrides)
    return PaneState(**data)


def test_tui_append_only_output() -> None:
    output = io.StringIO()
    tui = TwoPaneTui(output, is_tty=True)
    tui.start(_pane())
    tui.announce_input("/help")
    tui.print_event("MiniCLI4 Help")
    tui.update_status(_pane(stage="planner"))
    tui.print_status(_pane(stage="planner"))
    tui.start_thinking()
    tui.tick_thinking()
    tui.stop_thinking()
    tui.start_assistant_stream()
    tui.append_assistant_chunk("Hello")
    tui.append_assistant_chunk(" world")
    tui.end_assistant_stream()

    text = output.getvalue()
    plain = strip_ansi(text)
    assert "\x1b[2J" not in text
    assert "MiniCLI4 Python Agent CLI" in plain
    assert "[YOU 01] /help" in plain
    assert "MiniCLI4 Help" in plain
    assert "[STATUS] session=default model=glm-5 stage=planner mode=strict approval=none" in plain
    assert "MiniCLI4" in plain
    assert "Hello world" in plain


def test_update_status_is_silent_print_status_is_explicit() -> None:
    output = io.StringIO()
    tui = TwoPaneTui(output, is_tty=True)
    tui.start(_pane())
    tui.update_status(_pane())
    tui.update_status(_pane(stage="planner"))
    tui.print_status(_pane(stage="planner"))
    plain = strip_ansi(output.getvalue())
    assert plain.count("[STATUS]") == 1


def test_thinking_spinner_animates_over_time() -> None:
    output = io.StringIO()
    tui = TwoPaneTui(output, is_tty=True)
    tui.start(_pane())
    tui.start_thinking()
    time.sleep(0.22)
    tui.stop_thinking()
    text = output.getvalue()
    # At least one initial frame and one timer-updated frame should be rendered.
    assert text.count("\r\x1b[2K") >= 2
