"""Interactive REPL for MiniCLI3."""

from __future__ import annotations

from collections.abc import Generator
import json
from pathlib import Path
from typing import Any

from prompt_toolkit import PromptSession
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from prompt_toolkit.completion import Completer, Completion
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.keys import Keys
from rich.console import Console
from rich.panel import Panel

from minicli3.commands.builtin import build_registry
from minicli3.completion.engine import command_completions
from minicli3.config import AppConfig, ConfigManager
from minicli3.provider.glm import GLMProvider, ProviderError
from minicli3.session import SessionManager
from minicli3.tools.registry import ToolRegistry


class SlashCompleter(Completer):
    def __init__(self, app: "MiniCLIApp"):
        self.app = app

    def get_completions(self, document, complete_event):
        text = document.text_before_cursor
        items = command_completions(
            text,
            self.app.commands,
            session_ids=[row["session_id"] for row in self.app.session_manager.list()],
            config_keys=list(self.app.config.__dict__.keys()),
            tool_names=[row["name"] for row in self.app.tools.list_tools()],
            project_root=self.app.project_root,
        )
        if not items:
            return
        token = ""
        if text and not text.endswith(" "):
            token = text.split()[-1]
        start_position = -len(token)
        for item in items:
            yield Completion(item, start_position=start_position)


