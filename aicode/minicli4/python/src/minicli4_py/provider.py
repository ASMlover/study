from __future__ import annotations

import json
import time
import urllib.error
import urllib.request
from collections.abc import Callable, Generator, Iterable
from http.client import HTTPResponse
from dataclasses import dataclass
from typing import Any, Literal

from .config import RuntimeConfig

ChatRole = Literal["system", "user", "assistant", "tool"]


class ProviderError(RuntimeError):
    def __init__(self, message: str, retryable: bool = False) -> None:
        super().__init__(message)
        self.retryable = retryable


@dataclass(slots=True)
class HttpResponse:
    status: int
    json_body: dict[str, Any] | None = None
    lines: Iterable[str] | None = None


HttpTransport = Callable[[str, dict[str, str], bytes, int, bool], HttpResponse]


def _default_transport(url: str, headers: dict[str, str], body: bytes, timeout_ms: int, stream: bool) -> HttpResponse:
    req = urllib.request.Request(url=url, method="POST", headers=headers, data=body)
    try:
        res = urllib.request.urlopen(req, timeout=timeout_ms / 1000)
        status = int(getattr(res, "status", 200))
        if stream:
            return HttpResponse(status=status, lines=_iter_http_lines(res))
        text = res.read().decode("utf-8")
        res.close()
        parsed = json.loads(text) if text.strip() else {}
        return HttpResponse(status=status, json_body=parsed if isinstance(parsed, dict) else {})
    except urllib.error.HTTPError as err:
        text = err.read().decode("utf-8", errors="replace")
        return HttpResponse(status=int(err.code), json_body={"error": text})
    except TimeoutError as err:
        raise ProviderError("request timeout", retryable=True) from err
    except urllib.error.URLError as err:
        raise ProviderError(f"network error: {err.reason}", retryable=True) from err


def _iter_http_lines(res: HTTPResponse) -> Generator[str, None, None]:
    try:
        for raw_line in res:
            yield raw_line.decode("utf-8", errors="replace")
    finally:
        res.close()


class GLMProvider:
    def __init__(self, config: RuntimeConfig, transport: HttpTransport | None = None) -> None:
        if config.model != "glm-5":
            raise ProviderError("model must be glm-5")
        self.config = config
        self.transport = transport or _default_transport

    def chat(self, req: dict[str, Any]) -> dict[str, Any] | Generator[dict[str, Any], None, None]:
        payload = {
            "model": "glm-5",
            "messages": req.get("messages", []),
            "tools": req.get("tools"),
            "stream": bool(req.get("stream", False)),
            "temperature": req.get("temperature", self.config.temperature),
            "max_tokens": req.get("max_tokens", self.config.max_tokens),
        }
        body = json.dumps(payload).encode("utf-8")
        url = f"{self.config.base_url.rstrip('/')}/chat/completions"
        headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {self.config.api_key}",
        }

        last_error: Exception | None = None
        for attempt in range(self.config.max_retries + 1):
            try:
                response = self.transport(url, headers, body, self.config.timeout_ms, bool(payload["stream"]))
                status = response.status
                if status == 401:
                    raise ProviderError("authentication failed (401)", retryable=False)
                if status == 429:
                    raise ProviderError("rate limited (429)", retryable=True)
                if status >= 500:
                    raise ProviderError(f"provider error ({status})", retryable=True)
                if status >= 400:
                    raise ProviderError(f"request failed ({status})", retryable=False)

                if payload["stream"]:
                    return self._parse_stream(response.lines or [])
                return response.json_body or {"choices": []}
            except Exception as err:  # noqa: BLE001
                last_error = err
                retryable = err.retryable if isinstance(err, ProviderError) else True  # type: ignore[attr-defined]
                if not retryable or attempt == self.config.max_retries:
                    break
                time.sleep((2**attempt) * 0.2)

        if isinstance(last_error, Exception):
            raise last_error
        raise ProviderError("provider error")

    @staticmethod
    def _parse_stream(lines: Iterable[str]) -> Generator[dict[str, Any], None, None]:
        for raw_line in lines:
            line = raw_line.strip()
            if not line.startswith("data: "):
                continue
            payload = line[6:].strip()
            if payload == "[DONE]":
                return
            try:
                parsed = json.loads(payload)
                if isinstance(parsed, dict):
                    yield parsed
            except json.JSONDecodeError:
                continue
