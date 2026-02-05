# -*- coding: UTF-8 -*-
"""Shared constants to avoid magic numbers and literals."""

from __future__ import annotations


DEFAULT_HOST = "0.0.0.0"
DEFAULT_PORT = 8000
DEFAULT_MAX_FILES = 20
DEFAULT_CONFIG_PATH = "config.json"
DEFAULT_LOG_FILE = "run/hotfix_server.log"

DEFAULT_MAX_HEADER_BYTES = 65536
DEFAULT_READ_CHUNK_SIZE = 1024
REQUEST_HEADER_SEPARATOR = b"\r\n\r\n"
MULTIPART_BOUNDARY_KEY = "boundary="
DEFAULT_UPLOAD_MAX_FILE_BYTES = 10 * 1024 * 1024
DEFAULT_UPLOAD_MAX_REQUEST_BYTES = 100 * 1024 * 1024

WINDOWS_ANSI_FLAG = 0x0004

LOG_COLORS = {
    "DEBUG": "\x1b[38;5;245m",
    "INFO": "\x1b[38;5;45m",
    "WARNING": "\x1b[38;5;214m",
    "ERROR": "\x1b[38;5;196m",
    "CRITICAL": "\x1b[48;5;196m\x1b[38;5;15m",
}

LOGO = (
    "\x1b[38;5;45m  _   _       _   __ _   \n"
    "\x1b[38;5;87m | | | | ___ | |_ / _(_) \n"
    "\x1b[38;5;214m | |_| |/ _ \\| __| |_| | \n"
    "\x1b[38;5;208m |  _  | (_) | |_|  _| | \n"
    "\x1b[38;5;196m |_| |_|\\___/ \\__|_| |_| \n"
    "\x1b[0m"
)
