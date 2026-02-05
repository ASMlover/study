# -*- coding: UTF-8 -*-
from __future__ import annotations

"""Configuration parsing and live reload helpers."""

import argparse
import json
import logging
from pathlib import Path
from typing import Any

from .constants import (
    DEFAULT_CONFIG_PATH,
    DEFAULT_HOST,
    DEFAULT_LOG_FILE,
    DEFAULT_MAX_FILES,
    DEFAULT_PORT,
    DEFAULT_UPLOAD_MAX_FILE_BYTES,
    DEFAULT_UPLOAD_MAX_REQUEST_BYTES,
)
from .models import ModeLimit, ServerConfig


def load_config(path: Path) -> dict[str, Any]:
    """Load JSON config if present; return empty dict on error."""
    if not path.exists():
        return {}
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}


def parse_log_level(value: str | None, default: int) -> int:
    """Parse a log level string with a safe fallback."""
    if not value:
        return default
    upper = str(value).strip().upper()
    level = logging.getLevelName(upper)
    if isinstance(level, int):
        return level
    return default


def parse_positive_int(value: Any, default: int) -> int:
    """Parse a positive integer with a safe fallback."""
    try:
        parsed = int(value)
    except (TypeError, ValueError):
        return default
    if parsed <= 0:
        return default
    return parsed


def parse_args() -> ServerConfig:
    """Parse CLI args and merge with config.json."""
    parser = argparse.ArgumentParser(description="HotfixServer file manager")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--upload-dir", default="uploads")
    parser.add_argument("--static-dir", default="static")
    parser.add_argument("--max-files", type=int, default=None)
    parser.add_argument("--trust-x-forwarded-for", action="store_true")
    parser.add_argument("--config", default=DEFAULT_CONFIG_PATH)
    parser.add_argument("--log-level", default=None)
    parser.add_argument("--log-file", default=None)
    args = parser.parse_args()

    config_path = Path(str(args.config))
    payload = load_config(config_path)

    def pick(key: str, default: Any) -> Any:
        """Read a config key with a fallback."""
        value = payload.get(key, default)
        return default if value is None else value

    max_files = int(
        args.max_files
        if args.max_files is not None
        else pick("max_files_per_mode", DEFAULT_MAX_FILES)
    )
    upload_max_file_bytes = parse_positive_int(
        pick("upload_max_file_bytes", DEFAULT_UPLOAD_MAX_FILE_BYTES),
        DEFAULT_UPLOAD_MAX_FILE_BYTES,
    )
    upload_max_request_bytes = parse_positive_int(
        pick("upload_max_request_bytes", DEFAULT_UPLOAD_MAX_REQUEST_BYTES),
        DEFAULT_UPLOAD_MAX_REQUEST_BYTES,
    )

    whitelist: set[str] = set()
    file_whitelist = payload.get("delete_allow", [])
    if isinstance(file_whitelist, list):
        for ip in file_whitelist:
            if isinstance(ip, str) and ip.strip():
                whitelist.add(ip.strip())
    if not whitelist:
        whitelist.update({"127.0.0.1", "::1"})

    mode_limits: dict[str, ModeLimit] = {}
    raw_limits = payload.get("mode_limits", {})
    if isinstance(raw_limits, dict):
        for key, entry in raw_limits.items():
            if not isinstance(entry, dict):
                continue
            try:
                mode_limits[key] = ModeLimit(
                    max_files=int(entry.get("max_files", max_files)),
                )
            except (TypeError, ValueError):
                continue

    log_level = parse_log_level(args.log_level, logging.INFO)
    log_level = parse_log_level(str(payload.get("log_level", "")), log_level)

    log_file = args.log_file if args.log_file is not None else pick(
        "log_file", DEFAULT_LOG_FILE
    )

    return ServerConfig(
        host=str(pick("host", args.host)),
        port=int(pick("port", args.port)),
        upload_dir=Path(str(pick("upload_dir", args.upload_dir))),
        static_dir=Path(str(pick("static_dir", args.static_dir))),
        max_files_per_mode=max_files,
        upload_max_file_bytes=upload_max_file_bytes,
        upload_max_request_bytes=upload_max_request_bytes,
        mode_limits=mode_limits,
        delete_allow=whitelist,
        trust_x_forwarded_for=bool(
            pick("trust_x_forwarded_for", args.trust_x_forwarded_for)
        ),
        config_path=config_path,
        log_level=log_level,
        log_file=Path(str(log_file)),
    )


def apply_config_payload(
    config: ServerConfig, payload: dict[str, Any], logger: logging.Logger
) -> None:
    """Apply config.json values to a live ServerConfig instance."""
    file_whitelist = payload.get("delete_allow", [])
    whitelist: set[str] = set()
    if isinstance(file_whitelist, list):
        for ip in file_whitelist:
            if isinstance(ip, str) and ip.strip():
                whitelist.add(ip.strip())
    if whitelist:
        config.delete_allow = whitelist

    if "max_files_per_mode" in payload:
        try:
            config.max_files_per_mode = int(payload["max_files_per_mode"])
        except (TypeError, ValueError):
            pass
    if "upload_max_file_bytes" in payload:
        config.upload_max_file_bytes = parse_positive_int(
            payload["upload_max_file_bytes"], config.upload_max_file_bytes
        )
    if "upload_max_request_bytes" in payload:
        config.upload_max_request_bytes = parse_positive_int(
            payload["upload_max_request_bytes"], config.upload_max_request_bytes
        )

    raw_limits = payload.get("mode_limits", {})
    if isinstance(raw_limits, dict):
        mode_limits: dict[str, ModeLimit] = {}
        for key, entry in raw_limits.items():
            if not isinstance(entry, dict):
                continue
            try:
                mode_limits[key] = ModeLimit(
                    max_files=int(entry.get("max_files", config.max_files_per_mode)),
                )
            except (TypeError, ValueError):
                continue
        if mode_limits:
            config.mode_limits = mode_limits

    if "trust_x_forwarded_for" in payload:
        config.trust_x_forwarded_for = bool(payload["trust_x_forwarded_for"])

    new_level = parse_log_level(str(payload.get("log_level", "")), config.log_level)
    if new_level != config.log_level:
        config.log_level = new_level
        logger.setLevel(new_level)
        for handler in logger.handlers:
            handler.setLevel(new_level)
        logger.info("Log level updated to %s", logging.getLevelName(new_level))
