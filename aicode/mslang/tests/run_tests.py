#!/usr/bin/env python3
"""Expect-comment test runner for Maple language (.ms) scripts.

Parses `// expect: <value>` comments from .ms files, runs them through
the mslang interpreter, and compares actual output to expected output.
"""
import glob
import os
import subprocess
import sys

def find_executable():
    """Find the mslang executable."""
    candidates = [
        os.path.join("build", "mslang"),
        os.path.join("build", "mslang.exe"),
        os.path.join("build", "Debug", "mslang.exe"),
        os.path.join("build", "Release", "mslang.exe"),
    ]
    for path in candidates:
        if os.path.isfile(path):
            return path
    return None

def parse_expectations(filepath):
    """Extract expected output and error lines from // expect: comments."""
    expectations = []
    error_expectations = []
    with open(filepath, "r") as f:
        for lineno, line in enumerate(f, 1):
            # Check for runtime error expectation
            idx = line.find("// expect runtime error: ")
            if idx != -1:
                expected = line[idx + len("// expect runtime error: "):].rstrip("\n\r")
                error_expectations.append((lineno, expected))
                continue

            idx = line.find("// expect: ")
            if idx != -1:
                expected = line[idx + len("// expect: "):].rstrip("\n\r")
                expectations.append(expected)
            elif "// expect:" in line:
                # Handle `// expect:` with nothing after (empty output)
                idx = line.find("// expect:")
                expected = line[idx + len("// expect:"):].rstrip("\n\r")
                expectations.append(expected)
    return expectations, error_expectations

def run_test(executable, filepath):
    """Run a single test file and return (passed, message)."""
    expectations, error_expectations = parse_expectations(filepath)
    if not expectations and not error_expectations:
        return True, f"  SKIP {filepath} (no expectations)"

    result = subprocess.run(
        [executable, filepath],
        capture_output=True,
        text=True,
        timeout=10,
    )

    # Handle runtime error tests
    if error_expectations:
        if result.returncode == 0:
            return False, f"  FAIL {filepath}\n    Expected runtime error but got exit code 0"
        stderr = result.stderr
        for lineno, expected_msg in error_expectations:
            if expected_msg not in stderr:
                return False, (
                    f"  FAIL {filepath}\n"
                    f"    Line {lineno}: expected error '{expected_msg}' not found in stderr\n"
                    f"    Stderr: {stderr.strip()}"
                )
        # Also check stdout expectations if any
        if expectations:
            actual_lines = result.stdout.rstrip("\n").split("\n") if result.stdout.strip() else []
            for i, (expected, actual) in enumerate(zip(expectations, actual_lines)):
                if expected != actual:
                    return False, (
                        f"  FAIL {filepath}\n"
                        f"    Output line {i + 1}: expected '{expected}', got '{actual}'"
                    )
        total = len(expectations) + len(error_expectations)
        return True, f"  PASS {filepath} ({total} assertions)"

    actual_lines = result.stdout.rstrip("\n").split("\n") if result.stdout.strip() else []

    if len(actual_lines) != len(expectations):
        msg = (
            f"  FAIL {filepath}\n"
            f"    Expected {len(expectations)} lines, got {len(actual_lines)}\n"
            f"    Expected: {expectations}\n"
            f"    Actual:   {actual_lines}"
        )
        if result.stderr:
            msg += f"\n    Stderr: {result.stderr.strip()}"
        return False, msg

    for i, (expected, actual) in enumerate(zip(expectations, actual_lines)):
        if expected != actual:
            msg = (
                f"  FAIL {filepath}\n"
                f"    Line {i + 1}: expected '{expected}', got '{actual}'"
            )
            return False, msg

    return True, f"  PASS {filepath} ({len(expectations)} assertions)"

def main():
    test_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(test_dir)
    os.chdir(project_dir)

    executable = find_executable()
    if executable is None:
        print("ERROR: Could not find mslang executable. Build the project first.")
        sys.exit(1)

    test_files = sorted(glob.glob(os.path.join("tests", "*.ms")))
    test_files += sorted(glob.glob(os.path.join("tests", "errors", "*.ms")))
    if not test_files:
        print("No test files found.")
        sys.exit(1)

    passed = 0
    failed = 0
    skipped = 0

    print(f"Running {len(test_files)} test files with {executable}\n")

    for filepath in test_files:
        try:
            ok, msg = run_test(executable, filepath)
            print(msg)
            if "SKIP" in msg:
                skipped += 1
            elif ok:
                passed += 1
            else:
                failed += 1
        except subprocess.TimeoutExpired:
            print(f"  FAIL {filepath} (timeout)")
            failed += 1
        except Exception as e:
            print(f"  FAIL {filepath} ({e})")
            failed += 1

    print(f"\nResults: {passed} passed, {failed} failed, {skipped} skipped")
    sys.exit(1 if failed > 0 else 0)

if __name__ == "__main__":
    main()
