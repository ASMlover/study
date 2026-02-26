from __future__ import annotations

from dataclasses import asdict
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Callable
from typing import TextIO
from typing import cast

from .agent import AgentCallbacks, run_multi_agent_round
from .ansi import set_ansi_theme
from .commands import COMMANDS, CommandContext, run_slash
from .completion import CompletionContext, complete
from .config import RuntimeConfig, load_config
from .provider import GLMProvider
from .session import SessionMessage, SessionRecord, SessionStore
from .tools import ToolRegistry
from .tui import PaneState, TwoPaneTui

try:
    from prompt_toolkit import PromptSession
    from prompt_toolkit.completion import Completer, Completion
    from prompt_toolkit.key_binding import KeyBindings
except ImportError:  # pragma: no cover
    PromptSession = None  # type: ignore[assignment]
    Completer = object  # type: ignore[assignment]
    Completion = None  # type: ignore[assignment]
    KeyBindings = None  # type: ignore[assignment]


def _now_iso() -> str:
    return datetime.now(tz=timezone.utc).isoformat().replace("+00:00", "Z")


@dataclass(slots=True)
class ReplRuntime:
    input: TextIO
    output: TextIO
    stdout: Callable[[str], None]
    stderr: Callable[[str], None]


class SlashCompleter(Completer):
    def __init__(self, callback: Callable[[str], list[str]]) -> None:
        self.callback = callback

    def get_completions(self, document, _complete_event):  # type: ignore[no-untyped-def]
        text = document.text_before_cursor
        options = self.callback(text)
        token = "" if text.endswith(" ") else ((text.split()[-1] if text.split() else text))
        for item in options:
            yield Completion(item, start_position=-len(token))


def _handle_ctrl_l(is_tty: bool, tui: TwoPaneTui, pane: PaneState, push_line: Callable[[str], None], flush: Callable[[], None]) -> None:
    if is_tty:
        tui.print_event("[ui] append-only mode; showing latest status")
        tui.print_status(pane)
        flush()
        return
    push_line("[ui] append-only mode; showing latest status")


def _handle_f1(
    is_tty: bool,
    tui: TwoPaneTui,
    push_lines: Callable[[list[str]], None],
    help_lines_provider: Callable[[], list[str]],
    flush: Callable[[], None],
) -> None:
    if is_tty:
        tui.announce_input("/help")
    push_lines(help_lines_provider())
    flush()


def _handle_escape(push_line: Callable[[str], None]) -> None:
    push_line("[completion] dismissed")


