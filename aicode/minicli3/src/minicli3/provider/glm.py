"""GLM-5 provider for MiniCLI3."""

from __future__ import annotations

import json
import time
from collections.abc import Generator
from typing import Any

import httpx


class ProviderError(Exception):
    pass


class GLMProvider:
    def __init__(
        self,
        api_key: str,
        base_url: str,
        model: str,
        timeout_ms: int,
        max_retries: int,
    ):
        if model != "glm-5":
            raise ProviderError("MiniCLI3 supports glm-5 only.")
        if not api_key:
            raise ProviderError("Missing API key. Set GLM_API_KEY or minicli3 config api_key.")

        self.api_key = api_key
        self.base_url = base_url.rstrip("/")
        self.model = model
        self.timeout_ms = timeout_ms
        self.max_retries = max_retries
        self.client = httpx.Client(
            timeout=timeout_ms / 1000,
            headers={
                "Authorization": f"Bearer {api_key}",
                "Content-Type": "application/json",
            },
        )

    def _url(self) -> str:
        return f"{self.base_url}/chat/completions"

    def chat(
        self,
        messages: list[dict[str, Any]],
        tools: list[dict[str, Any]] | None = None,
        stream: bool = True,
        temperature: float = 0.7,
        max_tokens: int = 4096,
    ) -> dict[str, Any] | Generator[dict[str, Any], None, None]:
        payload: dict[str, Any] = {
            "model": "glm-5",
            "messages": messages,
            "stream": stream,
            "temperature": temperature,
            "max_tokens": max_tokens,
        }
        if tools:
            payload["tools"] = tools

        last_error: Exception | None = None
        for attempt in range(self.max_retries + 1):
            try:
                if stream:
                    return self._stream_request(payload)
                response = self.client.post(self._url(), json=payload)
                return self._parse_response(response)
            except ProviderError as exc:
                last_error = exc
                if attempt == self.max_retries:
                    break
                time.sleep(2**attempt)
            except httpx.HTTPError as exc:
                last_error = ProviderError(f"http error: {exc}")
                if attempt == self.max_retries:
                    break
                time.sleep(2**attempt)

        raise last_error or ProviderError("unknown provider error")

    def _parse_response(self, response: httpx.Response) -> dict[str, Any]:
        if response.status_code == 429:
            raise ProviderError("rate limited (429)")
        if response.status_code >= 500:
            raise ProviderError(f"provider error ({response.status_code})")
        if response.status_code >= 400:
            raise ProviderError(f"request failed ({response.status_code}): {response.text}")

        try:
            payload = response.json()
        except json.JSONDecodeError as exc:
            raise ProviderError(f"invalid json response: {exc}") from exc

        if "error" in payload:
            err = payload["error"]
            msg = err.get("message", "provider returned error")
            raise ProviderError(msg)
        return payload

    def _stream_request(self, payload: dict[str, Any]) -> Generator[dict[str, Any], None, None]:
        with self.client.stream("POST", self._url(), json=payload) as response:
            if response.status_code != 200:
                self._parse_response(response)
                return
            for line in response.iter_lines():
                if not line:
                    continue
                text = line.strip()
                if not text.startswith("data: "):
                    continue
                data = text[6:]
                if data == "[DONE]":
                    break
                yield json.loads(data)

    def close(self) -> None:
        self.client.close()
