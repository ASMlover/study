#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List


ROOT = Path(__file__).resolve().parents[1]


@dataclass
class CommandResult:
    command: List[str]
    cwd: Path
    exit_code: int
    duration_sec: float
    stdout: str
    stderr: str


@dataclass
class LanguageResult:
    language: str
    passed: bool = True
    duration_sec: float = 0.0
    commands: List[CommandResult] = field(default_factory=list)
    failure_reason: str | None = None


def _out(line: str = "") -> None:
    encoding = sys.stdout.encoding or "utf-8"
    sys.stdout.buffer.write(f"{line}\n".encode(encoding, errors="replace"))
    sys.stdout.flush()


def _run(command: List[str], cwd: Path, env: Dict[str, str] | None = None) -> CommandResult:
    executable = shutil.which(command[0]) or command[0]
    resolved_command = [executable, *command[1:]]
    start = time.monotonic()
    try:
        completed = subprocess.run(
            resolved_command,
            cwd=str(cwd),
            env=env,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            shell=False,
        )
        exit_code = completed.returncode
        stdout = completed.stdout
        stderr = completed.stderr
    except FileNotFoundError as exc:
        exit_code = 127
        stdout = ""
        stderr = str(exc)
    duration = time.monotonic() - start
    return CommandResult(
        command=resolved_command,
        cwd=cwd,
        exit_code=exit_code,
        duration_sec=duration,
        stdout=stdout,
        stderr=stderr,
    )


def _cpp_smoke_command() -> List[str]:
    if os.name == "nt":
        debug_exe = ROOT / "build" / "cpp" / "Debug" / "minicli4-cpp.exe"
        release_exe = ROOT / "build" / "cpp" / "Release" / "minicli4-cpp.exe"
        exe = debug_exe if debug_exe.exists() else release_exe
    else:
        exe = ROOT / "build" / "cpp" / "minicli4-cpp"
    return [str(exe), "--version"]


def _print_command_result(language: str, result: CommandResult) -> None:
    command = " ".join(result.command)
    _out(f"[{language}] $ {command}  ({result.duration_sec:.2f}s)")
    if result.exit_code == 0:
        return
    _out(f"[{language}] command failed with exit code {result.exit_code}")
    if result.stdout.strip():
        _out(f"[{language}] stdout:\n{result.stdout.rstrip()}")
    if result.stderr.strip():
        _out(f"[{language}] stderr:\n{result.stderr.rstrip()}")


def run_typescript() -> LanguageResult:
    result = LanguageResult(language="typescript")
    start = time.monotonic()
    env = dict(os.environ)
    commands = [
        ["npm", "run", "build"],
        ["node", "--test", "--experimental-test-isolation=none", "build/tests/unit/completion.test.js"],
        ["node", "build/src/index.js", "--version"],
    ]
    cwd = ROOT / "typescript"
    for cmd in commands:
        cmd_result = _run(cmd, cwd, env=env)
        result.commands.append(cmd_result)
        _print_command_result(result.language, cmd_result)
        if cmd_result.exit_code != 0:
            result.passed = False
            result.failure_reason = f"Command failed: {' '.join(cmd)}"
            break
    result.duration_sec = time.monotonic() - start
    return result


def run_python() -> LanguageResult:
    result = LanguageResult(language="python")
    start = time.monotonic()
    env = dict(os.environ)
    existing = env.get("PYTHONPATH", "")
    src_path = str(ROOT / "python" / "src")
    env["PYTHONPATH"] = f"{src_path}{os.pathsep}{existing}" if existing else src_path
    temp_root = Path(tempfile.gettempdir()) / "minicli4-contracts"
    temp_root.mkdir(parents=True, exist_ok=True)
    commands = [
        [
            sys.executable,
            "-m",
            "pytest",
            "--basetemp",
            str(temp_root / "pytest"),
            "tests/unit/test_completion.py",
        ],
        [sys.executable, "-m", "minicli4_py", "--version"],
    ]
    cwd = ROOT / "python"
    for cmd in commands:
        cmd_result = _run(cmd, cwd, env=env)
        result.commands.append(cmd_result)
        _print_command_result(result.language, cmd_result)
        if cmd_result.exit_code != 0:
            result.passed = False
            result.failure_reason = f"Command failed: {' '.join(cmd)}"
            break
    result.duration_sec = time.monotonic() - start
    return result


def run_cpp() -> LanguageResult:
    result = LanguageResult(language="cpp")
    start = time.monotonic()
    commands = [
        ["cmake", "-S", "cpp", "-B", "build/cpp"],
        ["cmake", "--build", "build/cpp", "--config", "Debug"],
        ["ctest", "--test-dir", "build/cpp", "-C", "Debug", "--output-on-failure"],
        _cpp_smoke_command(),
    ]
    for cmd in commands:
        cmd_result = _run(cmd, ROOT)
        result.commands.append(cmd_result)
        _print_command_result(result.language, cmd_result)
        if cmd_result.exit_code != 0:
            result.passed = False
            result.failure_reason = f"Command failed: {' '.join(cmd)}"
            break
    result.duration_sec = time.monotonic() - start
    return result


def write_report(path: Path, results: List[LanguageResult]) -> None:
    data = {
        "generated_at_epoch": time.time(),
        "host_platform": platform.platform(),
        "results": [
            {
                "language": r.language,
                "passed": r.passed,
                "duration_sec": round(r.duration_sec, 3),
                "failure_reason": r.failure_reason,
                "commands": [
                    {
                        "command": c.command,
                        "cwd": str(c.cwd),
                        "exit_code": c.exit_code,
                        "duration_sec": round(c.duration_sec, 3),
                    }
                    for c in r.commands
                ],
            }
            for r in results
        ],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2), encoding="utf-8")


def print_summary(results: List[LanguageResult]) -> None:
    _out("")
    _out("Summary")
    for result in results:
        status = "PASS" if result.passed else "FAIL"
        _out(f"- {result.language}: {status} ({result.duration_sec:.2f}s)")
        if result.failure_reason:
            _out(f"  reason: {result.failure_reason}")
    passed = all(r.passed for r in results)
    _out(f"- aggregate: {'PASS' if passed else 'FAIL'}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run shared MiniCLI4 contract vectors and smoke checks across all language implementations."
    )
    parser.add_argument(
        "--json-report",
        type=Path,
        default=None,
        help="Optional path to write machine-readable summary JSON.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    results = [
        run_typescript(),
        run_python(),
        run_cpp(),
    ]
    print_summary(results)
    if args.json_report is not None:
        write_report(args.json_report, results)
        _out(f"Report written: {args.json_report}")
    return 0 if all(r.passed for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
