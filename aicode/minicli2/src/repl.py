"""REPL interactive mode for MiniCLI2."""

import sys
from typing import Optional, Generator
from prompt_toolkit import PromptSession
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from prompt_toolkit.completion import Completer, Completion
from prompt_toolkit.keys import Keys
from prompt_toolkit.key_binding import KeyBindings
from rich.console import Console
from rich.panel import Panel
from rich.table import Table
from rich.text import Text

from src.config import get_config_manager
from src.provider import get_provider
from src.session import get_session_manager, get_current_session
from src.tools.registry import get_tool_registry

console = Console()


class CommandCompleter(Completer):
    """Auto-complete for REPL commands."""

    # Sorted alphabetically
    COMMANDS = sorted([
        "/clear",
        "/config",
        "/context",
        "/exit",
        "/grep",
        "/help",
        "/history",
        "/session",
        "/tools",
    ])

    def get_completions(self, document, complete_event):
        """Get completions for current input.

        Args:
            document: Current document.
            complete_event: Completion event.

        Yields:
            Completions.
        """
        text = document.text

        # Show all commands when starting with /
        if text.startswith("/"):
            # Get the complete word including the leading /
            # since / is treated as a word boundary by default
            if text == "/":
                word = "/"
            elif len(text) >= 2 and text[1].isalpha():
                # Text starts with / and has letters after it
                # Get everything from the / to cursor position
                word = text[1:]  # This will be matched against commands like /help
                # Actually we need to match against full command prefix
                word = text  # Use full text including /
            else:
                word = text

            for cmd in self.COMMANDS:
                if cmd.startswith(text):
                    yield Completion(cmd, start_position=-len(text))


