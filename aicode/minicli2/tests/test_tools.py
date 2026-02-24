"""Tests for tools module."""

import pytest
import tempfile
from pathlib import Path

from src.tools.bash import BashTool
from src.tools.file import FileReadTool, FileWriteTool
from src.tools.grep import GrepTool
from src.tools.registry import ToolRegistry


def test_bash_tool_execute():
    """Test bash tool execution."""
    tool = BashTool()

    # Test simple command
    result = tool.execute("echo hello")
    assert result.success is True
    assert "hello" in result.stdout


def test_bash_tool_dangerous():
    """Test bash tool blocks dangerous commands."""
    tool = BashTool()

    result = tool.execute("rm -rf /")
    assert result.success is False
    assert "dangerous" in result.stderr.lower()


def test_bash_tool_definition():
    """Test bash tool definition."""
    tool = BashTool()
    definition = tool.definition

    assert definition["type"] == "function"
    assert definition["function"]["name"] == "bash"


def test_file_read_tool():
    """Test file read tool."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tool = FileReadTool([Path(tmpdir)])

        # Create test file
        test_file = Path(tmpdir) / "test.txt"
        test_file.write_text("Hello, World!")

        # Read file
        result = tool.read(str(test_file))
        assert result.success is True
        assert result.content == "Hello, World!"


def test_file_read_tool_not_found():
    """Test file read with non-existent file."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tool = FileReadTool([Path(tmpdir)])

        result = tool.read(str(Path(tmpdir) / "nonexistent.txt"))
        assert result.success is False
        assert "not found" in result.error.lower()


def test_file_write_tool():
    """Test file write tool."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tool = FileWriteTool([Path(tmpdir)])

        test_file = Path(tmpdir) / "output.txt"
        result = tool.write(str(test_file), "Test content")

        assert result.success is True
        assert test_file.read_text() == "Test content"


def test_file_write_tool_creates_dir():
    """Test file write creates parent directories."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tool = FileWriteTool([Path(tmpdir)])

        test_file = Path(tmpdir) / "subdir" / "output.txt"
        result = tool.write(str(test_file), "Content")

        assert result.success is True
        assert test_file.read_text() == "Content"


def test_grep_tool():
    """Test grep tool."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tool = GrepTool([Path(tmpdir)])

        # Create test file
        test_file = Path(tmpdir) / "test.py"
        test_file.write_text("def hello():\n    print('hello')\n    return 42")

        # Search
        result = tool.search("hello", str(tmpdir))
        assert result.success is True
        assert len(result.matches) == 2  # Function name and string


def test_grep_tool_no_matches():
    """Test grep with no matches."""
    with tempfile.TemporaryDirectory() as tmpdir:
        tool = GrepTool([Path(tmpdir)])

        test_file = Path(tmpdir) / "test.txt"
        test_file.write_text("Hello, World!")

        result = tool.search("xyz", str(tmpdir))
        assert result.success is True
        assert len(result.matches) == 0


def test_tool_registry():
    """Test tool registry."""
    registry = ToolRegistry()

    # Check default tools are registered
    assert registry.get("bash") is not None
    assert registry.get("read") is not None
    assert registry.get("write") is not None
    assert registry.get("grep") is not None


def test_tool_registry_enable_disable():
    """Test enabling/disabling tools."""
    registry = ToolRegistry()

    # Disable a tool
    registry.disable("bash")
    assert "bash" not in registry.enabled

    # Enable a tool
    registry.enable("bash")
    assert "bash" in registry.enabled


def test_tool_registry_execute():
    """Test executing tools via registry."""
    registry = ToolRegistry()
    registry.enable("bash")  # Enable the bash tool

    # Execute echo command through registry
    result = registry.execute("bash", {"command": "echo hello"})
    assert "hello" in result
