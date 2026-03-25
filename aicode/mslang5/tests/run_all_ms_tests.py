#!/usr/bin/env python3
"""Run all Maple .ms tests and report overall pass rate.

Coverage:
- tests/conformance/**/*.ms: validated by metadata header (@expect/@stdout/@diag)
- tests/scripts/**/*.ms: validated by naming convention (error scripts must fail)

This runner is cross-platform and only depends on Python 3 stdlib.
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


SUPPORT_MODULE_SCRIPTS = {
    "tests/scripts/module/cycle_a.ms",
    "tests/scripts/module/cycle_b.ms",
    "tests/scripts/module/side.ms",
    "tests/scripts/module/util.ms",
    "tests/scripts/module/pkg/math.ms",
}


@dataclass
class RunResult:
    returncode: int
    stdout: str
    stderr: str

    @property
    def combined(self) -> str:
        return self.stdout + self.stderr


@dataclass
class CaseResult:
    path: str
    group: str
    expected: str
    passed: bool
    returncode: int
    detail: str


def normalize_newline(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def detect_cli(repo_root: Path, cli_arg: Optional[str]) -> Path:
    if cli_arg:
        candidate = Path(cli_arg)
        if not candidate.is_absolute():
            candidate = repo_root / candidate
        candidate = candidate.resolve()
        if candidate.exists():
            return candidate
        raise FileNotFoundError(f"maple_cli not found: {candidate}")

    candidates = [
        repo_root / "build" / "Debug" / "maple_cli.exe",
        repo_root / "build" / "maple_cli.exe",
        repo_root / "build" / "maple_cli",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate.resolve()
    raise FileNotFoundError("maple_cli not found. Build the project first.")


def parse_conformance_header(path: Path) -> dict[str, str]:
    meta: dict[str, str] = {}
    stdout_lines: list[str] = []

    in_header = True
    capture_stdout = False
    for raw_line in normalize_newline(path.read_text(encoding="utf-8")).split("\n"):
        line = raw_line
        if in_header and line.startswith("// @"):
            capture_stdout = False
            payload = line[4:]
            if ":" not in payload:
                continue
            key, value = payload.split(":", 1)
            key = key.strip()
            value = value.strip()
            if key == "stdout":
                capture_stdout = True
            elif key == "spec":
                pass
            else:
                meta[key] = value
            continue

        if in_header and capture_stdout and line.startswith("//"):
            value = line[2:]
            if value.startswith(" "):
                value = value[1:]
            stdout_lines.append(value)
            continue

        if in_header:
            in_header = False

    if stdout_lines:
        meta["stdout"] = "\n".join(stdout_lines) + "\n"
    return meta


def run_case(cli_path: Path, repo_root: Path, script_rel: str) -> RunResult:
    script_abs = (repo_root / script_rel).resolve()
    if not script_abs.exists():
        raise FileNotFoundError(f"script not found: {script_abs}")

    module_root = repo_root / "tests" / "scripts" / "module"

    with tempfile.TemporaryDirectory(prefix="maple_ms_case_") as tmp:
        tmp_path = Path(tmp)
        shutil.copy2(script_abs, tmp_path / "entry.ms")

        if module_root.exists():
            for item in module_root.iterdir():
                dst = tmp_path / item.name
                if item.is_dir():
                    shutil.copytree(item, dst)
                else:
                    shutil.copy2(item, dst)

        proc = subprocess.run(
            [str(cli_path), "entry.ms"],
            cwd=str(tmp_path),
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )

    return RunResult(
        returncode=proc.returncode,
        stdout=normalize_newline(proc.stdout),
        stderr=normalize_newline(proc.stderr),
    )


def is_expected_error_script(script_rel: str) -> bool:
    normalized = script_rel.replace("\\", "/").lower()
    if normalized in {
        "tests/scripts/module/cycle_a.ms",
        "tests/scripts/module/cycle_b.ms",
    }:
        return True

    stem = Path(script_rel).stem
    return ("_error" in stem) or stem.startswith("error_") or stem.endswith("_error")


def evaluate_conformance(cli_path: Path, repo_root: Path, script_rel: str) -> CaseResult:
    path = repo_root / script_rel
    meta = parse_conformance_header(path)
    expected = meta.get("expect", "")

    run = run_case(cli_path, repo_root, script_rel)

    passed = False
    detail = ""

    if expected == "ok":
        if run.returncode != 0:
            detail = "expected rc=0"
        elif "stdout" in meta and run.stdout != meta["stdout"]:
            detail = "stdout mismatch"
        else:
            passed = True
    elif expected in {"compile_error", "runtime_error"}:
        if run.returncode == 0:
            detail = "expected non-zero rc"
        else:
            phase = meta.get("diag.phase", "")
            code = meta.get("diag.code", "")
            marker = f"[{phase} {code}]" if phase and code else ""
            if marker and (marker not in run.combined and code not in run.combined):
                detail = f"missing diag marker/code {marker}"
            elif "diag.message_contains" in meta and meta["diag.message_contains"] not in run.combined:
                detail = "missing diag.message_contains"
            else:
                passed = True
    else:
        detail = "unsupported @expect"

    return CaseResult(
        path=script_rel,
        group="conformance",
        expected=expected or "unknown",
        passed=passed,
        returncode=run.returncode,
        detail=detail,
    )


def evaluate_script_fixture(cli_path: Path, repo_root: Path, script_rel: str) -> CaseResult:
    expect_error = is_expected_error_script(script_rel)
    run = run_case(cli_path, repo_root, script_rel)

    if expect_error:
        passed = run.returncode != 0
        detail = "" if passed else "expected non-zero rc"
        expected = "error"
    else:
        passed = run.returncode == 0
        detail = "" if passed else "expected rc=0"
        expected = "ok"

    return CaseResult(
        path=script_rel,
        group="scripts",
        expected=expected,
        passed=passed,
        returncode=run.returncode,
        detail=detail,
    )


def print_case_result(index: int, total: int, case: CaseResult) -> None:
    status = "PASS" if case.passed else "FAIL"
    suffix = ""
    if case.detail:
        suffix = f" detail={case.detail}"
    print(
        f"[{index:03d}/{total:03d}] {status} "
        f"[{case.group}] {case.path} expected={case.expected} rc={case.returncode}{suffix}"
    )


def summarize_group(results: list[CaseResult], group: str) -> tuple[int, int, float]:
    group_cases = [r for r in results if r.group == group]
    total = len(group_cases)
    passed = sum(1 for r in group_cases if r.passed)
    rate = (passed * 100.0 / total) if total else 0.0
    return total, passed, rate


def main() -> int:
    parser = argparse.ArgumentParser(description="Run all Maple .ms tests and report pass rate.")
    parser.add_argument("--cli", default=None, help="Path to maple_cli executable.")
    parser.add_argument(
        "--skip-support-modules",
        action="store_true",
        help="Skip module support fixtures (cycle_a/cycle_b/side/util/pkg.math).",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    cli_path = detect_cli(repo_root, args.cli)

    conformance_files = sorted((repo_root / "tests" / "conformance").rglob("*.ms"))
    script_files = sorted((repo_root / "tests" / "scripts").rglob("*.ms"))

    skipped: list[str] = []
    plan: list[tuple[str, str]] = []

    for path in conformance_files:
        plan.append(("conformance", path.relative_to(repo_root).as_posix()))

    for path in script_files:
        rel = path.relative_to(repo_root).as_posix()
        if args.skip_support_modules and rel.lower() in SUPPORT_MODULE_SCRIPTS:
            skipped.append(rel)
            continue
        plan.append(("scripts", rel))

    total_planned = len(plan)
    results: list[CaseResult] = []

    print(f"maple_cli: {cli_path}")
    print(f"planned_cases: {total_planned}")
    print("running cases...")

    for index, (group, rel) in enumerate(plan, start=1):
        if group == "conformance":
            case = evaluate_conformance(cli_path, repo_root, rel)
        else:
            case = evaluate_script_fixture(cli_path, repo_root, rel)
        results.append(case)
        print_case_result(index, total_planned, case)

    total = len(results)
    passed_count = sum(1 for r in results if r.passed)
    failed = [r for r in results if not r.passed]
    failed_count = len(failed)
    pass_rate = (passed_count * 100.0 / total) if total > 0 else 0.0

    conf_total, conf_passed, conf_rate = summarize_group(results, "conformance")
    scripts_total, scripts_passed, scripts_rate = summarize_group(results, "scripts")

    print("\nsummary:")
    print(f"total: {total}")
    print(f"passed: {passed_count}")
    print(f"failed: {failed_count}")
    print(f"skipped: {len(skipped)}")
    print(f"pass_rate: {pass_rate:.2f}%")
    print(
        "groups: "
        f"conformance {conf_passed}/{conf_total} ({conf_rate:.2f}%), "
        f"scripts {scripts_passed}/{scripts_total} ({scripts_rate:.2f}%)"
    )

    if skipped:
        print("\nskipped support module fixtures:")
        for path in skipped:
            print(f"  - {path}")

    if failed:
        print("\nfailed cases:")
        for case in failed:
            print(
                f"  - {case.path} [{case.group}] "
                f"expected={case.expected} rc={case.returncode} detail={case.detail}"
            )
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
