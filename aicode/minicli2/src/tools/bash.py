"""Bash command execution tool."""

import subprocess
import shutil
from typing import Optional
from dataclasses import dataclass


# Dangerous commands blacklist
DANGEROUS_COMMANDS = [
    "rm -rf /",
    "mkfs",
    "dd if=/dev/zero",
    ":(){:|:&};:",  # Fork bomb
    "chmod -R 777 /",
    "chown -R",
]


@dataclass
class BashResult:
    """Bash execution result."""

    success: bool
    stdout: str
    stderr: str
    return_code: int


class BashTool:
    """Execute bash commands safely."""

    def __init__(self, timeout: int = 60):
        """Initialize bash tool.

        Args:
            timeout: Command timeout in seconds.
        """
        self.timeout = timeout

    def is_command_safe(self, command: str) -> bool:
        """Check if command is safe to execute.

        Args:
            command: Command to check.

        Returns:
            True if safe, False otherwise.
        """
        lower_cmd = command.lower()
        for dangerous in DANGEROUS_COMMANDS:
            if dangerous in lower_cmd:
                return False
        return True

    def execute(self, command: str, working_dir: Optional[str] = None) -> BashResult:
        """Execute bash command.

        Args:
            command: Command to execute.
            working_dir: Working directory.

        Returns:
            Execution result.
        """
        # Security check
        if not self.is_command_safe(command):
            return BashResult(
                success=False,
                stdout="",
                stderr="Command blocked: potentially dangerous command detected",
                return_code=1,
            )

        # Check if bash is available
        bash_path = shutil.which("bash")
        if bash_path is None:
            # Try sh on Windows
            bash_path = shutil.which("sh")

        try:
            result = subprocess.run(
                command,
                shell=True,
                capture_output=True,
                text=True,
                timeout=self.timeout,
                cwd=working_dir,
            )
            return BashResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
            )
        except subprocess.TimeoutExpired:
            return BashResult(
                success=False,
                stdout="",
                stderr=f"Command timed out after {self.timeout} seconds",
                return_code=124,
            )
        except Exception as e:
            return BashResult(
                success=False,
                stdout="",
                stderr=str(e),
                return_code=1,
            )

    @property
    def definition(self) -> dict:
        """Get tool definition for API.

        Returns:
            Tool definition dict.
        """
        return {
            "type": "function",
            "function": {
                "name": "bash",
                "description": "Execute bash shell commands. Use this to run terminal commands, git operations, file operations, or any command output or system commands. Returns error message.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "command": {
                            "type": "string",
                            "description": "The bash command to execute",
                        },
                        "working_dir": {
                            "type": "string",
                            "description": "Optional working directory for the command",
                        },
                    },
                    "required": ["command"],
                },
            },
        }


def execute_bash(command: str, working_dir: Optional[str] = None) -> str:
    """Execute bash command and return result as string.

    Args:
        command: Command to execute.
        working_dir: Working directory.

    Returns:
        Result string.
    """
    tool = BashTool()
    result = tool.execute(command, working_dir)

    if result.success:
        return result.stdout or "(command executed successfully with no output)"
    else:
        return f"Error: {result.stderr}"
