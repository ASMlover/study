from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path
from typing import Literal

MessageRole = Literal["system", "user", "assistant", "tool"]


def _now_iso() -> str:
    return datetime.now(tz=timezone.utc).isoformat().replace("+00:00", "Z")


@dataclass(slots=True)
class SessionMessage:
    role: MessageRole
    content: str
    created_at: str
    tool_call_id: str | None = None
    name: str | None = None


@dataclass(slots=True)
class SessionRecord:
    session_id: str
    title: str
    messages: list[SessionMessage] = field(default_factory=list)
    updated_at: str = field(default_factory=_now_iso)


class SessionStore:
    def __init__(self, state_dir: Path) -> None:
        self.state_dir = state_dir

    def _sessions_dir(self) -> Path:
        return self.state_dir / "sessions"

    def _current_path(self) -> Path:
        return self.state_dir / "current.txt"

    def _session_path(self, session_id: str) -> Path:
        return self._sessions_dir() / f"{session_id}.json"

    def _ensure(self) -> None:
        self._sessions_dir().mkdir(parents=True, exist_ok=True)

    @staticmethod
    def _decode_record(data: dict[str, object]) -> SessionRecord:
        msgs = [
            SessionMessage(
                role=item.get("role", "user"),  # type: ignore[arg-type]
                content=str(item.get("content", "")),
                created_at=str(item.get("created_at", _now_iso())),
                tool_call_id=(str(item["tool_call_id"]) if item.get("tool_call_id") is not None else None),
                name=(str(item["name"]) if item.get("name") is not None else None),
            )
            for item in data.get("messages", [])
            if isinstance(item, dict)
        ]
        return SessionRecord(
            session_id=str(data.get("session_id", "default")),
            title=str(data.get("title", data.get("session_id", "default"))),
            messages=msgs,
            updated_at=str(data.get("updated_at", _now_iso())),
        )

    def list(self) -> list[SessionRecord]:
        self._ensure()
        records: list[SessionRecord] = []
        for path in self._sessions_dir().glob("*.json"):
            try:
                raw = json.loads(path.read_text(encoding="utf-8"))
                if isinstance(raw, dict):
                    records.append(self._decode_record(raw))
            except (OSError, json.JSONDecodeError):
                continue
        return sorted(records, key=lambda x: x.updated_at, reverse=True)

    def create(self, session_id: str | None = None) -> SessionRecord:
        self._ensure()
        sid = (session_id or "").strip() or f"session-{int(datetime.now(tz=timezone.utc).timestamp() * 1000)}"
        rec = SessionRecord(session_id=sid, title=sid, messages=[])
        self.save(rec)
        self.set_current(sid)
        return rec

    def load(self, session_id: str) -> SessionRecord:
        p = self._session_path(session_id)
        if not p.exists():
            raise ValueError(f"session not found: {session_id}")
        raw = json.loads(p.read_text(encoding="utf-8"))
        if not isinstance(raw, dict):
            raise ValueError(f"invalid session file: {session_id}")
        return self._decode_record(raw)

    def save(self, rec: SessionRecord) -> None:
        self._ensure()
        rec.updated_at = _now_iso()
        self._session_path(rec.session_id).write_text(json.dumps(asdict(rec), indent=2), encoding="utf-8")

    def set_current(self, session_id: str) -> None:
        self._ensure()
        self._current_path().write_text(session_id, encoding="utf-8")

    def current(self) -> SessionRecord:
        self._ensure()
        marker = self._current_path()
        if not marker.exists():
            return self.create("default")
        sid = marker.read_text(encoding="utf-8").strip()
        try:
            return self.load(sid)
        except ValueError:
            return self.create("default")

    def delete(self, session_id: str) -> bool:
        p = self._session_path(session_id)
        if not p.exists():
            return False
        p.unlink()
        return True

    def rename(self, old_id: str, new_id: str) -> bool:
        old_path = self._session_path(old_id)
        new_path = self._session_path(new_id)
        if not old_path.exists() or new_path.exists():
            return False
        rec = self.load(old_id)
        rec.session_id = new_id
        rec.title = new_id
        old_path.unlink()
        self.save(rec)
        current = self.current().session_id
        if current == old_id:
            self.set_current(new_id)
        return True
