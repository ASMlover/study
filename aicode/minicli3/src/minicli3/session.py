"""Session persistence for MiniCLI3."""

from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from datetime import datetime, UTC
from pathlib import Path
from typing import Any
from uuid import uuid4


def _now_iso() -> str:
    return datetime.now(UTC).isoformat()


@dataclass
class ChatMessage:
    role: str
    content: str
    tool_calls: list[dict[str, Any]] | None = None
    tool_call_id: str | None = None

    def to_dict(self) -> dict[str, Any]:
        payload: dict[str, Any] = {"role": self.role, "content": self.content}
        if self.tool_calls is not None:
            payload["tool_calls"] = self.tool_calls
        if self.tool_call_id is not None:
            payload["tool_call_id"] = self.tool_call_id
        return payload


@dataclass
class SessionRecord:
    session_id: str
    messages: list[ChatMessage] = field(default_factory=list)
    created_at: str = field(default_factory=_now_iso)
    updated_at: str = field(default_factory=_now_iso)

    def add_message(self, role: str, content: str, tool_calls: list[dict[str, Any]] | None = None) -> None:
        self.messages.append(ChatMessage(role=role, content=content, tool_calls=tool_calls))
        self.updated_at = _now_iso()

    def add_tool_result(self, tool_call_id: str, content: str) -> None:
        self.messages.append(ChatMessage(role="tool", content=content, tool_call_id=tool_call_id))
        self.updated_at = _now_iso()

    def clear(self) -> None:
        self.messages.clear()
        self.updated_at = _now_iso()

    def to_dict(self) -> dict[str, Any]:
        data = asdict(self)
        data["messages"] = [m.to_dict() for m in self.messages]
        return data

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "SessionRecord":
        return cls(
            session_id=payload["session_id"],
            messages=[ChatMessage(**m) for m in payload.get("messages", [])],
            created_at=payload.get("created_at", _now_iso()),
            updated_at=payload.get("updated_at", _now_iso()),
        )


class SessionManager:
    def __init__(self, project_root: Path | None = None):
        self.project_root = project_root or Path.cwd()
        self.sessions_dir = self.project_root / ".minicli3" / "sessions"
        self._current: SessionRecord | None = None

    def _ensure_dir(self) -> None:
        self.sessions_dir.mkdir(parents=True, exist_ok=True)

    def _path_for(self, session_id: str) -> Path:
        return self.sessions_dir / f"{session_id}.json"

    def create(self, session_id: str | None = None) -> SessionRecord:
        self._ensure_dir()
        sid = session_id or str(uuid4())[:8]
        rec = SessionRecord(session_id=sid)
        self.save(rec)
        self._current = rec
        return rec

    def load(self, session_id: str) -> SessionRecord | None:
        path = self._path_for(session_id)
        if not path.exists():
            return None
        payload = json.loads(path.read_text(encoding="utf-8"))
        rec = SessionRecord.from_dict(payload)
        self._current = rec
        return rec

    def current(self) -> SessionRecord:
        if self._current is None:
            self._current = self.load("default") or self.create("default")
        return self._current

    def set_current(self, session_id: str) -> SessionRecord:
        rec = self.load(session_id)
        if rec is None:
            rec = self.create(session_id)
        self._current = rec
        return rec

    def save(self, rec: SessionRecord | None = None) -> None:
        self._ensure_dir()
        target = rec or self.current()
        self._path_for(target.session_id).write_text(
            json.dumps(target.to_dict(), indent=2, ensure_ascii=False),
            encoding="utf-8",
        )

    def list(self) -> list[dict[str, Any]]:
        self._ensure_dir()
        rows: list[dict[str, Any]] = []
        for path in self.sessions_dir.glob("*.json"):
            payload = json.loads(path.read_text(encoding="utf-8"))
            rows.append(
                {
                    "session_id": payload["session_id"],
                    "message_count": len(payload.get("messages", [])),
                    "updated_at": payload.get("updated_at", ""),
                }
            )
        rows.sort(key=lambda row: row["updated_at"], reverse=True)
        return rows

    def delete(self, session_id: str) -> bool:
        path = self._path_for(session_id)
        if not path.exists():
            return False
        path.unlink()
        if self._current and self._current.session_id == session_id:
            self._current = None
        return True

    def rename(self, old_id: str, new_id: str) -> bool:
        old_path = self._path_for(old_id)
        new_path = self._path_for(new_id)
        if (not old_path.exists()) or new_path.exists():
            return False
        payload = json.loads(old_path.read_text(encoding="utf-8"))
        payload["session_id"] = new_id
        new_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")
        old_path.unlink()
        if self._current and self._current.session_id == old_id:
            self._current = SessionRecord.from_dict(payload)
        return True
