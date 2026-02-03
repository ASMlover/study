# -*- coding: UTF-8 -*-
from __future__ import annotations

"""Persistence and retention utilities for uploaded files."""

import json
from datetime import datetime, timezone
from pathlib import Path

from .models import FileRecord, MODE_DEFS, ModeLimit, ServerConfig, now_utc

METADATA_FILENAME = ".metadata.json"


def match_mode(filename: str) -> str | None:
    """Return the matched mode key based on filename tokens."""
    lower = filename.lower()
    ordered = sorted(
        MODE_DEFS,
        key=lambda mode: sum(len(token) for token in mode.tokens),
        reverse=True,
    )
    for mode in ordered:
        if all(token in lower for token in mode.tokens):
            return mode.key
    return None


def metadata_path(upload_dir: Path) -> Path:
    """Return the metadata JSON path inside the upload directory."""
    return upload_dir / METADATA_FILENAME


def load_records(upload_dir: Path) -> dict[str, FileRecord]:
    """Load file metadata from disk and reconcile with actual files."""
    records: dict[str, FileRecord] = {}
    path = metadata_path(upload_dir)
    if path.exists():
        try:
            payload = json.loads(path.read_text(encoding="utf-8"))
            for item in payload.get("records", []):
                record = FileRecord.from_json(item)
                if record:
                    records[record.name] = record
        except (json.JSONDecodeError, OSError):
            records = {}

    if upload_dir.exists():
        for file_path in upload_dir.iterdir():
            if not file_path.is_file():
                continue
            if file_path.name == METADATA_FILENAME:
                continue
            mode = match_mode(file_path.name)
            if not mode:
                continue
            if file_path.name not in records:
                uploaded_at = datetime.fromtimestamp(
                    file_path.stat().st_mtime, tz=timezone.utc
                )
                records[file_path.name] = FileRecord(
                    name=file_path.name,
                    mode=mode,
                    uploaded_at=uploaded_at,
                    size=file_path.stat().st_size,
                )
            else:
                if records[file_path.name].mode != mode:
                    records[file_path.name].mode = mode

    valid_records: dict[str, FileRecord] = {}
    for name, record in records.items():
        file_path = upload_dir / name
        if not file_path.exists():
            continue
        record.size = file_path.stat().st_size
        valid_records[name] = record
    return valid_records


def save_records(upload_dir: Path, records: dict[str, FileRecord]) -> None:
    """Persist file metadata to disk."""
    upload_dir.mkdir(parents=True, exist_ok=True)
    payload = {"records": [record.to_json() for record in records.values()]}
    metadata_path(upload_dir).write_text(
        json.dumps(payload, ensure_ascii=True, indent=2),
        encoding="utf-8",
    )


def mode_limit_for(config: ServerConfig, mode_key: str) -> ModeLimit:
    """Resolve retention limit for a mode with fallback to global settings."""
    return config.mode_limits.get(
        mode_key,
        ModeLimit(max_files=config.max_files_per_mode),
    )


def prune_records(config: ServerConfig, records: dict[str, FileRecord]) -> list[str]:
    """Prune old records beyond max file count per mode."""
    removed: list[str] = []
    for mode in MODE_DEFS:
        limit = mode_limit_for(config, mode.key)
        mode_records = [r for r in records.values() if r.mode == mode.key]
        mode_records.sort(key=lambda r: r.uploaded_at, reverse=True)
        for record in mode_records[limit.max_files :]:
            records.pop(record.name, None)
            removed.append(record.name)
    return removed


def cleanup_removed(upload_dir: Path, removed: list[str]) -> None:
    """Delete files removed by pruning."""
    for name in removed:
        path = upload_dir / name
        try:
            if path.exists():
                path.unlink()
        except OSError:
            continue
