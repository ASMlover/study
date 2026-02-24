"""Tool registry for MiniCLI2."""

from typing import Optional, Dict, Any, Callable
from dataclasses import dataclass

from src.tools.bash import BashTool, execute_bash
from src.tools.file import FileReadTool, FileWriteTool, read_file, write_file
from src.tools.grep import GrepTool, grep


@dataclass
class Tool:
    """Tool definition."""

    name: str
    description: str
    definition: dict
    handler: Callable[[dict], str]


class ToolRegistry:
    """Registry for available tools."""

    def __init__(self):
        """Initialize tool registry."""
        self._tools: Dict[str, Tool] = {}
        self._enabled_tools: set = set()

        # Register default tools
        self._register_default_tools()

    def _register_default_tools(self) -> None:
        """Register default tools."""
        # Bash tool
        bash_tool = BashTool()
        self.register("bash", bash_tool.definition, execute_bash)

        # File read tool
        read_tool = FileReadTool()
        self.register("read", read_tool.definition, read_file)

        # File write tool
        write_tool = FileWriteTool()
        self.register("write", write_tool.definition, write_file)

        # Grep tool
        grep_tool = GrepTool()
        self.register("grep", grep_tool.definition, grep)

    def register(
        self,
        name: str,
        definition: dict,
        handler: Callable[[dict], str],
    ) -> None:
        """Register a tool.

        Args:
            name: Tool name.
            definition: Tool definition for API.
            handler: Tool handler function.
        """
        func_def = definition.get("function", {})
        self._tools[name] = Tool(
            name=name,
            description=func_def.get("description", ""),
            definition=definition,
            handler=handler,
        )

    def unregister(self, name: str) -> bool:
        """Unregister a tool.

        Args:
            name: Tool name.

        Returns:
            True if tool was unregistered, False if not found.
        """
        if name in self._tools:
            del self._tools[name]
            self._enabled_tools.discard(name)
            return True
        return False

    def enable(self, name: str) -> bool:
        """Enable a tool.

        Args:
            name: Tool name.

        Returns:
            True if tool was enabled, False if not found.
        """
        if name in self._tools:
            self._enabled_tools.add(name)
            return True
        return False

    def disable(self, name: str) -> bool:
        """Disable a tool.

        Args:
            name: Tool name.

        Returns:
            True if tool was disabled.
        """
        self._enabled_tools.discard(name)
        return True

    def get(self, name: str) -> Optional[Tool]:
        """Get tool by name.

        Args:
            name: Tool name.

        Returns:
            Tool if found, None otherwise.
        """
        return self._tools.get(name)

    def list_tools(self, enabled_only: bool = False) -> list[dict]:
        """List available tools.

        Args:
            enabled_only: Only list enabled tools.

        Returns:
            List of tool info dicts.
        """
        tools = []
        for name, tool in self._tools.items():
            if enabled_only and name not in self._enabled_tools:
                continue

            tools.append({
                "name": tool.name,
                "description": tool.description,
                "enabled": name in self._enabled_tools,
            })

        return tools

    def get_enabled_tools(self) -> list[dict]:
        """Get definitions for enabled tools.

        Returns:
            List of tool definitions.
        """
        definitions = []
        for name in self._enabled_tools:
            tool = self._tools.get(name)
            if tool:
                definitions.append(tool.definition)
        return definitions

    def execute(self, name: str, arguments: dict) -> str:
        """Execute a tool.

        Args:
            name: Tool name.
            arguments: Tool arguments.

        Returns:
            Tool execution result.
        """
        tool = self._tools.get(name)
        if not tool:
            return f"Error: Tool '{name}' not found"

        if name not in self._enabled_tools:
            return f"Error: Tool '{name}' is not enabled"

        try:
            # Pass keyword arguments to handler
            return tool.handler(**arguments)
        except Exception as e:
            return f"Error executing tool: {e}"

    def set_enabled_tools(self, tools: list[str]) -> None:
        """Set enabled tools.

        Args:
            tools: List of tool names to enable.
        """
        self._enabled_tools = set(tools) & set(self._tools.keys())

    @property
    def enabled(self) -> set:
        """Get enabled tool names."""
        return self._enabled_tools.copy()


# Global registry instance
_registry: Optional[ToolRegistry] = None


def get_tool_registry() -> ToolRegistry:
    """Get global tool registry instance."""
    global _registry
    if _registry is None:
        _registry = ToolRegistry()
    return _registry
