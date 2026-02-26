from __future__ import annotations

import threading
import time
from dataclasses import dataclass
from typing import TextIO

from .ansi import ansi, get_ansi_theme
from .config import RuntimeConfig


@dataclass(slots=True)
class PaneState:
    session_id: str
    model: str
    stage: str
    tools: str
    mode: str
    pending_approval: str


def _now_time() -> str:
    return time.strftime("%H:%M:%S")


class TwoPaneTui:
    def __init__(self, output: TextIO, is_tty: bool = False) -> None:
        self.output = output
        self.is_tty = is_tty
        self.stream_open = False
        self.motion = "full"
        self.turn = 0
        self.thinking_active = False
        self.thinking_frame = 0
        self.thinking_started_at = 0.0
        self._thinking_thread: threading.Thread | None = None
        self._thinking_stop = threading.Event()
        self._last_status_signature = ""
        self._thinking_stages: list[tuple[str, str, list[str]]] = [
            ("Thinking", "TH", ["collecting context", "reviewing intent", "loading constraints"]),
            ("Reasoning", "RE", ["evaluating options", "checking tradeoffs", "validating assumptions"]),
            ("Drafting", "DR", ["structuring response", "building answer", "assembling details"]),
            ("Refining", "RF", ["improving clarity", "tightening phrasing", "verifying coherence"]),
            ("Finalizing", "FI", ["preparing stream", "readying output", "sending response"]),
        ]

    def start(self, status: PaneState) -> None:
        if not self.is_tty:
            return
        theme = get_ansi_theme()
        self.output.write(f"{ansi.bold(ansi.cyan('MiniCLI4'))} {ansi.gray('Python Agent CLI')}\n")
        self.output.write(
            f"{ansi.gray('Mode:')} {ansi.green(status.mode)}  "
            f"{ansi.gray('Model:')} {ansi.magenta(status.model)}  "
            f"{ansi.gray('Theme:')} {ansi.yellow(theme)}  "
            f"{ansi.gray('Motion:')} {ansi.yellow(self.motion)}  "
            f"{ansi.gray('Session:')} {ansi.cyan(status.session_id)}\n"
        )
        self.output.write(
            f"{ansi.gray('Keys:')} {ansi.blue('Tab')} complete | {ansi.blue('Ctrl+C')} stop | "
            f"{ansi.blue('Ctrl+L')} status | {ansi.blue('F1')} help | {ansi.blue('Esc')} close menu\n"
        )
        self.output.write(f"{ansi.gray('-' * 72)}\n")
        self._last_status_signature = self._signature(status)

    def announce_input(self, input_text: str) -> None:
        if not self.is_tty:
            return
        self.stop_thinking()
        self.turn += 1
        self.end_assistant_stream()
        self.output.write(f"\n{ansi.gray(f'[{_now_time()}]')} {ansi.green(f'[YOU {self.turn:02d}]')} {ansi.bold(input_text)}\n")

    def print_event(self, line: str) -> None:
        if not self.is_tty:
            return
        self.stop_thinking()
        self.end_assistant_stream()
        self.output.write(f"{line}\n")

    def update_status(self, status: PaneState) -> None:
        if self.is_tty:
            self._last_status_signature = self._signature(status)

    def print_status(self, status: PaneState) -> None:
        if not self.is_tty:
            return
        self.stop_thinking()
        self._last_status_signature = self._signature(status)
        self.end_assistant_stream()
        self.output.write(
            f"{ansi.yellow('[STATUS]')} "
            f"{ansi.gray('session=')}{ansi.cyan(status.session_id)} "
            f"{ansi.gray('model=')}{ansi.magenta(status.model)} "
            f"{ansi.gray('stage=')}{ansi.blue(status.stage)} "
            f"{ansi.gray('mode=')}{ansi.green(status.mode)} "
            f"{ansi.gray('approval=')}{ansi.yellow(status.pending_approval)}\n"
        )

    def start_assistant_stream(self) -> None:
        if not self.is_tty:
            return
        self.stop_thinking()
        if self.stream_open:
            return
        self.stream_open = True

    def append_assistant_chunk(self, chunk: str) -> None:
        if not self.is_tty:
            return
        if not self.stream_open:
            self.start_assistant_stream()
        self.output.write(chunk)

    def end_assistant_stream(self) -> None:
        if not self.is_tty or not self.stream_open:
            return
        self.output.write("\n")
        self.stream_open = False

    def start_thinking(self) -> None:
        if not self.is_tty or self.thinking_active:
            return
        self.end_assistant_stream()
        self.thinking_active = True
        self.thinking_frame = 0
        self.thinking_started_at = time.time()
        self._thinking_stop.clear()
        self.output.write(f"\r\x1b[2K{self._render_thinking_frame()}")
        self._thinking_thread = threading.Thread(target=self._thinking_loop, daemon=True)
        self._thinking_thread.start()

    def tick_thinking(self) -> None:
        if not self.is_tty or not self.thinking_active:
            return
        self.thinking_frame += 1
        self.output.write(f"\r\x1b[2K{self._render_thinking_frame()}")

    def stop_thinking(self) -> None:
        if not self.is_tty or not self.thinking_active:
            return
        self._thinking_stop.set()
        if self._thinking_thread is not None:
            self._thinking_thread.join(timeout=0.2)
            self._thinking_thread = None
        self.thinking_active = False
        self.output.write("\r\x1b[2K")

    def apply_config(self, config: RuntimeConfig) -> None:
        self.motion = config.motion

    @staticmethod
    def _signature(status: PaneState) -> str:
        return f"{status.session_id}|{status.model}|{status.stage}|{status.mode}|{status.pending_approval}"

    def _render_thinking_frame(self) -> str:
        elapsed_sec = int(time.time() - self.thinking_started_at)
        mm, ss = elapsed_sec // 60, elapsed_sec % 60
        stage_name, stage_icon, details = self._thinking_stages[(self.thinking_frame // 6) % len(self._thinking_stages)]
        detail = details[self.thinking_frame % len(details)]
        timer = ansi.gray(f"{mm:02d}:{ss:02d}")
        if self.motion == "minimal":
            return f"{ansi.cyan('[MiniCLI4]')} {ansi.magenta(stage_icon)} {ansi.blue(stage_name):<10} {timer} {ansi.gray(detail)}"
        return f"{ansi.cyan('[MiniCLI4]')} {ansi.yellow('*')} {ansi.magenta(stage_icon)} {ansi.blue(stage_name):<10} {timer} {ansi.yellow('*')} {ansi.gray(detail)}"

    def _thinking_loop(self) -> None:
        while not self._thinking_stop.wait(timeout=0.48 if self.motion == "minimal" else 0.16):
            if not self.thinking_active:
                return
            self.thinking_frame += 1
            self.output.write(f"\r\x1b[2K{self._render_thinking_frame()}")
