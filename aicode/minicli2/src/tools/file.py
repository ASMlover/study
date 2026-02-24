"""File read/write tools."""

import os
from pathlib import Path
from typing import Optional
from dataclasses import dataclass


@dataclass
class FileResult:
    """File operation result."""

    success: bool
    content: str = ""
    error: str = ""


class FileReadTool:
    """Read files safely."""

    def __init__(self, allowed_dirs: Optional[list[Path]] = None):
        """Initialize file read tool.

        Args:
            allowed_dirs: List of allowed directories. Defaults to current directory.
        """
        self.allowed_dirs = allowed_dirs or [Path.cwd()]

    def _is_path_allowed(self, path: Path) -> bool:
        """Check if path is within allowed directories.

        Args:
            path: Path to check.

        Returns:
            True if allowed, False otherwise.
        """
        try:
            resolved = path.resolve()
            for allowed in self.allowed_dirs:
                allowed_resolved = allowed.resolve()
                if resolved.is_relative_to(allowed_resolved):
                    return True
            return False
        except (ValueError, OSError):
            return False

    def read(self, file_path: str) -> FileResult:
        """Read file content.

        Args:
            file_path: Path to file.

        Returns:
            File result.
        """
        try:
            path = Path(file_path).expanduser()

            if not self._is_path_allowed(path):
                return FileResult(
                    success=False,
                    error=f"Access denied: {file_path} is not within allowed directories",
                )

            if not path.exists():
                return FileResult(
                    success=False,
                    error=f"File not found: {file_path}",
                )

            if not path.is_file():
                return FileResult(
                    success=False,
                    error=f"Not a file: {file_path}",
                )

            # Try multiple encodings
            encodings = ["utf-8", "gbk", "gb2312", "latin-1"]
            content = None

            for encoding in encodings:
                try:
                    with open(path, "r", encoding=encoding) as f:
                        content = f.read()
                    break
                except UnicodeDecodeError:
                    continue

            if content is None:
                # Read as binary and decode
                with open(path, "rb") as f:
                    content = f.read().decode("utf-8", errors="replace")

            return FileResult(success=True, content=content)

        except Exception as e:
            return FileResult(success=False, error=str(e))

    @property
    def definition(self) -> dict:
        """Get tool definition for API.

        Returns:
            Tool definition dict.
        """
        return {
            "type": "function",
            "function": {
                "name": "read",
                "description": "Read the contents of a file from the filesystem. Returns the file contents as a string, or an error message if the file cannot be read.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "file_path": {
                            "type": "string",
                            "description": "The path to the file to read",
                        },
                    },
                    "required": ["file_path"],
                },
            },
        }


class FileWriteTool:
    """Write files safely."""

    def __init__(self, allowed_dirs: Optional[list[Path]] = None):
        """Initialize file write tool.

        Args:
            allowed_dirs: List of allowed directories. Defaults to current directory.
        """
        self.allowed_dirs = allowed_dirs or [Path.cwd()]

    def _is_path_allowed(self, path: Path) -> bool:
        """Check if path is within allowed directories.

        Args:
            path: Path to check.

        Returns:
            True if allowed, False otherwise.
        """
        try:
            resolved = path.resolve()
            for allowed in self.allowed_dirs:
                allowed_resolved = allowed.resolve()
                if resolved.is_relative_to(allowed_resolved):
                    return True
            return False
        except (ValueError, OSError):
            return False

    def write(self, file_path: str, content: str) -> FileResult:
        """Write content to file.

        Args:
            file_path: Path to file.
            content: Content to write.

        Returns:
            File result.
        """
        try:
            path = Path(file_path).expanduser()

            if not self._is_path_allowed(path):
                return FileResult(
                    success=False,
                    error=f"Access denied: {file_path} is not within allowed directories",
                )

            # Create parent directories
            path.parent.mkdir(parents=True, exist_ok=True)

            with open(path, "w", encoding="utf-8") as f:
                f.write(content)

            return FileResult(success=True, content=f"File written: {file_path}")

        except Exception as e:
            return FileResult(success=False, error=str(e))

    @property
    def definition(self) -> dict:
        """Get tool definition for API.

        Returns:
            Tool definition dict.
        """
        return {
            "type": "function",
            "function": {
                "name": "write",
                "description": "Write content to a file. Creates the file if it doesn't exist, or overwrites the file if it does. Returns a success message or error.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "file_path": {
                            "type": "string",
                            "description": "The path to the file to write",
                        },
                        "content": {
                            "type": "string",
                            "description": "The content to write to the file",
                        },
                    },
                    "required": ["file_path", "content"],
                },
            },
        }


def read_file(file_path: str) -> str:
    """Read file and return content as string.

    Args:
        file_path: Path to file.

    Returns:
        File content or error message.
    """
    tool = FileReadTool()
    result = tool.read(file_path)

    if result.success:
        return result.content
    else:
        return f"Error: {result.error}"


def write_file(file_path: str, content: str) -> str:
    """Write content to file and return result as string.

    Args:
        file_path: Path to file.
        content: Content to write.

    Returns:
        Success or error message.
    """
    tool = FileWriteTool()
    result = tool.write(file_path, content)

    if result.success:
        return result.content
    else:
        return f"Error: {result.error}"
