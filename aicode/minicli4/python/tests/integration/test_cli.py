from __future__ import annotations

import io
import sys
from pathlib import Path

from minicli4_py.cli import run_cli
from minicli4_py.repl import ReplRuntime, start_repl


def test_cli_version_flag() -> None:
    out = io.StringIO()
    err = io.StringIO()
    old_out = sys.stdout
    old_err = sys.stderr
    sys.stdout = out
    sys.stderr = err
    try:
        code = run_cli(["--version"])
    finally:
        sys.stdout = old_out
        sys.stderr = old_err
    assert code == 0
    assert "0.1.0" in out.getvalue()


def test_cli_invalid_arg() -> None:
    out = io.StringIO()
    err = io.StringIO()
    old_out = sys.stdout
    old_err = sys.stderr
    sys.stdout = out
    sys.stderr = err
    try:
        code = run_cli(["unknown"])
    finally:
        sys.stdout = old_out
        sys.stderr = old_err
    assert code == 1
    assert "Unknown argument" in err.getvalue()


def test_repl_shows_prompt_and_exit_message(tmp_path: Path) -> None:
    output = io.StringIO()
    input_stream = io.StringIO("/exit\n")
    runtime = ReplRuntime(
        input=input_stream,
        output=output,
        stdout=lambda text: output.write(text),
        stderr=lambda text: output.write(text),
    )
    start_repl(runtime, project_root=tmp_path)
    text = output.getvalue()
    assert "MiniCLI4 Python REPL" in text
    assert "Type /help for commands." in text
    assert "> " in text
    assert "Bye." in text
