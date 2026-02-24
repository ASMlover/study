"""Session management for MiniCLI2."""

import json
import uuid
from pathlib import Path
from typing import Optional
from datetime import datetime


class Session:
    """Chat session."""

    def __init__(
        self,
        session_id: str,
        messages: Optional[list[dict]] = None,
        created_at: Optional[str] = None,
        updated_at: Optional[str] = None,
    ):
        """Initialize session.

        Args:
            session_id: Session ID.
            messages: Chat messages.
            created_at: Creation timestamp.
            updated_at: Last update timestamp.
        """
        self.session_id = session_id
        self.messages = messages or []
        self.created_at = created_at or datetime.now().isoformat()
        self.updated_at = updated_at or datetime.now().isoformat()

    def add_message(self, role: str, content: str, tool_calls: Optional[list[dict]] = None) -> None:
        """Add message to session.

        Args:
            role: Message role (user/assistant/tool).
            content: Message content.
            tool_calls: Tool calls (for assistant messages).
        """
        message: dict = {"role": role, "content": content}
        if tool_calls:
            message["tool_calls"] = tool_calls

        self.messages.append(message)
        self.updated_at = datetime.now().isoformat()

    def add_tool_result(self, tool_call_id: str, content: str) -> None:
        """Add tool result message.

        Args:
            tool_call_id: Tool call ID.
            content: Tool result content.
        """
        self.messages.append({
            "role": "tool",
            "tool_call_id": tool_call_id,
            "content": content,
        })
        self.updated_at = datetime.now().isoformat()

    def clear(self) -> None:
        """Clear session messages."""
        self.messages = []
        self.updated_at = datetime.now().isoformat()

    def to_dict(self) -> dict:
        """Convert session to dictionary."""
        return {
            "session_id": self.session_id,
            "messages": self.messages,
            "created_at": self.created_at,
            "updated_at": self.updated_at,
        }

    @classmethod
    def from_dict(cls, data: dict) -> "Session":
        """Create session from dictionary.

        Args:
            data: Session data.

        Returns:
            Session instance.
        """
        return cls(
            session_id=data["session_id"],
            messages=data.get("messages", []),
            created_at=data.get("created_at"),
            updated_at=data.get("updated_at"),
        )


class SessionManager:
    """Manages chat sessions."""

    def __init__(self, sessions_dir: Optional[Path] = None):
        """Initialize session manager.

        Args:
            sessions_dir: Path to sessions directory. Defaults to .minicli2/sessions.
        """
        if sessions_dir is None:
            sessions_dir = Path.cwd() / ".minicli2" / "sessions"
        self.sessions_dir = sessions_dir
        self._current_session: Optional[Session] = None
        self._sessions_index: dict[str, str] = {}  # session_id -> filename

    def _ensure_sessions_dir(self) -> None:
        """Ensure sessions directory exists."""
        self.sessions_dir.mkdir(parents=True, exist_ok=True)

    def _get_session_file(self, session_id: str) -> Path:
        """Get session file path.

        Args:
            session_id: Session ID.

        Returns:
            Path to session file.
        """
        return self.sessions_dir / f"{session_id}.json"

    def create_session(self, session_id: Optional[str] = None) -> Session:
        """Create new session.

        Args:
            session_id: Session ID. Auto-generated if not provided.

        Returns:
            New session.
        """
        self._ensure_sessions_dir()

        if session_id is None:
            session_id = str(uuid.uuid4())[:8]

        session = Session(session_id=session_id)
        self._save_session(session)
        self._current_session = session
        return session

    def _save_session(self, session: Session) -> None:
        """Save session to file.

        Args:
            session: Session to save.
        """
        self._ensure_sessions_dir()

        file_path = self._get_session_file(session.session_id)
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(session.to_dict(), f, indent=2, ensure_ascii=False)

    def load_session(self, session_id: str) -> Optional[Session]:
        """Load session by ID.

        Args:
            session_id: Session ID.

        Returns:
            Session if found, None otherwise.
        """
        file_path = self._get_session_file(session_id)

        if not file_path.exists():
            return None

        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)

        session = Session.from_dict(data)
        self._current_session = session
        return session

    def delete_session(self, session_id: str) -> bool:
        """Delete session.

        Args:
            session_id: Session ID.

        Returns:
            True if deleted, False if not found.
        """
        file_path = self._get_session_file(session_id)

        if not file_path.exists():
            return False

        file_path.unlink()

        if self._current_session and self._current_session.session_id == session_id:
            self._current_session = None

        return True

    def list_sessions(self) -> list[dict]:
        """List all sessions.

        Returns:
            List of session info dicts.
        """
        self._ensure_sessions_dir()

        sessions = []
        for file_path in self.sessions_dir.glob("*.json"):
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
                sessions.append({
                    "session_id": data["session_id"],
                    "created_at": data.get("created_at"),
                    "updated_at": data.get("updated_at"),
                    "message_count": len(data.get("messages", [])),
                })

        # Sort by updated_at
        sessions.sort(key=lambda x: x.get("updated_at", ""), reverse=True)
        return sessions

    def get_current_session(self) -> Optional[Session]:
        """Get current session.

        Returns:
            Current session.
        """
        if self._current_session is None:
            # Try to load default session
            default_session = self.load_session("default")
            if default_session is None:
                # Create default session
                default_session = self.create_session("default")
            self._current_session = default_session

        return self._current_session

    def set_current_session(self, session: Session) -> None:
        """Set current session.

        Args:
            session: Session to set as current.
        """
        self._current_session = session
        self._save_session(session)

    def save_current_session(self) -> None:
        """Save current session."""
        if self._current_session:
            self._save_session(self._current_session)

    def clear_current_session(self) -> None:
        """Clear current session messages."""
        if self._current_session:
            self._current_session.clear()
            self._save_session(self._current_session)


# Global session manager instance
_session_manager: Optional[SessionManager] = None


def get_session_manager() -> SessionManager:
    """Get global session manager instance."""
    global _session_manager
    if _session_manager is None:
        _session_manager = SessionManager()
    return _session_manager


def get_current_session() -> Session:
    """Get current session."""
    return get_session_manager().get_current_session()
