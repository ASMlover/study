# -*- coding: UTF-8 -*-
from __future__ import annotations

"""Async HTTP handlers and request processing for HotfixServer."""

import asyncio
import json
import mimetypes
import os
from http import HTTPStatus
import logging
from typing import Any, Awaitable, Callable
from urllib.parse import unquote, urlparse

from .constants import (
    DEFAULT_MAX_HEADER_BYTES,
    DEFAULT_READ_CHUNK_SIZE,
    MULTIPART_BOUNDARY_KEY,
    REQUEST_HEADER_SEPARATOR,
)
from .models import MODE_DEFS, FileRecord, ServerConfig, ServerState, now_utc
from .storage import cleanup_removed, match_mode, prune_records, save_records


class RequestTooLarge(Exception):
    """Raised when a request exceeds the allowed body size."""

    def __init__(
        self,
        method: str,
        target: str,
        headers: dict[str, str],
        content_length: int,
        limit: int,
    ) -> None:
        super().__init__(f"request too large: {content_length} > {limit}")
        self.method = method
        self.target = target
        self.headers = headers
        self.content_length = content_length
        self.limit = limit


async def read_request(
    reader: asyncio.StreamReader,
    max_body_bytes: int | None,
) -> tuple[str, str, dict[str, str], bytes, int] | None:
    """Read a single HTTP request from the stream."""
    header_bytes = b""
    while REQUEST_HEADER_SEPARATOR not in header_bytes:
        chunk = await reader.read(DEFAULT_READ_CHUNK_SIZE)
        if not chunk:
            return None
        header_bytes += chunk
        if len(header_bytes) > DEFAULT_MAX_HEADER_BYTES:
            return None

    header_part, rest = header_bytes.split(REQUEST_HEADER_SEPARATOR, 1)
    try:
        lines = header_part.decode("utf-8").split("\r\n")
    except UnicodeDecodeError:
        return None
    if not lines:
        return None
    request_line = lines[0]
    parts = request_line.split()
    if len(parts) != 3:
        return None
    method, target, _ = parts
    headers: dict[str, str] = {}
    for line in lines[1:]:
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        headers[key.strip().lower()] = value.strip()

    try:
        content_length = int(headers.get("content-length", "0") or 0)
    except ValueError:
        return None
    if max_body_bytes is not None and content_length > max_body_bytes:
        raise RequestTooLarge(method, target, headers, content_length, max_body_bytes)
    body = rest
    if len(body) < content_length:
        body += await reader.readexactly(content_length - len(body))
    return method, target, headers, body, content_length


def parse_multipart(body: bytes, content_type: str) -> list[tuple[str, bytes]]:
    """Parse multipart/form-data body into (filename, content) tuples."""
    if MULTIPART_BOUNDARY_KEY not in content_type:
        return []
    boundary = content_type.split(MULTIPART_BOUNDARY_KEY, 1)[1].strip()
    if boundary.startswith('"') and boundary.endswith('"'):
        boundary = boundary[1:-1]
    boundary_bytes = f"--{boundary}".encode("utf-8")
    parts = body.split(boundary_bytes)
    files: list[tuple[str, bytes]] = []
    for part in parts:
        if not part or part in (b"--\r\n", b"--"):
            continue
        if part.startswith(b"\r\n"):
            part = part[2:]
        if part.endswith(b"\r\n"):
            part = part[:-2]
        if part.endswith(b"--"):
            part = part[:-2]
        if b"\r\n\r\n" not in part:
            continue
        header_blob, content = part.split(b"\r\n\r\n", 1)
        header_lines = header_blob.decode("utf-8", errors="ignore").split("\r\n")
        headers: dict[str, str] = {}
        for line in header_lines:
            if ":" not in line:
                continue
            key, value = line.split(":", 1)
            headers[key.strip().lower()] = value.strip()
        disposition = headers.get("content-disposition", "")
        if "filename=" not in disposition:
            continue
        filename_part = disposition.split("filename=", 1)[1]
        filename = filename_part.strip().strip('"')
        if not filename:
            continue
        files.append((filename, content))
    return files


