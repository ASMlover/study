"""Code search tool."""

import re
from pathlib import Path
from typing import Optional
from dataclasses import dataclass


@dataclass
class GrepResult:
    """Grep result."""

    success: bool
    matches: list[dict] = None
    error: str = ""

    def __post_init__(self):
        if self.matches is None:
            self.matches = []


class GrepTool:
    """Search code files."""

    def __init__(self, allowed_dirs: Optional[list[Path]] = None):
        """Initialize grep tool.

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

    def search(
        self,
        pattern: str,
        path: str = ".",
        file_pattern: str = "*",
        context_lines: int = 2,
    ) -> GrepResult:
        """Search for pattern in files.

        Args:
            pattern: Search pattern (regex supported).
            path: Search path.
            file_pattern: File pattern (glob).
            context_lines: Number of context lines.

        Returns:
            Grep result.
        """
        try:
            search_path = Path(path).expanduser()

            if not self._is_path_allowed(search_path):
                return GrepResult(
                    success=False,
                    error=f"Access denied: {path} is not within allowed directories",
                )

            if not search_path.exists():
                return GrepResult(
                    success=False,
                    error=f"Path not found: {path}",
                )

            # Compile regex
            try:
                regex = re.compile(pattern)
            except re.error as e:
                return GrepResult(
                    success=False,
                    error=f"Invalid regex pattern: {e}",
                )

            matches = []
            files = search_path.rglob(file_pattern)

            for file_path in files:
                if not file_path.is_file():
                    continue

                if not self._is_path_allowed(file_path):
                    continue

                # Skip binary files
                try:
                    with open(file_path, "r", encoding="utf-8") as f:
                        lines = f.readlines()
                except (UnicodeDecodeError, PermissionError):
                    continue

                for line_num, line in enumerate(lines, start=1):
                    if regex.search(line):
                        matches.append({
                            "file": str(file_path),
                            "line": line_num,
                            "content": line.rstrip(),
                        })

            return GrepResult(success=True, matches=matches)

        except Exception as e:
            return GrepResult(success=False, error=str(e))

    @property
    def definition(self) -> dict:
        """Get tool definition for API.

        Returns:
            Tool definition dict.
        """
        return {
            "type": "function",
            "function": {
                "name": "grep",
                "description": "Search for text patterns in code files using regex. Returns matching lines with file paths and line numbers.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "pattern": {
                            "type": "string",
                            "description": "The regex pattern to search for",
                        },
                        "path": {
                            "type": "string",
                            "description": "The directory path to search in (default: current directory)",
                        },
                        "file_pattern": {
                            "type": "string",
                            "description": "File glob pattern to filter files (e.g., '*.py', '*.js')",
                        },
                    },
                    "required": ["pattern"],
                },
            },
        }


def grep(pattern: str, path: str = ".", file_pattern: str = "*") -> str:
    """Search for pattern and return results as string.

    Args:
        pattern: Search pattern.
        path: Search path.
        file_pattern: File pattern.

    Returns:
        Search results or error message.
    """
    tool = GrepTool()
    result = tool.search(pattern, path, file_pattern)

    if not result.success:
        return f"Error: {result.error}"

    if not result.matches:
        return "No matches found."

    output_lines = []
    for match in result.matches:
        output_lines.append(f"{match['file']}:{match['line']}: {match['content']}")

    return "\n".join(output_lines)