class MiniCLIApp:
    def __init__(self, project_root: Path | None = None, interactive: bool = True):
        self.console = Console()
        self.project_root = project_root or Path.cwd()
        self.config_manager = ConfigManager(self.project_root)
        self.config: AppConfig = self.config_manager.config
        self.session_manager = SessionManager(self.project_root)
        self.session = self.session_manager.current()
        self.tools = ToolRegistry(self.project_root, self.config)
        self.commands = build_registry()
        self._provider: GLMProvider | None = None

        self.prompt: PromptSession | None = None
        if interactive:
            bindings = KeyBindings()
            bindings.add(Keys.Tab)(self._on_tab)
            bindings.add(Keys.ControlC)(self._on_interrupt)
            self.prompt = PromptSession(
                completer=SlashCompleter(self),
                auto_suggest=AutoSuggestFromHistory(),
                key_bindings=bindings,
                complete_while_typing=True,
            )

    def _on_tab(self, event) -> None:
        buf = event.app.current_buffer
        new_text = self.apply_tab_completion(buf.text)
        if new_text is not None:
            buf.text = new_text
            buf.cursor_position = len(new_text)
            event.handled = True
            return
        buf.start_completion(select_first=True)
        event.handled = True

    def _on_interrupt(self, event) -> None:
        event.app.current_buffer.text = ""
        self.console.print("\n[yellow]Interrupted. Type /exit to quit.[/yellow]")

    def _get_provider(self) -> GLMProvider:
        if self._provider is None:
            self._provider = GLMProvider(
                api_key=self.config.api_key,
                base_url=self.config.base_url,
                model=self.config.model,
                timeout_ms=self.config.timeout_ms,
                max_retries=self.config.max_retries,
            )
        return self._provider

    def print_welcome(self) -> None:
        self.console.print(
            Panel(
                "[bold cyan]MiniCLI3[/bold cyan] - standalone GLM-5 agent CLI\nType /help for commands.",
                title="Welcome",
                border_style="cyan",
            )
        )

    def handle_line(self, text: str) -> bool:
        """Handle one user line. Returns False to exit."""
        if text.startswith("/"):
            parts = text.strip().split()
            cmd = parts[0]
            spec = self.commands.get(cmd)
            if spec is None:
                self.console.print(f"[yellow]Unknown command: {cmd}[/yellow]")
                return True
            result = spec.handler(self, parts)
            self.session_manager.save()
            if result is False:
                return False
            return True

        self._chat_stream(text)
        return True

    def _chat_stream(self, text: str) -> None:
        self.session.add_message("user", text)
        try:
            provider = self._get_provider()
            base_messages = [m.to_dict() for m in self.session.messages]
            had_tool_calls = False

            # Phase 1: non-stream planning for tool calls.
            planning_response = provider.chat(
                messages=base_messages,
                tools=self.tools.tool_definitions(),
                stream=False,
                temperature=self.config.temperature,
                max_tokens=self.config.max_tokens,
            )
            if isinstance(planning_response, Generator):
                raise ProviderError("unexpected streaming response in planning phase")

            while True:
                message, tool_calls = self._extract_message_and_tool_calls(planning_response)
                if not tool_calls:
                    break

                had_tool_calls = True
                self.session.add_message("assistant", message, tool_calls=tool_calls)
                for tool_call in tool_calls:
                    fn = tool_call.get("function", {})
                    name = fn.get("name", "")
                    args = self._parse_tool_arguments(fn.get("arguments"))
                    result = self.tools.execute(name, args)
                    call_id = tool_call.get("id", "tool_call")
                    self.session.add_tool_result(call_id, result)
                    self.console.print(f"[yellow]tool {name}[/yellow]: [dim]{result[:200]}[/dim]")

                planning_response = provider.chat(
                    messages=[m.to_dict() for m in self.session.messages],
                    tools=self.tools.tool_definitions(),
                    stream=False,
                    temperature=self.config.temperature,
                    max_tokens=self.config.max_tokens,
                )
                if isinstance(planning_response, Generator):
                    raise ProviderError("unexpected streaming response in planning loop")

            # Phase 2: stream final user-facing answer.
            stream_messages = [m.to_dict() for m in self.session.messages] if had_tool_calls else base_messages
            self.console.print("[bold blue]assistant:[/bold blue] ", end="")
            stream = provider.chat(
                messages=stream_messages,
                tools=None,
                stream=self.config.stream,
                temperature=self.config.temperature,
                max_tokens=self.config.max_tokens,
            )
            content = ""
            if isinstance(stream, Generator):
                for chunk in stream:
                    part = self._chunk_content(chunk)
                    if part:
                        content += part
                        self.console.print(part, end="")
                self.console.print()
            else:
                content = self._extract_content(stream)
                self.console.print(content)
            self.session.add_message("assistant", content)
            self.session_manager.save()
        except ProviderError as exc:
            self.console.print(f"\n[bold red]Provider error:[/bold red] {exc}")
        except Exception as exc:  # pragma: no cover - defensive path
            self.console.print(f"\n[bold red]Error:[/bold red] {exc}")

    def apply_tab_completion(self, text: str) -> str | None:
        """Apply slash completion directly to input text.

        Returns:
            Updated text when completion is applied, otherwise None.
        """
        if not text.startswith("/"):
            return None

        items = command_completions(
            text,
            self.commands,
            session_ids=[row["session_id"] for row in self.session_manager.list()],
            config_keys=list(self.config.__dict__.keys()),
            tool_names=[row["name"] for row in self.tools.list_tools()],
            project_root=self.project_root,
        )
        if not items:
            return None

        # Replace only the current token; prefer first deterministic candidate.
        token = ""
        if text and not text.endswith(" "):
            token = text.split()[-1]
        replacement = items[0]
        if token:
            prefix = text[: len(text) - len(token)]
            completed = prefix + replacement
        else:
            completed = text + replacement

        if completed.count(" ") == 0 and not completed.endswith(" "):
            completed += " "
        return completed

    @staticmethod
    def _chunk_content(chunk: dict[str, Any]) -> str:
        choices = chunk.get("choices", [])
        if not choices:
            return ""
        delta = choices[0].get("delta", {})
        return delta.get("content") or delta.get("reasoning_content") or ""

    @staticmethod
    def _extract_content(payload: dict[str, Any]) -> str:
        choices = payload.get("choices", [])
        if not choices:
            return ""
        msg = choices[0].get("message", {})
        return msg.get("content") or msg.get("reasoning_content") or ""

    @staticmethod
    def _extract_message_and_tool_calls(payload: dict[str, Any]) -> tuple[str, list[dict[str, Any]]]:
        choices = payload.get("choices", [])
        if not choices:
            return "", []
        msg = choices[0].get("message", {})
        content = msg.get("content") or msg.get("reasoning_content") or ""
        tool_calls = msg.get("tool_calls", []) or []
        return content, tool_calls

    @staticmethod
    def _parse_tool_arguments(raw: Any) -> dict[str, Any]:
        if isinstance(raw, dict):
            return raw
        if isinstance(raw, str):
            try:
                data = json.loads(raw)
                if isinstance(data, dict):
                    return data
            except json.JSONDecodeError:
                return {}
        return {}

    def run(self) -> None:
        if self.prompt is None:
            raise RuntimeError("REPL prompt is not initialized (interactive=False).")
        self.print_welcome()
        while True:
            try:
                line = self.prompt.prompt("> ")
                if not line.strip():
                    continue
                keep_running = self.handle_line(line.strip())
                if not keep_running:
                    break
            except KeyboardInterrupt:
                self.console.print("\n[yellow]Use /exit to quit.[/yellow]")
            except EOFError:
                self.console.print()
                break


def run_repl() -> None:
    MiniCLIApp().run()
