"""Tests for session module."""

import pytest
import tempfile
from pathlib import Path

from src.session import Session, SessionManager


def test_session_creation():
    """Test session creation."""
    session = Session(session_id="test-123")

    assert session.session_id == "test-123"
    assert session.messages == []
    assert session.created_at is not None
    assert session.updated_at is not None


def test_session_add_message():
    """Test adding messages to session."""
    session = Session(session_id="test")
    session.add_message("user", "Hello")

    assert len(session.messages) == 1
    assert session.messages[0]["role"] == "user"
    session.add_message("assistant", "Hi there")
    assert len(session.messages) == 2


def test_session_add_tool_result():
    """Test adding tool results."""
    session = Session(session_id="test")
    session.add_tool_result("call-123", "Result content")

    assert len(session.messages) == 1
    assert session.messages[0]["role"] == "tool"
    assert session.messages[0]["tool_call_id"] == "call-123"
    assert session.messages[0]["content"] == "Result content"


def test_session_clear():
    """Test clearing session."""
    session = Session(session_id="test")
    session.add_message("user", "Hello")
    session.add_message("assistant", "Hi")

    session.clear()

    assert len(session.messages) == 0


def test_session_serialization():
    """Test session to/from dict."""
    session = Session(session_id="test")
    session.add_message("user", "Hello")

    data = session.to_dict()
    assert data["session_id"] == "test"
    assert len(data["messages"]) == 1

    restored = Session.from_dict(data)
    assert restored.session_id == "test"
    assert len(restored.messages) == 1


def test_session_manager():
    """Test session manager."""
    with tempfile.TemporaryDirectory() as tmpdir:
        manager = SessionManager(Path(tmpdir))

        # Create session
        session = manager.create_session("test-session")
        assert session.session_id == "test-session"

        # Load session
        loaded = manager.load_session("test-session")
        assert loaded is not None
        assert loaded.session_id == "test-session"

        # List sessions
        sessions = manager.list_sessions()
        assert len(sessions) == 1
        assert sessions[0]["session_id"] == "test-session"

        # Delete session
        assert manager.delete_session("test-session") is True
        assert manager.load_session("test-session") is None
