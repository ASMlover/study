# -*- coding: UTF-8 -*-
from __future__ import annotations

"""Shared data models for HotfixServer."""

import asyncio
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class ModeDef:
    """Defines a filename matching mode and UI label."""
    key: str
    title: str
    tokens: tuple[str, str]


MODE_DEFS: list[ModeDef] = [
    ModeDef(key="h75_hotfix", title="H75 Hotfix", tokens=("h75", "hotfix")),
    ModeDef(
        key="h75_server_hotfix",
        title="H75 Server Hotfix",
        tokens=("h75", "server_hotfix"),
    ),
    ModeDef(key="h75na_hotfix", title="H75NA Hotfix", tokens=("h75na", "hotfix")),
    ModeDef(
        key="h75na_server_hotfix",
        title="H75NA Server Hotfix",
        tokens=("h75na", "server_hotfix"),
    ),
]


@dataclass(frozen=True)
class ModeLimit:
    """Retention limit for a mode (by count only)."""
    max_files: int


@dataclass
class ServerConfig:
    """Runtime configuration, loaded from CLI and config.json."""
    host: str
    port: int
    upload_dir: Path
    static_dir: Path
    max_files_per_mode: int
    mode_limits: dict[str, ModeLimit]
    delete_allow: set[str]
    trust_x_forwarded_for: bool
    config_path: Path
    log_level: int
    log_file: Path


@dataclass
class FileRecord:
    """Metadata stored for each uploaded file."""
    name: str
    mode: str
    uploaded_at: datetime
    size: int

    def to_json(self) -> dict[str, Any]:
        """Serialize to JSON for persistence."""
        return {
            "name": self.name,
            "mode": self.mode,
            "uploaded_at": self.uploaded_at.isoformat(),
            "size": self.size,
        }

    @classmethod
    def from_json(cls, payload: dict[str, Any]) -> FileRecord | None:
        """Parse from JSON, returning None if payload is invalid."""
        try:
            name = str(payload["name"])
            mode = str(payload["mode"])
            uploaded_at = datetime.fromisoformat(str(payload["uploaded_at"]))
            size = int(payload.get("size", 0))
        except (KeyError, ValueError, TypeError):
            return None
        if uploaded_at.tzinfo is None:
            uploaded_at = uploaded_at.replace(tzinfo=timezone.utc)
        return cls(name=name, mode=mode, uploaded_at=uploaded_at, size=size)


@dataclass
class ServerState:
    """In-memory state shared by all connections."""
    lock: asyncio.Lock
    records: dict[str, FileRecord]
    config_lock: asyncio.Lock
    last_config_check: float
    config_mtime: float


def now_utc() -> datetime:
    """Return timezone-aware current UTC time."""
    return datetime.now(timezone.utc)
