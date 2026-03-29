#!/usr/bin/env python3
"""Run all .ms scripts with the configured mslangc runner."""

import argparse
import pathlib
import shutil
import subprocess
import sys


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_RUNNER = REPO_ROOT / "build" / "Debug" / "mslangc.exe"


def parse_args(argv=None):
  parser = argparse.ArgumentParser(
    description="Execute all .ms scripts under a directory.")
  parser.add_argument(
    "--runner",
    default=str(DEFAULT_RUNNER),
    help="Path or command name for the mslangc executable.")
  parser.add_argument(
    "--root",
    default=str(REPO_ROOT),
    help="Directory to scan recursively for .ms files.")
  return parser.parse_args(argv)


def resolve_runner(runner_arg):
  candidate = pathlib.Path(runner_arg)
  if candidate.exists():
    return candidate.resolve()

  resolved = shutil.which(runner_arg)
  if resolved is None:
    return None
  return pathlib.Path(resolved)


def find_ms_files(root):
  root = pathlib.Path(root)
  files = [path.relative_to(root) for path in root.rglob("*.ms") if path.is_file()]
  return sorted(files, key=lambda path: path.as_posix())


def run_script(runner, script_path):
  return subprocess.run(
    [str(runner), str(script_path)],
    capture_output=True,
    text=True,
    check=False,
  )


def main(argv=None):
  args = parse_args(argv)
  root = pathlib.Path(args.root).resolve()
  runner = resolve_runner(args.runner)
  if runner is None:
    print(f"error: runner not found: {args.runner}", file=sys.stderr)
    return 2

  scripts = find_ms_files(root)
  if not scripts:
    print(f"No .ms scripts found under {root}")
    return 0

  passed = 0
  failed = 0
  for relative_path in scripts:
    script_path = root / relative_path
    result = run_script(runner, script_path)
    if result.stdout:
      sys.stdout.write(result.stdout)
      if not result.stdout.endswith("\n"):
        sys.stdout.write("\n")
    if result.stderr:
      sys.stderr.write(result.stderr)
      if not result.stderr.endswith("\n"):
        sys.stderr.write("\n")

    if result.returncode == 0:
      passed += 1
      print(f"PASS {relative_path.as_posix()}")
    else:
      failed += 1
      print(f"FAIL {relative_path.as_posix()} (exit {result.returncode})")

  print(f"Summary: {passed} passed, {failed} failed, {len(scripts)} total")
  return 0 if failed == 0 else 1


if __name__ == "__main__":
  raise SystemExit(main())