def start_repl(runtime: ReplRuntime, project_root: Path | None = None) -> None:
    root = project_root or Path.cwd()
    loaded = load_config(root)
    config = loaded.config
    set_ansi_theme(config.theme)
    sessions = SessionStore(loaded.state_dir)
    session = sessions.current()
    pending_approval = ""
    provider = GLMProvider(config)
    tools = ToolRegistry(root, config)
    is_tty = hasattr(runtime.output, "isatty") and runtime.output.isatty()
    tui = TwoPaneTui(runtime.output, is_tty=is_tty)
    pane = PaneState(
        session_id=session.session_id,
        model=config.model,
        stage="idle",
        tools=",".join(tools.list_tool_names()),
        mode=config.safe_mode,
        pending_approval="none",
    )

    def _flush() -> None:
        flush = getattr(runtime.output, "flush", None)
        if callable(flush):
            flush()

    def _stdout(text: str) -> None:
        runtime.stdout(text)
        _flush()

    def _stderr(text: str) -> None:
        runtime.stderr(text)
        _flush()

    def push_line(line: str) -> None:
        if is_tty:
            tui.print_event(line)
            _flush()
            return
        _stdout(f"{line}\n")

    def push_lines(lines: list[str]) -> None:
        if not lines:
            return
        if is_tty:
            for line in lines:
                tui.print_event(line)
            _flush()
            return
        _stdout(f"{'\n'.join(lines)}\n")

    _stdout("MiniCLI4 Python REPL\n")
    _stdout("Type /help for commands.\n")
    if is_tty:
        tui.apply_config(config)
        tui.start(pane)
        _flush()

    def completion_items(prefix: str) -> list[str]:
        return complete(
            prefix,
            CompletionContext(
                command_names=COMMANDS,
                session_ids=[s.session_id for s in sessions.list()],
                config_keys=list(asdict(config).keys()),
                tool_names=tools.list_tool_names(),
                project_root=root,
            ),
        )

    prompt_session = None
    key_bindings = None
    if is_tty and KeyBindings is not None:
        key_bindings = KeyBindings()

        def help_lines_provider() -> list[str]:
            help_lines: list[str] = []
            run_slash(
                CommandContext(
                    project_root=root,
                    config=config,
                    session=session,
                    sessions=sessions,
                    tools=tools,
                    set_config=lambda _next: None,
                    set_session=lambda _next: None,
                    out=lambda line: help_lines.append(line),
                    set_pending_approval=lambda _value: None,
                    clear_pending_approval=lambda: None,
                    pending_approval=lambda: pending_approval,
                ),
                "/help",
            )
            return help_lines

        @key_bindings.add("c-l")
        def _on_ctrl_l(_event) -> None:  # type: ignore[no-untyped-def]
            _handle_ctrl_l(is_tty, tui, pane, push_line, _flush)

        @key_bindings.add("f1")
        def _on_f1(_event) -> None:  # type: ignore[no-untyped-def]
            _handle_f1(is_tty, tui, push_lines, help_lines_provider, _flush)

        @key_bindings.add("escape")
        def _on_escape(_event) -> None:  # type: ignore[no-untyped-def]
            _handle_escape(push_line)

    if is_tty and PromptSession is not None:
        prompt_session = PromptSession(completer=SlashCompleter(completion_items), key_bindings=key_bindings)

    while True:
        if prompt_session is not None:
            try:
                input_text = cast(str, prompt_session.prompt("> ")).strip()
            except EOFError:
                break
            except KeyboardInterrupt:
                _stdout("\nInterrupted. Type /exit to quit.\n")
                continue
        else:
            _stdout("> ")
            line = runtime.input.readline()
            if line == "":
                break
            input_text = line.strip()
        if not input_text:
            continue
        if is_tty:
            tui.announce_input(input_text)

        command_lines: list[str] = []

        def set_config(next_cfg: RuntimeConfig) -> None:
            nonlocal config, tools
            config = next_cfg
            set_ansi_theme(config.theme)
            tools = ToolRegistry(root, config)
            pane.model = config.model
            pane.mode = config.safe_mode
            pane.tools = ",".join(tools.list_tool_names())
            tui.apply_config(config)
            tui.update_status(pane)

        def set_session(next_session: SessionRecord) -> None:
            nonlocal session
            session = next_session
            pane.session_id = next_session.session_id
            tui.update_status(pane)

        def _set_pending(value: str) -> None:
            nonlocal pending_approval
            pending_approval = value
            pane.pending_approval = value

        def _clear_pending() -> None:
            nonlocal pending_approval
            pending_approval = ""
            pane.pending_approval = "none"

        if input_text.startswith("/"):
            keep = run_slash(
                CommandContext(
                    project_root=root,
                    config=config,
                    session=session,
                    sessions=sessions,
                    tools=tools,
                    set_config=set_config,
                    set_session=set_session,
                    out=lambda t: command_lines.append(t),
                    set_pending_approval=lambda v: _set_pending(v),
                    clear_pending_approval=lambda: _clear_pending(),
                    pending_approval=lambda: pending_approval,
                ),
                input_text,
            )
            push_lines(command_lines)
            tui.update_status(pane)
            if not keep:
                break
            continue

        session.messages.append(SessionMessage(role="user", content=input_text, created_at=_now_iso()))
        sessions.save(session)
        pane.stage = "planner"
        tui.update_status(pane)
        if is_tty:
            tui.start_thinking()

        stream_started = False

        def on_delta(chunk: str) -> None:
            nonlocal stream_started
            if not chunk:
                return
            if not stream_started:
                stream_started = True
                if is_tty:
                    tui.stop_thinking()
                    tui.start_assistant_stream()
            if is_tty:
                tui.append_assistant_chunk(chunk)
                _flush()
            else:
                _stdout(chunk)

        try:
            result = run_multi_agent_round(
                input_text,
                session.messages,
                provider,
                tools,
                config,
                AgentCallbacks(
                    on_stage=lambda stage, _detail: _set_stage(stage, pane, tui),
                    on_tool=lambda _name, _output: None,
                    on_delta=on_delta,
                ),
            )
            if stream_started:
                if is_tty:
                    tui.stop_thinking()
                    tui.end_assistant_stream()
                    _flush()
                else:
                    _stdout("\n")
            else:
                if is_tty:
                    tui.stop_thinking()
                    tui.start_assistant_stream()
                    tui.append_assistant_chunk(result.final)
                    tui.end_assistant_stream()
                    _flush()
                else:
                    _stdout(f"{result.final}\n")
            session.messages.append(SessionMessage(role="assistant", content=result.final, created_at=_now_iso()))
            sessions.save(session)
            pane.stage = "idle"
            tui.update_status(pane)
        except Exception as err:  # noqa: BLE001
            if is_tty:
                tui.stop_thinking()
            _stderr(f"[error] {err}\n")
            pane.stage = "error"
            tui.update_status(pane)
    _stdout("Bye.\n")


def _set_stage(stage: str, pane: PaneState, tui: TwoPaneTui) -> None:
    pane.stage = stage
    tui.update_status(pane)


def get_completions(prefix: str, project_root: Path, config: RuntimeConfig, sessions: SessionStore, tools: ToolRegistry) -> list[str]:
    return complete(
        prefix,
        CompletionContext(
            command_names=COMMANDS,
            session_ids=[s.session_id for s in sessions.list()],
            config_keys=list(asdict(config).keys()),
            tool_names=tools.list_tool_names(),
            project_root=project_root,
        ),
    )
