#!/usr/bin/env python3
"""Run all .ms script files with the Maple interpreter."""

import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent
BUILD_DIR = PROJECT_ROOT / "build"
MAPLE_EXE = BUILD_DIR / ("maple.exe" if sys.platform == "win32" else "maple")

SEARCH_DIRS = [
    PROJECT_ROOT / "tests" / "basic",
    PROJECT_ROOT / "tests" / "integration",
    PROJECT_ROOT / "tests" / "regression",
    PROJECT_ROOT / "examples",
]


def find_scripts():
    scripts = []
    for d in SEARCH_DIRS:
        if d.is_dir():
            scripts.extend(sorted(d.rglob("*.ms")))
    return scripts


def build_maple():
    if MAPLE_EXE.is_file():
        return True
    print(f"Building maple in {BUILD_DIR} ...")
    BUILD_DIR.mkdir(exist_ok=True)
    r = subprocess.run(
        ["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTS=OFF", ".."],
        cwd=BUILD_DIR,
    )
    if r.returncode != 0:
        return False
    r = subprocess.run(["cmake", "--build", ".", "--config", "Debug"], cwd=BUILD_DIR)
    return r.returncode == 0


def run_script(path: Path):
    print(f"\n{'=' * 60}")
    print(f"  Running: {path.relative_to(PROJECT_ROOT)}")
    print(f"{'=' * 60}")
    r = subprocess.run([str(MAPLE_EXE), str(path)])
    return r.returncode


def main():
    if not build_maple():
        print("ERROR: Failed to build maple.", file=sys.stderr)
        sys.exit(1)

    scripts = find_scripts()
    if not scripts:
        print("No .ms scripts found. Searched:")
        for d in SEARCH_DIRS:
            print(f"  {d}")
        sys.exit(0)

    print(f"Found {len(scripts)} script(s).")
    passed = 0
    failed = 0
    errors = []

    for s in scripts:
        rc = run_script(s)
        if rc == 0:
            passed += 1
            print(f"  [PASS] {s.relative_to(PROJECT_ROOT)}")
        else:
            failed += 1
            errors.append((s, rc))
            print(f"  [FAIL] {s.relative_to(PROJECT_ROOT)} (exit code {rc})")

    print(f"\n{'=' * 60}")
    print(f"Results: {passed} passed, {failed} failed, {len(scripts)} total")
    if errors:
        print("Failed scripts:")
        for p, rc in errors:
            print(f"  {p.relative_to(PROJECT_ROOT)} (exit {rc})")
        sys.exit(1)


if __name__ == "__main__":
    main()