def response_bytes(
    status: HTTPStatus, body: bytes, headers: dict[str, str] | None = None
) -> bytes:
    """Build a raw HTTP/1.1 response."""
    reason = status.phrase
    lines = [f"HTTP/1.1 {status.value} {reason}"]
    header_map = {"Content-Length": str(len(body))}
    if headers:
        header_map.update(headers)
    for key, value in header_map.items():
        lines.append(f"{key}: {value}")
    lines.append("")
    lines.append("")
    head = "\r\n".join(lines).encode("utf-8")
    return head + body


def json_response(status: HTTPStatus, payload: dict[str, Any]) -> bytes:
    """Build a JSON response with UTF-8 body."""
    data = json.dumps(payload, ensure_ascii=True).encode("utf-8")
    return response_bytes(
        status, data, {"Content-Type": "application/json; charset=utf-8"}
    )


async def handle_client(
    reader: asyncio.StreamReader,
    writer: asyncio.StreamWriter,
    config: ServerConfig,
    state: ServerState,
    logger: logging.Logger,
    maybe_reload: Callable[[], Awaitable[None]],
) -> None:
    """Process a single HTTP connection."""
    await maybe_reload()
    try:
        request = await read_request(reader, config.upload_max_request_bytes)
    except RequestTooLarge as exc:
        requester_ip = client_ip(exc.headers, writer, config)
        status = HTTPStatus.REQUEST_ENTITY_TOO_LARGE
        response = json_response(status, {"error": "request too large"})
        logger.warning(
            "UPLOAD rejected ip=%s reason=max_request_bytes length=%d limit=%d",
            requester_ip,
            exc.content_length,
            exc.limit,
        )
        writer.write(response)
        await writer.drain()
        writer.close()
        await writer.wait_closed()
        return
    if request is None:
        writer.close()
        await writer.wait_closed()
        return
    method, target, headers, body, content_length = request
    path = urlparse(target).path
    path = unquote(path)
    requester_ip = client_ip(headers, writer, config)
    status = HTTPStatus.OK
    response = b""
    try:
        if method == "GET" and path in ("/", "/index.html"):
            status, response = await serve_static(config, "index.html")
        elif method == "GET" and path.startswith("/static/"):
            status, response = await serve_static(config, path.removeprefix("/static/"))
        elif method == "GET" and path == "/health":
            status = HTTPStatus.OK
            response = json_response(status, {"status": "ok"})
        elif method == "GET" and path == "/api/config":
            status = HTTPStatus.OK
            response = json_response(
                status,
                {
                    "modes": [mode.__dict__ for mode in MODE_DEFS],
                    "delete_allowed": delete_allowed(headers, writer, config),
                    "server_time": now_utc().isoformat(),
                },
            )
        elif method == "GET" and path == "/api/files":
            async with state.lock:
                removed = prune_records(config, state.records)
                cleanup_removed(config.upload_dir, removed)
                await asyncio.to_thread(
                    save_records, config.upload_dir, state.records
                )
                status = HTTPStatus.OK
                response = json_response(
                    status, build_file_response(config, state, headers, writer)
                )
            logger.info(
                "LIST ip=%s modes=%d removed=%d",
                requester_ip,
                len(MODE_DEFS),
                len(removed),
            )
        elif method == "GET" and path.startswith("/files/"):
            name = path.removeprefix("/files/")
            status, response = await serve_file(config, name)
            if status == HTTPStatus.OK:
                logger.info("DOWNLOAD ip=%s file=%s", requester_ip, name)
        elif method == "POST" and path == "/upload":
            status, response, _uploaded = await handle_upload(
                config, state, headers, body, content_length, requester_ip, logger
            )
        elif method == "DELETE" and path.startswith("/files/"):
            name = path.removeprefix("/files/")
            status, response, _deleted = await handle_delete(
                config, state, headers, writer, name, requester_ip, logger
            )
        else:
            status = HTTPStatus.NOT_FOUND
            response = json_response(status, {"error": "not found"})
    except Exception as exc:
        logger.exception("request failed: %s", exc)
        status = HTTPStatus.INTERNAL_SERVER_ERROR
        response = json_response(status, {"error": "internal error"})

    writer.write(response)
    await writer.drain()
    writer.close()
    await writer.wait_closed()
    logger.info("%s %s ip=%s -> %s", method, path, requester_ip, status.value)


def delete_allowed(
    headers: dict[str, str],
    writer: asyncio.StreamWriter,
    config: ServerConfig,
) -> bool:
    """Return True if client IP is allowed to delete files."""
    ip = client_ip(headers, writer, config)
    return ip in config.delete_allow


