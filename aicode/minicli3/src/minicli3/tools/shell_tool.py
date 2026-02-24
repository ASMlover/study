"""Shell tool with command allowlist."""

from __future__ import annotations

import subprocess
from pathlib import Path


def is_command_allowed(command: str, allowlist: list[str]) -> bool:
    cmd = command.strip().lower()
    return any(cmd.startswith(prefix.lower()) for prefix in allowlist)


def run_shell(command: str, *, root: Path, allowlist: list[str], timeout_sec: int = 30) -> str:
    if not is_command_allowed(command, allowlist):
        return "Error: command blocked by restricted policy."
    proc = subprocess.run(
        command,
        cwd=str(root),
        shell=True,
        capture_output=True,
        text=True,
        timeout=timeout_sec,
    )
    output = (proc.stdout or "") + (("\n" + proc.stderr) if proc.stderr else "")
    output = output.strip()
    return output if output else f"Exit code: {proc.returncode}"
