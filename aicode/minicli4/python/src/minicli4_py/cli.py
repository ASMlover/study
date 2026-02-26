from __future__ import annotations

import sys
from pathlib import Path

from .repl import ReplRuntime, start_repl


def run_cli(argv: list[str]) -> int:
    if len(argv) == 1 and argv[0] in {"-v", "--version"}:
        sys.stdout.write("minicli4-py 0.1.0\n")
        return 0
    if argv and argv[0] != "repl":
        sys.stderr.write(f"Unknown argument: {argv[0]}\n")
        return 1
    start_repl(
        ReplRuntime(
            input=sys.stdin,
            output=sys.stdout,
            stdout=lambda text: sys.stdout.write(text),
            stderr=lambda text: sys.stderr.write(text),
        ),
        project_root=Path.cwd(),
    )
    return 0
