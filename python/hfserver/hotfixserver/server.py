# -*- coding: UTF-8 -*-
from __future__ import annotations

"""Server bootstrap and main asyncio entrypoint."""

import asyncio
import logging
import time

from .config import apply_config_payload, load_config, parse_args
from .http_server import handle_client
from .logging_utils import print_logo, setup_logging
from .models import ServerState
from .storage import cleanup_removed, load_records, prune_records, save_records


async def maybe_reload_config(
    config, state: ServerState, logger: logging.Logger
) -> None:
    """Reload config.json on modification."""
    now = time.monotonic()
    if now - state.last_config_check < 1.0:
        return
    state.last_config_check = now
    try:
        mtime = config.config_path.stat().st_mtime
    except OSError:
        return
    if mtime <= state.config_mtime:
        return
    async with state.config_lock:
        try:
            mtime = config.config_path.stat().st_mtime
        except OSError:
            return
        if mtime <= state.config_mtime:
            return
        payload = await asyncio.to_thread(load_config, config.config_path)
        apply_config_payload(config, payload, logger)
        state.config_mtime = mtime


async def main_async() -> None:
    """Initialize state and serve forever."""
    config = parse_args()
    config.upload_dir.mkdir(parents=True, exist_ok=True)
    state = ServerState(
        lock=asyncio.Lock(),
        records=load_records(config.upload_dir),
        config_lock=asyncio.Lock(),
        last_config_check=0.0,
        config_mtime=0.0,
    )
    async with state.lock:
        removed = prune_records(config, state.records)
        cleanup_removed(config.upload_dir, removed)
        await asyncio.to_thread(save_records, config.upload_dir, state.records)

    logger = setup_logging(config.log_level, config.log_file)
    print_logo()
    logger.info("Config file: %s", config.config_path)
    logger.info("Upload dir: %s", config.upload_dir)
    logger.info("Static dir: %s", config.static_dir)
    logger.info(
        "Retention: max_files_per_mode=%s",
        config.max_files_per_mode,
    )
    logger.info("Delete allow: %s", ", ".join(sorted(config.delete_allow)))
    logger.info("Log file: %s", config.log_file)

    server = await asyncio.start_server(
        lambda r, w: handle_client(
            r,
            w,
            config,
            state,
            logger,
            lambda: maybe_reload_config(config, state, logger),
        ),
        host=config.host,
        port=config.port,
    )
    addr = ", ".join(str(sock.getsockname()) for sock in server.sockets or [])
    logger.info("HotfixServer running on http://%s", addr)
    async with server:
        await server.serve_forever()


def main() -> None:
    """CLI entrypoint."""
    try:
        asyncio.run(main_async())
    except KeyboardInterrupt:
        pass