def client_ip(
    headers: dict[str, str],
    writer: asyncio.StreamWriter,
    config: ServerConfig,
) -> str:
    """Resolve client IP with optional proxy support."""
    if config.trust_x_forwarded_for:
        forwarded = headers.get("x-forwarded-for")
        if forwarded:
            return forwarded.split(",")[0].strip()
    peer = writer.get_extra_info("peername")
    if not peer:
        return ""
    return str(peer[0])


async def serve_file(config: ServerConfig, name: str) -> tuple[HTTPStatus, bytes]:
    """Serve a file download response."""
    safe_name = os.path.basename(name)
    if safe_name != name:
        status = HTTPStatus.BAD_REQUEST
        return status, json_response(status, {"error": "invalid path"})
    path = (config.upload_dir / safe_name).resolve()
    base = config.upload_dir.resolve()
    if base not in path.parents and base != path:
        status = HTTPStatus.BAD_REQUEST
        return status, json_response(status, {"error": "invalid path"})
    if not path.exists():
        status = HTTPStatus.NOT_FOUND
        return status, json_response(status, {"error": "file not found"})
    data = await asyncio.to_thread(path.read_bytes)
    content_type, _ = mimetypes.guess_type(path.name)
    headers = {
        "Content-Type": content_type or "application/octet-stream",
        "Content-Disposition": f'attachment; filename="{path.name}"',
    }
    status = HTTPStatus.OK
    return status, response_bytes(status, data, headers)


async def serve_static(config: ServerConfig, relative: str) -> tuple[HTTPStatus, bytes]:
    """Serve a static asset response."""
    safe_name = relative.lstrip("/").replace("\\", "/")
    path = (config.static_dir / safe_name).resolve()
    base = config.static_dir.resolve()
    if base not in path.parents and base != path:
        status = HTTPStatus.NOT_FOUND
        return status, json_response(status, {"error": "not found"})
    if not path.exists() or not path.is_file():
        status = HTTPStatus.NOT_FOUND
        return status, json_response(status, {"error": "not found"})
    data = await asyncio.to_thread(path.read_bytes)
    content_type, _ = mimetypes.guess_type(path.name)
    status = HTTPStatus.OK
    return status, response_bytes(
        status,
        data,
        {"Content-Type": content_type or "application/octet-stream"},
    )


async def handle_upload(
    config: ServerConfig,
    state: ServerState,
    headers: dict[str, str],
    body: bytes,
    content_length: int,
    requester_ip: str,
    logger: logging.Logger,
) -> tuple[HTTPStatus, bytes, list[str]]:
    """Handle multipart upload and update metadata."""
    content_type = headers.get("content-type", "")
    if "multipart/form-data" not in content_type:
        status = HTTPStatus.BAD_REQUEST
        logger.warning(
            "UPLOAD rejected ip=%s reason=invalid_content_type content_type=%s",
            requester_ip,
            content_type,
        )
        return status, json_response(
            status, {"error": "expected multipart/form-data"}
        ), []
    if content_length <= 0:
        status = HTTPStatus.BAD_REQUEST
        logger.warning("UPLOAD rejected ip=%s reason=missing_content_length", requester_ip)
        return status, json_response(status, {"error": "missing content-length"}), []
    files = parse_multipart(body, content_type)
    if not files:
        status = HTTPStatus.BAD_REQUEST
        logger.warning("UPLOAD rejected ip=%s reason=no_files", requester_ip)
        return status, json_response(status, {"error": "no files found"}), []

    max_file_bytes = config.upload_max_file_bytes
    violations: list[str] = []
    has_size_violation = False
    has_mode_violation = False
    for filename, content in files:
        safe_name = os.path.basename(filename)
        if not safe_name:
            violations.append("empty filename")
            has_mode_violation = True
            logger.warning("UPLOAD rejected ip=%s reason=empty_filename", requester_ip)
            continue
        mode = match_mode(safe_name)
        if not mode:
            violations.append(f"{safe_name}: unknown mode")
            has_mode_violation = True
            logger.warning(
                "UPLOAD rejected ip=%s file=%s reason=unknown_mode",
                requester_ip,
                safe_name,
            )
            continue
        if len(content) > max_file_bytes:
            violations.append(f"{safe_name}: file too large")
            has_size_violation = True
            logger.warning(
                "UPLOAD rejected ip=%s file=%s reason=max_file_bytes size=%d limit=%d",
                requester_ip,
                safe_name,
                len(content),
                max_file_bytes,
            )
            continue

    if violations:
        status = (
            HTTPStatus.BAD_REQUEST
            if has_mode_violation
            else HTTPStatus.REQUEST_ENTITY_TOO_LARGE
        )
        return status, json_response(status, {"error": "upload rejected"}), []

    saved: list[dict[str, Any]] = []
    config.upload_dir.mkdir(parents=True, exist_ok=True)
    async with state.lock:
        for filename, content in files:
            safe_name = os.path.basename(filename)
            mode = match_mode(safe_name)
            target = config.upload_dir / safe_name
            await asyncio.to_thread(target.write_bytes, content)
            record = FileRecord(
                name=target.name,
                mode=mode,
                uploaded_at=now_utc(),
                size=len(content),
            )
            state.records[target.name] = record
            saved.append({"name": target.name, "mode": mode})

        removed = prune_records(config, state.records)
        cleanup_removed(config.upload_dir, removed)
        await asyncio.to_thread(save_records, config.upload_dir, state.records)

    payload: dict[str, Any] = {"saved": saved}
    status = HTTPStatus.OK
    uploaded_names = [item["name"] for item in saved]
    logger.info(
        "UPLOAD ip=%s files=%d saved=%s warnings=%d",
        requester_ip,
        len(files),
        ", ".join(uploaded_names),
        0,
    )
    return status, json_response(status, payload), uploaded_names


