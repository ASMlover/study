from __future__ import annotations

import os
import re
from typing import Literal

AnsiThemeName = Literal["dark", "light"]
RESET = "\x1b[0m"

THEMES: dict[str, dict[str, str]] = {
    "dark": {
        "cyan": "\x1b[36m",
        "blue": "\x1b[34m",
        "green": "\x1b[32m",
        "yellow": "\x1b[33m",
        "magenta": "\x1b[35m",
        "red": "\x1b[31m",
        "gray": "\x1b[90m",
        "code": "\x1b[38;5;81m",
    },
    "light": {
        "cyan": "\x1b[36m",
        "blue": "\x1b[34m",
        "green": "\x1b[32m",
        "yellow": "\x1b[38;5;130m",
        "magenta": "\x1b[35m",
        "red": "\x1b[31m",
        "gray": "\x1b[90m",
        "code": "\x1b[38;5;24m",
    },
}

_active_theme: AnsiThemeName = "dark"


def _wrap(code: str, text: str) -> str:
    return f"{code}{text}{RESET}"


def detect_ansi_theme(env: dict[str, str] | None = None) -> AnsiThemeName:
    env_map = env if env is not None else os.environ
    configured = env_map.get("MINICLI4_THEME", "").strip().lower()
    if configured in ("dark", "light"):
        return configured  # type: ignore[return-value]

    colorfgbg = env_map.get("COLORFGBG", "")
    if colorfgbg:
        parts: list[int] = []
        for raw in colorfgbg.split(";"):
            try:
                parts.append(int(raw.strip()))
            except ValueError:
                continue
        if parts:
            return "dark" if parts[-1] <= 6 else "light"
    return "dark"


def set_ansi_theme(theme: AnsiThemeName) -> None:
    global _active_theme
    _active_theme = theme


def get_ansi_theme() -> AnsiThemeName:
    return _active_theme


def strip_ansi(text: str) -> str:
    return re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", text)


class ansi:
    @staticmethod
    def bold(text: str) -> str:
        return _wrap("\x1b[1m", text)

    @staticmethod
    def dim(text: str) -> str:
        return _wrap("\x1b[2m", text)

    @staticmethod
    def italic(text: str) -> str:
        return _wrap("\x1b[3m", text)

    @staticmethod
    def cyan(text: str) -> str:
        return _wrap(THEMES[_active_theme]["cyan"], text)

    @staticmethod
    def blue(text: str) -> str:
        return _wrap(THEMES[_active_theme]["blue"], text)

    @staticmethod
    def green(text: str) -> str:
        return _wrap(THEMES[_active_theme]["green"], text)

    @staticmethod
    def yellow(text: str) -> str:
        return _wrap(THEMES[_active_theme]["yellow"], text)

    @staticmethod
    def magenta(text: str) -> str:
        return _wrap(THEMES[_active_theme]["magenta"], text)

    @staticmethod
    def red(text: str) -> str:
        return _wrap(THEMES[_active_theme]["red"], text)

    @staticmethod
    def gray(text: str) -> str:
        return _wrap(THEMES[_active_theme]["gray"], text)

    @staticmethod
    def code(text: str) -> str:
        return _wrap(THEMES[_active_theme]["code"], text)