class REPL:
    """REPL interactive mode."""

    def __init__(self):
        """Initialize REPL."""
        self.config_manager = get_config_manager()
        self.session_manager = get_session_manager()
        self.provider = get_provider()
        self.tool_registry = get_tool_registry()
        self.session = self.session_manager.get_current_session()

        # Set up key bindings
        self.key_bindings = KeyBindings()
        self.key_bindings.add(Keys.ControlC)(self._interrupt)
        self.key_bindings.add(Keys.Tab)(self._trigger_completion)

        # Set up prompt session
        self.prompt_session = PromptSession(
            completer=CommandCompleter(),
            auto_suggest=AutoSuggestFromHistory(),
            key_bindings=self.key_bindings,
            complete_while_typing=True,
        )

    def _trigger_completion(self, event):
        """Trigger auto completion on Tab."""
        # Start completion if not already started
        # select_first=True to auto-select the first match
        buf = event.app.current_buffer
        buf.start_completion(select_first=True)
        event.handled = True

    def _interrupt(self, event):
        """Handle interrupt."""
        event.app.current_buffer.text = ""
        console.print("\n[yellow]Interrupted. Type /exit to quit.[/yellow]")

    def _print_welcome(self) -> None:
        """Print welcome message."""
        console.print(Panel(
            "[bold cyan]MiniCLI2[/bold cyan] - AI Agent CLI based on GLM-5\n"
            "Type /help for available commands",
            title="Welcome",
            border_style="cyan",
        ))

    def _print_help(self, command: Optional[str] = None) -> None:
        """Print help message.

        Args:
            command: Specific command to get help for.
        """
        if command:
            self._print_command_help(command)
            return

        table = Table(title="Available Commands")
        table.add_column("Command", style="cyan")
        table.add_column("Description", style="white")

        commands = sorted([
            ("/clear", "Clear current session history"),
            ("/config", "View or modify configuration"),
            ("/context", "View context usage statistics"),
            ("/exit", "Exit current REPL session"),
            ("/grep", "Search code files"),
            ("/help", "Show help information"),
            ("/history", "View session history"),
            ("/session", "Switch or create new session"),
            ("/tools", "List all available tools"),
        ])

        for cmd, desc in commands:
            table.add_row(cmd, desc)

        console.print(table)
        console.print("\n[dim]Type /help <command> for detailed help[/dim]")

    def _print_command_help(self, command: str) -> None:
        """Print help for specific command.

        Args:
            command: Command name.
        """
        help_text = {
            "/exit": "Exit the current REPL session.",
            "/help": "Show this help message. Usage: /help [command]",
            "/clear": "Clear current session history. Usage: /clear",
            "/session": "Manage sessions. Usage: /session [list|new|switch <id>]",
            "/config": "Manage configuration. Usage: /config [get <key>|set <key> <value>|list]",
            "/grep": "Search code. Usage: /grep <pattern> [path] [file_pattern]",
            "/tools": "List all available tools.",
            "/history": "View session history. Usage: /history [--limit N]",
            "/context": "View context usage statistics.",
        }

        if command in help_text:
            console.print(f"[cyan]{command}[/cyan]: {help_text[command]}")
        else:
            console.print(f"[yellow]Unknown command: {command}[/yellow]")
            console.print("Type /help for available commands")

    def _do_exit(self) -> bool:
        """Handle /exit command."""
        self.session_manager.save_current_session()
        console.print("[green]Goodbye![/green]")
        return False

    def _do_help(self, args: list[str]) -> None:
        """Handle /help command.

        Args:
            args: Command arguments.
        """
        command = args[1] if len(args) > 1 else None
        self._print_help(command)

    def _do_clear(self) -> None:
        """Handle /clear command."""
        self.session_manager.clear_current_session()
        console.print("[green]Session cleared.[/green]")

    def _do_session(self, args: list[str]) -> None:
        """Handle /session command.

        Args:
            args: Command arguments.
        """
        if len(args) <= 1:
            self._do_session_list()
            return

        subcommand = args[1]

        if subcommand == "list":
            self._do_session_list()
        elif subcommand == "new":
            session_id = args[2] if len(args) > 2 else None
            session = self.session_manager.create_session(session_id)
            console.print(f"[green]Created session: {session.session_id}[/green]")
        elif subcommand == "switch" and len(args) > 2:
            session_id = args[2]
            session = self.session_manager.load_session(session_id)
            if session is None:
                console.print(f"[yellow]Session not found, creating new.[/yellow]")
                session = self.session_manager.create_session(session_id)
            self.session_manager.set_current_session(session)
            self.session = session
            console.print(f"[green]Switched to session: {session_id}[/green]")
        else:
            console.print("[yellow]Usage: /session [list|new|switch <id>][/yellow]")

    def _do_session_list(self) -> None:
        """List sessions."""
        sessions = self.session_manager.list_sessions()

        if not sessions:
            console.print("[yellow]No sessions found.[/yellow]")
            return

        table = Table(title="Sessions")
        table.add_column("ID", style="cyan")
        table.add_column("Messages", style="magenta")
        table.add_column("Updated", style="yellow")

        for session in sessions:
            marker = " *" if session["session_id"] == self.session.session_id else ""
            table.add_row(
                session["session_id"] + marker,
                str(session["message_count"]),
                session.get("updated_at", "")[:19],
            )

        console.print(table)

    def _do_config(self, args: list[str]) -> None:
        """Handle /config command.

        Args:
            args: Command arguments.
        """
        if len(args) <= 1:
            self._do_config_show()
            return

        subcommand = args[1]

        if subcommand == "get" and len(args) > 2:
            key = args[2]
            value = self.config_manager.get(key)
            console.print(f"[cyan]{key}:[/cyan] {value}" if value else f"[yellow]Key '{key}' not found.[/yellow]")
        elif subcommand == "set" and len(args) > 3:
            key = args[2]
            value = args[3]
            self.config_manager.set(key, value)
            console.print(f"[green]Set {key} = {value}[/green]")
        elif subcommand == "list":
            self._do_config_show()
        else:
            console.print("[yellow]Usage: /config [get <key>|set <key> <value>|list][/yellow]")

    def _do_config_show(self) -> None:
        """Show configuration."""
        config = self.config_manager.config
        console.print(f"[cyan]API Key:[/cyan] {'*' * 8}{config.api_key[-4:] if config.api_key else ''}")
        console.print(f"[cyan]Base URL:[/cyan] {config.base_url}")
        console.print(f"[cyan]Model:[/cyan] {config.model}")
        console.print(f"[cyan]Timeout:[/cyan] {config.timeout_ms}ms")
        console.print(f"[cyan]Tools:[/cyan] {', '.join(config.tools_enabled)}")

    def _do_grep(self, args: list[str]) -> None:
        """Handle /grep command.

        Args:
            args: Command arguments.
        """
        if len(args) < 2:
            console.print("[yellow]Usage: /grep <pattern> [path] [file_pattern][/yellow]")
            return

        from src.tools.grep import grep

        pattern = args[1]
        path = args[2] if len(args) > 2 else "."
        file_pattern = args[3] if len(args) > 3 else "*"

        result = grep(pattern, path, file_pattern)
        console.print(result)

    def _do_tools(self) -> None:
        """Handle /tools command."""
        tools = self.tool_registry.list_tools()

        if not tools:
            console.print("[yellow]No tools available.[/yellow]")
            return

        table = Table(title="Available Tools")
        table.add_column("Name", style="cyan")
        table.add_column("Description", style="white")
        table.add_column("Status", style="magenta")

        for tool in tools:
            status = "[green]enabled[/green]" if tool["enabled"] else "[red]disabled[/red]"
            table.add_row(tool["name"], tool["description"], status)

        console.print(table)

    def _do_history(self, args: list[str]) -> None:
        """Handle /history command.

        Args:
            args: Command arguments.
        """
        limit = 10
        if len(args) > 1 and args[1] == "--limit":
            if len(args) > 2:
                try:
                    limit = int(args[2])
                except ValueError:
                    console.print("[yellow]Invalid limit value.[/yellow]")
                    return

        messages = self.session.messages[-limit:]
        if not messages:
            console.print("[yellow]No history.[/yellow]")
            return

        for i, msg in enumerate(messages):
            role = msg.get("role", "unknown")
            content = msg.get("content", "")[:100]

            if role == "user":
                console.print(f"[bold blue]User:[/bold blue] {content}")
            elif role == "assistant":
                console.print(f"[bold green]Assistant:[/bold green] {content}")
            elif role == "tool":
                console.print(f"[bold yellow]Tool:[/bold yellow] {content}")

            if i < len(messages) - 1:
                console.print()

    def _do_context(self) -> None:
        """Handle /context command."""
        messages = self.session.messages
        total_tokens = sum(len(str(m)) // 4 for m in messages)  # Rough estimate
        max_tokens = 128000  # GLM-5 context limit

        console.print(f"[cyan]Message count:[/cyan] {len(messages)}")
        console.print(f"[cyan]Estimated tokens:[/cyan] ~{total_tokens}")
        console.print(f"[cyan]Context limit:[/cyan] {max_tokens}")
        console.print(f"[cyan]Usage:[/cyan] {total_tokens * 100 // max_tokens}%")

    def _parse_command(self, text: str) -> tuple[Optional[str], list[str]]:
        """Parse command from input.

        Args:
            text: Input text.

        Returns:
            Tuple of (command, args).
        """
        if not text.startswith("/"):
            return None, [text]

        parts = text.split()
        command = parts[0] if parts else None
        return command, parts

    def _handle_command(self, text: str) -> Optional[bool]:
        """Handle slash command.

        Args:
            text: Input text.

        Returns:
            False to exit, None to continue.
        """
        command, args = self._parse_command(text)

        if command is None:
            return None

        handlers = {
            "/exit": self._do_exit,
            "/help": lambda: self._do_help(args),
            "/clear": lambda: self._do_clear(),
            "/session": lambda: self._do_session(args),
            "/config": lambda: self._do_config(args),
            "/grep": lambda: self._do_grep(args),
            "/tools": lambda: self._do_tools(),
            "/history": lambda: self._do_history(args),
            "/context": lambda: self._do_context(),
        }

        handler = handlers.get(command)
        if handler:
            result = handler()
            if command == "/exit":
                return False
            return None

        console.print(f"[yellow]Unknown command: {command}[/yellow]")
        console.print("Type /help for available commands")
        return None

    def _chat(self, message: str) -> None:
        """Send message to AI.

        Args:
            message: User message.
        """
        self.session.add_message("user", message)
        tools = self.tool_registry.get_enabled_tools()

        try:
            response = self.provider.chat(
                messages=self.session.messages,
                tools=tools if tools else None,
                stream=True,
            )

            if isinstance(response, Generator):
                # Streaming
                content = ""
                console.print("[bold blue]Assistant:[/bold blue] ", end="")
                for chunk in response:
                    if "choices" in chunk and chunk["choices"]:
                        delta = chunk["choices"][0].get("delta", {})
                        part = delta.get("content") or delta.get("reasoning_content", "")
                        if part:
                            content += part
                            console.print(part, end="")
                console.print()

                self.session.add_message("assistant", content)
            else:
                # Sync
                if "choices" in response and response["choices"]:
                    # GLM may return reasoning_content or content
                    message = response["choices"][0]["message"]
                    content = message.get("content") or message.get("reasoning_content", "")
                    console.print(f"[bold blue]Assistant:[/bold blue] {content}")
                    self.session.add_message("assistant", content)

                    # Handle tool calls
                    tool_calls = response["choices"][0]["message"].get("tool_calls", [])
                    for tool_call in tool_calls:
                        func = tool_call.get("function", {})
                        name = func.get("name")
                        args_str = func.get("arguments", "{}")

                        import json
                        try:
                            args = json.loads(args_str)
                        except json.JSONDecodeError:
                            args = {}

                        console.print(f"[yellow]Tool: {name}[/yellow]")

                        result = self.tool_registry.execute(name, args)
                        console.print(f"[dim]{result}[/dim]")

                        self.session.add_tool_result(tool_call["id"], result)

                        # Continue with tool result (streaming)
                        response = self.provider.chat(
                            messages=self.session.messages,
                            tools=tools if tools else None,
                            stream=True,
                        )

                        if isinstance(response, Generator):
                            content = ""
                            console.print("[bold blue]Assistant:[/bold blue] ", end="")
                            for chunk in response:
                                if "choices" in chunk and chunk["choices"]:
                                    delta = chunk["choices"][0].get("delta", {})
                                    part = delta.get("content") or delta.get("reasoning_content", "")
                                    if part:
                                        content += part
                                        console.print(part, end="")
                            console.print()
                            self.session.add_message("assistant", content)
                        elif "choices" in response and response["choices"]:
                            # GLM may return reasoning_content or content
                            message = response["choices"][0]["message"]
                            content = message.get("content") or message.get("reasoning_content", "")
                            console.print(f"[bold blue]Assistant:[/bold blue] {content}")
                            self.session.add_message("assistant", content)

            self.session_manager.save_current_session()

        except Exception as e:
            console.print(f"[bold red]Error:[/bold red] {e}")

    def run(self) -> None:
        """Run REPL."""
        self._print_welcome()

        while True:
            try:
                text = self.prompt_session.prompt("> ")

                if not text.strip():
                    continue

                # Handle commands
                result = self._handle_command(text)
                if result is False:
                    break

                # If not a command, chat
                if text.strip():
                    self._chat(text)

            except KeyboardInterrupt:
                console.print("\n[yellow]Type /exit to quit.[/yellow]")
                continue
            except EOFError:
                break


def repl() -> None:
    """Start REPL."""
    repl_instance = REPL()
    repl_instance.run()
