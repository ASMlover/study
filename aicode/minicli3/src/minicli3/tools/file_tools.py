"""Filesystem tools with restricted path checks."""

from __future__ import annotations

import re
from pathlib import Path


def _resolve(path: str, root: Path) -> Path:
    p = Path(path)
    if not p.is_absolute():
        p = root / p
    return p.resolve()


def _is_allowed(target: Path, allowed_roots: list[Path]) -> bool:
    for base in allowed_roots:
        try:
            target.relative_to(base)
            return True
        except ValueError:
            continue
    return False


def read_file(path: str, *, root: Path, allowed_roots: list[Path]) -> str:
    target = _resolve(path, root)
    if not _is_allowed(target, allowed_roots):
        return f"Error: path outside allowed roots: {target}"
    if not target.exists() or not target.is_file():
        return f"Error: file not found: {target}"
    return target.read_text(encoding="utf-8")


def write_file(path: str, content: str, *, root: Path, allowed_roots: list[Path]) -> str:
    target = _resolve(path, root)
    if not _is_allowed(target, allowed_roots):
        return f"Error: path outside allowed roots: {target}"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8")
    return f"Wrote {len(content)} chars to {target}"


def list_dir(path: str = ".", *, root: Path, allowed_roots: list[Path]) -> str:
    target = _resolve(path, root)
    if not _is_allowed(target, allowed_roots):
        return f"Error: path outside allowed roots: {target}"
    if not target.exists() or not target.is_dir():
        return f"Error: directory not found: {target}"
    rows = []
    for item in sorted(target.iterdir(), key=lambda p: (p.is_file(), p.name.lower())):
        mark = "d" if item.is_dir() else "f"
        rows.append(f"{mark} {item.name}")
    return "\n".join(rows) if rows else "(empty)"


def grep_files(pattern: str, path: str = ".", file_glob: str = "*", *, root: Path, allowed_roots: list[Path]) -> str:
    target = _resolve(path, root)
    if not _is_allowed(target, allowed_roots):
        return f"Error: path outside allowed roots: {target}"
    if not target.exists():
        return f"Error: path not found: {target}"

    try:
        regex = re.compile(pattern)
    except re.error as exc:
        return f"Error: invalid regex: {exc}"

    matches: list[str] = []
    files = target.rglob(file_glob) if target.is_dir() else [target]
    for file_path in files:
        if not file_path.is_file():
            continue
        try:
            lines = file_path.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError:
            continue
        for idx, line in enumerate(lines, start=1):
            if regex.search(line):
                rel = file_path.relative_to(root)
                matches.append(f"{rel}:{idx}: {line.strip()}")
    return "\n".join(matches) if matches else "No matches."
