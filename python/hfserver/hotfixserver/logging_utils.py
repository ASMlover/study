# -*- coding: UTF-8 -*-
from __future__ import annotations

"""Logging helpers with colored console output."""

import logging
import os
import sys
from pathlib import Path

from .constants import LOG_COLORS, LOGO, WINDOWS_ANSI_FLAG


class ColorFormatter(logging.Formatter):
    """Formatter that applies ANSI colors based on log level."""
    RESET = "\x1b[0m"

    def format(self, record: logging.LogRecord) -> str:
        base = super().format(record)
        color = LOG_COLORS.get(record.levelname, "")
        if not color:
            return base
        return f"{color}{base}{self.RESET}"


def enable_windows_ansi() -> None:
    """Enable ANSI color support for Windows consoles."""
    if os.name != "nt":
        return
    try:
        import ctypes

        kernel32 = ctypes.windll.kernel32
        handle = kernel32.GetStdHandle(-11)
        mode = ctypes.c_uint()
        if kernel32.GetConsoleMode(handle, ctypes.byref(mode)) == 0:
            return
        mode.value |= WINDOWS_ANSI_FLAG
        kernel32.SetConsoleMode(handle, mode)
    except Exception:
        return


def setup_logging(level: int, log_file: Path) -> logging.Logger:
    """Configure console + file logging for the hotfix server."""
    enable_windows_ansi()
    logger = logging.getLogger("hotfix")
    logger.setLevel(level)
    logger.handlers = []

    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(level)
    console_handler.setFormatter(ColorFormatter("%(asctime)s [%(levelname)s] %(message)s"))
    logger.addHandler(console_handler)

    log_file.parent.mkdir(parents=True, exist_ok=True)
    file_handler = logging.FileHandler(log_file, encoding="utf-8")
    file_handler.setLevel(level)
    file_handler.setFormatter(
        logging.Formatter("%(asctime)s [%(levelname)s] %(message)s")
    )
    logger.addHandler(file_handler)
    return logger


def print_logo() -> None:
    """Print the startup ASCII logo."""
    print(LOGO)
