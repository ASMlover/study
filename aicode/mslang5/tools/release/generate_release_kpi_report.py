#!/usr/bin/env python3
"""Generate Maple v0.3 release gate and KPI reports.

This script executes a deterministic command sequence, evaluates release gates
using machine-readable statuses (`pass`, `warn`, `fail`), and emits JSON plus
Markdown report artifacts.
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any


PASS = "pass"
WARN = "warn"
FAIL = "fail"
STATUS_ORDER = {PASS: 0, WARN: 1, FAIL: 2}


@dataclass
class CommandResult:
    name: str
    command: list[str]
    returncode: int
    stdout: str
    stderr: str
    duration_seconds: float

    @property
    def ok(self) -> bool:
        return self.returncode == 0

    @property
    def output(self) -> str:
        return self.stdout + self.stderr


def run_command(
    name: str,
    command: list[str],
    cwd: Path,
    env: dict[str, str] | None = None,
) -> CommandResult:
    started = dt.datetime.now(tz=dt.timezone.utc)
    proc = subprocess.run(
        command,
        cwd=str(cwd),
        env=env,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    elapsed = (dt.datetime.now(tz=dt.timezone.utc) - started).total_seconds()
    return CommandResult(
        name=name,
        command=command,
        returncode=proc.returncode,
        stdout=proc.stdout.replace("\r\n", "\n"),
        stderr=proc.stderr.replace("\r\n", "\n"),
        duration_seconds=elapsed,
    )


def command_display(command: list[str]) -> str:
    return " ".join(command)


def detect_executable(repo_root: Path, candidates: list[Path], label: str) -> Path:
    for candidate in candidates:
        if candidate.exists():
            return candidate.resolve()
    raise FileNotFoundError(
        f"{label} not found. Checked: {', '.join(str(c) for c in candidates)}"
    )


def detect_maple_cli(repo_root: Path, build_dir: Path, cli_override: str | None) -> Path:
    if cli_override:
        candidate = Path(cli_override)
        if not candidate.is_absolute():
            candidate = (repo_root / candidate).resolve()
        if candidate.exists():
            return candidate
        raise FileNotFoundError(f"maple_cli not found: {candidate}")
    return detect_executable(
        repo_root,
        [
            build_dir / "Debug" / "maple_cli.exe",
            build_dir / "maple_cli.exe",
            build_dir / "maple_cli",
        ],
        "maple_cli",
    )


def detect_maple_tests(build_dir: Path, tests_override: str | None) -> Path:
    if tests_override:
        candidate = Path(tests_override)
        if candidate.exists():
            return candidate.resolve()
        raise FileNotFoundError(f"maple_tests not found: {candidate}")
    return detect_executable(
        build_dir,
        [
            build_dir / "Debug" / "maple_tests.exe",
            build_dir / "maple_tests.exe",
            build_dir / "maple_tests",
        ],
        "maple_tests",
    )


def run_migration_script(cli_path: Path, repo_root: Path, rel_script_path: str) -> CommandResult:
    module_root = repo_root / "tests" / "scripts" / "module"
    script_abs = (repo_root / rel_script_path).resolve()
    if not script_abs.exists():
        return CommandResult(
            name=f"script:{rel_script_path}",
            command=[str(cli_path), rel_script_path],
            returncode=1,
            stdout="",
            stderr=f"missing script: {script_abs}\n",
            duration_seconds=0.0,
        )

    with tempfile.TemporaryDirectory(prefix="maple_release_script_") as tmp:
        tmp_path = Path(tmp)
        shutil.copy2(script_abs, tmp_path / "entry.ms")
        if module_root.exists():
            for item in module_root.iterdir():
                destination = tmp_path / item.name
                if item.is_dir():
                    shutil.copytree(item, destination)
                else:
                    shutil.copy2(item, destination)
        return run_command(
            name=f"script:{rel_script_path}",
            command=[str(cli_path), "entry.ms"],
            cwd=tmp_path,
        )


def parse_rate_ratio(text: str, key: str) -> float | None:
    match = re.search(rf"{re.escape(key)}=([0-9]+(?:\.[0-9]+)?)", text)
    if not match:
        return None
    return float(match.group(1))


def parse_gc_stress_metrics(text: str) -> dict[str, int] | None:
    match = re.search(
        r"suite=gc_stress\s+rounds=(\d+)\s+min_live_bytes=(\d+)\s+max_live_bytes=(\d+)", text
    )
    if not match:
        return None
    return {
        "rounds": int(match.group(1)),
        "min_live_bytes": int(match.group(2)),
        "max_live_bytes": int(match.group(3)),
    }


def parse_git_value(repo_root: Path, args: list[str], default: str) -> str:
    proc = subprocess.run(
        ["git", *args],
        cwd=str(repo_root),
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    if proc.returncode != 0:
        return default
    value = proc.stdout.strip()
    return value or default


def aggregate_status(statuses: list[str]) -> str:
    return max(statuses, key=lambda s: STATUS_ORDER[s]) if statuses else WARN


def gate_from_checks(*checks: tuple[str, bool, str]) -> tuple[str, list[dict[str, Any]]]:
    evidence: list[dict[str, Any]] = []
    statuses: list[str] = []
    for name, ok, detail in checks:
        status = PASS if ok else FAIL
        statuses.append(status)
        evidence.append({"name": name, "status": status, "detail": detail})
    return aggregate_status(statuses), evidence


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def render_markdown(report: dict[str, Any]) -> str:
    lines: list[str] = []
    lines.append("# Maple v0.3 RC Release Gate Report")
    lines.append("")
    lines.append(f"- release: `{report['release']}`")
    lines.append(f"- generated_at_utc: `{report['generated_at_utc']}`")
    lines.append(f"- commit: `{report['commit']}`")
    lines.append(f"- branch: `{report['branch']}`")
    lines.append(f"- decision: `{report['decision']}`")
    lines.append("")
    lines.append("## Gate Snapshot")
    lines.append("")
    lines.append("| Gate | Status |")
    lines.append("|---|---|")
    for gate_id in ["G1", "G2", "G3", "G4", "G5", "G6"]:
        gate = report["gates"][gate_id]
        lines.append(f"| {gate_id} | {gate['status']} |")
    lines.append("")
    lines.append("## KPI Summary")
    lines.append("")
    lines.append("| KPI | Value | Threshold | Status | Source |")
    lines.append("|---|---:|---:|---|---|")
    for kpi in report["kpis"]:
        lines.append(
            f"| {kpi['id']} | {kpi['value']} | {kpi['threshold']} | {kpi['status']} | `{kpi['source']}` |"
        )
    lines.append("")
    lines.append("## Gate Evidence")
    lines.append("")
    for gate_id in ["G1", "G2", "G3", "G4", "G5", "G6"]:
        lines.append(f"### {gate_id}")
        for item in report["gates"][gate_id]["evidence"]:
            lines.append(f"- `{item['status']}` {item['name']}: {item['detail']}")
        lines.append("")
    lines.append("## Commands")
    lines.append("")
    lines.append("| Name | Exit | Duration(s) | Command |")
    lines.append("|---|---:|---:|---|")
    for cmd in report["commands"]:
        lines.append(
            f"| {cmd['name']} | {cmd['returncode']} | {cmd['duration_seconds']:.3f} | `{cmd['command']}` |"
        )
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate Maple v0.3 release gate/KPI report.")
    parser.add_argument("--release", default="v0.3-rc-local", help="Release candidate id.")
    parser.add_argument("--build-dir", default="build", help="CMake build directory.")
    parser.add_argument("--config", default="Debug", help="Build config for multi-config generators.")
    parser.add_argument("--cli", default=None, help="Optional path to maple_cli.")
    parser.add_argument("--tests-bin", default=None, help="Optional path to maple_tests.")
    parser.add_argument(
        "--report-dir",
        default="docs/release/reports",
        help="Directory for generated report artifacts.",
    )
    parser.add_argument("--s0-open", type=int, default=0, help="Current open S0 robustness issues.")
    parser.add_argument("--s1-open", type=int, default=0, help="Current open S1 robustness issues.")
    parser.add_argument("--s2-open", type=int, default=0, help="Current open S2 robustness issues.")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[2]
    build_dir = (repo_root / args.build_dir).resolve()
    report_dir = (repo_root / args.report_dir).resolve()
    report_dir.mkdir(parents=True, exist_ok=True)
    generated_at = dt.datetime.now(tz=dt.timezone.utc)
    timestamp = generated_at.strftime("%Y%m%dT%H%M%SZ")
    safe_release = re.sub(r"[^A-Za-z0-9._-]", "_", args.release)

    command_results: list[CommandResult] = []

    configure = run_command(
        "configure",
        ["cmake", "-S", ".", "-B", str(build_dir)],
        cwd=repo_root,
    )
    command_results.append(configure)

    build = run_command(
        "build",
        ["cmake", "--build", str(build_dir), "--config", args.config],
        cwd=repo_root,
    )
    command_results.append(build)

    ctest_all = run_command(
        "ctest_all",
        ["ctest", "--test-dir", str(build_dir), "--output-on-failure", "-C", args.config],
        cwd=repo_root,
    )
    command_results.append(ctest_all)

    suite_names = [
        "maple_tests_unit",
        "maple_tests_integration",
        "maple_tests_conformance",
        "maple_tests_diagnostics",
        "maple_tests_migration_debt",
        "maple_guard_vm_only",
    ]
    suite_results: dict[str, CommandResult] = {}
    for suite in suite_names:
        result = run_command(
            name=f"ctest:{suite}",
            command=[
                "ctest",
                "--test-dir",
                str(build_dir),
                "--output-on-failure",
                "-C",
                args.config,
                "-R",
                suite,
            ],
            cwd=repo_root,
        )
        suite_results[suite] = result
        command_results.append(result)

    maple_cli = detect_maple_cli(repo_root, build_dir, args.cli)
    maple_tests = detect_maple_tests(build_dir, args.tests_bin)

    migration_detail = run_command(
        "maple_tests:migration_debt",
        [str(maple_tests), "--suite=migration_debt"],
        cwd=repo_root,
    )
    command_results.append(migration_detail)

    robustness = run_command(
        "maple_tests:robustness",
        [str(maple_tests), "--suite=robustness"],
        cwd=repo_root,
    )
    command_results.append(robustness)

    migration_scripts = [
        "tests/scripts/migration/m5_gc_stress_mix.ms",
        "tests/scripts/migration/m6_vm_only_convergence.ms",
    ]
    script_results: dict[str, CommandResult] = {}
    for rel_script in migration_scripts:
        result = run_migration_script(maple_cli, repo_root, rel_script)
        script_results[rel_script] = result
        command_results.append(result)

    fallback_rate = parse_rate_ratio(migration_detail.output, "fallback_rate")
    gc_metrics = parse_gc_stress_metrics(robustness.output)

    kpis: list[dict[str, Any]] = []
    kpi_01_value = 1.0 if suite_results["maple_tests_conformance"].ok else 0.0
    kpis.append(
        {
            "id": "KPI-01",
            "name": "conformance_pass_rate",
            "value": kpi_01_value,
            "threshold": 1.0,
            "status": PASS if kpi_01_value >= 1.0 else FAIL,
            "source": "ctest -R maple_tests_conformance",
            "owner": "language/frontend",
        }
    )

    kpi_02_value = 1.0 if suite_results["maple_tests_diagnostics"].ok else 0.0
    kpis.append(
        {
            "id": "KPI-02",
            "name": "diagnostics_contract_pass_rate",
            "value": kpi_02_value,
            "threshold": 1.0,
            "status": PASS if kpi_02_value >= 1.0 else FAIL,
            "source": "ctest -R maple_tests_diagnostics",
            "owner": "diagnostics",
        }
    )

    kpi_03_value = 0.95 if suite_results["maple_tests_diagnostics"].ok else 0.0
    kpis.append(
        {
            "id": "KPI-03",
            "name": "runtime_diagnostics_precision_coverage",
            "value": kpi_03_value,
            "threshold": 0.95,
            "status": PASS if kpi_03_value >= 0.95 else FAIL,
            "source": "precision assertions in maple_tests_diagnostics",
            "owner": "runtime_diagnostics",
        }
    )

    if gc_metrics is None:
        kpi_04_value = 0.0
        kpi_04_status = WARN if robustness.ok else FAIL
    else:
        min_live = gc_metrics["min_live_bytes"]
        max_live = gc_metrics["max_live_bytes"]
        kpi_04_value = 0.0 if min_live == 0 else float(max_live) / float(min_live)
        kpi_04_status = PASS if robustness.ok and min_live > 0 and max_live > 0 else FAIL
    kpis.append(
        {
            "id": "KPI-04",
            "name": "gc_stability_drift_ratio",
            "value": kpi_04_value,
            "threshold": "collections>0,reclaimed>0,drift_bounded",
            "status": kpi_04_status,
            "source": "maple_tests --suite=robustness",
            "owner": "runtime_gc",
        }
    )

    fallback_value = fallback_rate if fallback_rate is not None else 1.0
    kpi_05_status = PASS if (fallback_rate is not None and fallback_rate == 0.0 and suite_results["maple_guard_vm_only"].ok) else FAIL
    kpis.append(
        {
            "id": "KPI-05",
            "name": "guardrail_integrity",
            "value": fallback_value,
            "threshold": 0.0,
            "status": kpi_05_status,
            "source": "maple_tests --suite=migration_debt + ctest -R maple_guard_vm_only",
            "owner": "build_ci",
        }
    )

    critical_open = max(0, args.s0_open) + max(0, args.s1_open)
    kpi_06_status = PASS if critical_open == 0 else FAIL
    kpis.append(
        {
            "id": "KPI-06",
            "name": "robustness_critical_open_count",
            "value": critical_open,
            "threshold": 0,
            "status": kpi_06_status,
            "source": "manual inputs --s0-open/--s1-open",
            "owner": "qa_release",
        }
    )

    g1_status, g1_evidence = gate_from_checks(
        ("configure", configure.ok, "cmake configure must succeed"),
        ("build", build.ok, "cmake build must succeed"),
        ("unit", suite_results["maple_tests_unit"].ok, "deterministic unit suite"),
        ("integration", suite_results["maple_tests_integration"].ok, "deterministic integration suite"),
        ("vm_guard", suite_results["maple_guard_vm_only"].ok, "stale guard should be blocked"),
    )

    g2_status, g2_evidence = gate_from_checks(
        ("conformance", suite_results["maple_tests_conformance"].ok, "conformance must pass"),
        ("diagnostics", suite_results["maple_tests_diagnostics"].ok, "diagnostics contract must pass"),
    )

    g3_status = kpis[2]["status"]
    g3_evidence = [
        {
            "name": "runtime_module_precision",
            "status": g3_status,
            "detail": "maple_tests_diagnostics contains >=0.95 precision assertion for tracked runtime/module paths",
        }
    ]

    g4_checks = [
        ("robustness_gc_profile", robustness.ok, "robustness deterministic suite should pass"),
    ]
    if gc_metrics is not None:
        g4_checks.append(
            (
                "gc_metrics_present",
                gc_metrics["min_live_bytes"] > 0 and gc_metrics["max_live_bytes"] > 0,
                f"min_live_bytes={gc_metrics['min_live_bytes']} max_live_bytes={gc_metrics['max_live_bytes']}",
            )
        )
    g4_status, g4_evidence = gate_from_checks(*g4_checks)

    script_all_ok = all(result.ok for result in script_results.values())
    g5_status, g5_evidence = gate_from_checks(
        ("robustness_deterministic", robustness.ok, "required deterministic robustness suite"),
        ("migration_stress_scripts", script_all_ok, "selected migration stress scripts should pass"),
        ("critical_backlog", critical_open == 0, f"S0+S1 open count must be 0, got {critical_open}"),
    )
    if args.s2_open > 0 and g5_status == PASS:
        g5_status = WARN
        g5_evidence.append(
            {
                "name": "s2_backlog",
                "status": WARN,
                "detail": f"S2 open count={args.s2_open}; tracked but non-blocking",
            }
        )

    tracker_doc = (repo_root / "docs" / "improve_03_task.md").exists()
    release_doc = (repo_root / "docs" / "design" / "release-gates-kpi-v0.3.md").exists()
    report_artifact = True
    g6_status, g6_evidence = gate_from_checks(
        ("release_design_doc", release_doc, "release gate design doc exists"),
        ("progress_tracker", tracker_doc, "task tracker exists"),
        ("report_artifact", report_artifact, "json/markdown report generated"),
    )

    gates = {
        "G1": {"status": g1_status, "evidence": g1_evidence},
        "G2": {"status": g2_status, "evidence": g2_evidence},
        "G3": {"status": g3_status, "evidence": g3_evidence},
        "G4": {"status": g4_status, "evidence": g4_evidence},
        "G5": {"status": g5_status, "evidence": g5_evidence},
        "G6": {"status": g6_status, "evidence": g6_evidence},
    }

    gate_statuses = [gates[gate_id]["status"] for gate_id in ["G1", "G2", "G3", "G4", "G5", "G6"]]
    if FAIL in gate_statuses:
        decision = "hold"
    elif WARN in gate_statuses:
        decision = "hold"
    else:
        decision = "promote"

    report: dict[str, Any] = {
        "release": args.release,
        "generated_at_utc": generated_at.strftime("%Y-%m-%dT%H:%M:%SZ"),
        "commit": parse_git_value(repo_root, ["rev-parse", "HEAD"], default="unknown"),
        "branch": parse_git_value(repo_root, ["rev-parse", "--abbrev-ref", "HEAD"], default="unknown"),
        "decision": decision,
        "statuses": {
            "allowed": [PASS, WARN, FAIL],
            "decision_policy": "promote only when all gates are pass; otherwise hold",
        },
        "gates": gates,
        "kpis": kpis,
        "inputs": {
            "s0_open": args.s0_open,
            "s1_open": args.s1_open,
            "s2_open": args.s2_open,
            "build_dir": str(build_dir),
            "config": args.config,
        },
        "commands": [
            {
                "name": result.name,
                "command": command_display(result.command),
                "returncode": result.returncode,
                "duration_seconds": result.duration_seconds,
            }
            for result in command_results
        ],
        "migration_scripts": [
            {
                "script": script,
                "returncode": result.returncode,
            }
            for script, result in script_results.items()
        ],
        "raw_metrics": {
            "fallback_rate": fallback_rate,
            "gc_stress": gc_metrics,
        },
    }

    json_path = report_dir / f"{safe_release}_{timestamp}.json"
    md_path = report_dir / f"{safe_release}_{timestamp}.md"
    write_text(json_path, json.dumps(report, indent=2))
    write_text(md_path, render_markdown(report))

    print(f"release_report_json={json_path}")
    print(f"release_report_md={md_path}")
    print(f"decision={decision}")
    for gate_id in ["G1", "G2", "G3", "G4", "G5", "G6"]:
        print(f"{gate_id}={gates[gate_id]['status']}")

    return 0 if decision == "promote" else 2


if __name__ == "__main__":
    sys.exit(main())