async def handle_delete(
    config: ServerConfig,
    state: ServerState,
    headers: dict[str, str],
    writer: asyncio.StreamWriter,
    name: str,
    requester_ip: str,
    logger: logging.Logger,
) -> tuple[HTTPStatus, bytes, str | None]:
    """Handle delete requests with whitelist checks."""
    if not delete_allowed(headers, writer, config):
        status = HTTPStatus.FORBIDDEN
        logger.warning("DELETE denied ip=%s file=%s", requester_ip, name)
        return status, json_response(status, {"error": "delete not allowed"}), None

    safe_name = os.path.basename(name)
    if safe_name != name:
        status = HTTPStatus.BAD_REQUEST
        return status, json_response(status, {"error": "invalid path"}), None
    path = (config.upload_dir / safe_name).resolve()
    base = config.upload_dir.resolve()
    if base not in path.parents and base != path:
        status = HTTPStatus.BAD_REQUEST
        return status, json_response(status, {"error": "invalid path"}), None
    if not path.exists():
        status = HTTPStatus.NOT_FOUND
        return status, json_response(status, {"error": "file not found"}), None
    try:
        await asyncio.to_thread(path.unlink)
    except OSError:
        status = HTTPStatus.INTERNAL_SERVER_ERROR
        return status, json_response(status, {"error": "delete failed"}), None

    async with state.lock:
        state.records.pop(path.name, None)
        await asyncio.to_thread(save_records, config.upload_dir, state.records)
    status = HTTPStatus.OK
    logger.info("DELETE ip=%s file=%s", requester_ip, path.name)
    return status, json_response(status, {"deleted": path.name}), path.name


def build_file_response(
    config: ServerConfig,
    state: ServerState,
    headers: dict[str, str],
    writer: asyncio.StreamWriter,
) -> dict[str, Any]:
    """Build API response listing files by mode."""
    modes: list[dict[str, Any]] = []
    for mode in MODE_DEFS:
        mode_records = [
            record for record in state.records.values() if record.mode == mode.key
        ]
        mode_records.sort(key=lambda r: r.uploaded_at, reverse=True)
        total_size = sum(record.size for record in mode_records)
        modes.append(
            {
                "key": mode.key,
                "title": mode.title,
                "total_count": len(mode_records),
                "total_size": total_size,
                "files": [
                    {
                        "name": record.name,
                        "mode": record.mode,
                        "uploaded_at": record.uploaded_at.isoformat(),
                        "size": record.size,
                    }
                    for record in mode_records
                ],
            }
        )
    return {
        "server_time": now_utc().isoformat(),
        "delete_allowed": delete_allowed(headers, writer, config),
        "modes": modes,
    }
