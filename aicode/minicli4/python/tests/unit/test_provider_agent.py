from __future__ import annotations

from pathlib import Path
from typing import Any

import pytest

from minicli4_py.agent import AgentCallbacks, run_multi_agent_round
from minicli4_py.config import RuntimeConfig
from minicli4_py.provider import GLMProvider, HttpResponse, ProviderError
from minicli4_py.session import SessionMessage
from minicli4_py.tools import ToolRegistry


def test_provider_maps_http_errors() -> None:
    cfg = RuntimeConfig(api_key="x")

    def transport(_url: str, _headers: dict[str, str], _body: bytes, _timeout_ms: int, _stream: bool) -> HttpResponse:
        return HttpResponse(status=401, json_body={"error": "no"})

    provider = GLMProvider(cfg, transport=transport)
    with pytest.raises(ProviderError, match="401"):
        provider.chat({"messages": [], "stream": False, "temperature": 0.7, "max_tokens": 10})


def test_provider_stream_parsing() -> None:
    cfg = RuntimeConfig(api_key="x")

    def transport(_url: str, _headers: dict[str, str], _body: bytes, _timeout_ms: int, _stream: bool) -> HttpResponse:
        return HttpResponse(
            status=200,
            lines=[
                'data: {"choices":[{"delta":{"content":"Hel"}}]}',
                'data: {"choices":[{"delta":{"content":"lo"}}]}',
                "data: [DONE]",
            ],
        )

    provider = GLMProvider(cfg, transport=transport)
    chunks = provider.chat({"messages": [], "stream": True, "temperature": 0.7, "max_tokens": 10})
    assert not isinstance(chunks, dict)
    content = "".join(c["choices"][0]["delta"]["content"] for c in chunks)
    assert content == "Hello"


def test_provider_default_transport_streams_incrementally(monkeypatch: pytest.MonkeyPatch) -> None:
    cfg = RuntimeConfig(api_key="x")

    class FakeResponse:
        status = 200

        def __iter__(self):
            yield b'data: {"choices":[{"delta":{"content":"Hi"}}]}\n'
            yield b"data: [DONE]\n"

        def read(self) -> bytes:
            raise AssertionError("read() must not be used for stream mode")

        def close(self) -> None:
            return

    monkeypatch.setattr("urllib.request.urlopen", lambda *args, **kwargs: FakeResponse())

    provider = GLMProvider(cfg)
    stream = provider.chat({"messages": [], "stream": True, "temperature": 0.7, "max_tokens": 10})
    assert not isinstance(stream, dict)
    chunks = list(stream)
    assert len(chunks) == 1
    assert chunks[0]["choices"][0]["delta"]["content"] == "Hi"


def test_agent_multi_stage_round_with_tool_execution(tmp_path: Path) -> None:
    cfg = RuntimeConfig(api_key="x")
    cfg.stream = False

    responses: list[dict[str, Any]] = [
        {"choices": [{"message": {"content": "plan"}}]},
        {
            "choices": [
                {
                    "message": {
                        "content": "need tool",
                        "tool_calls": [
                            {
                                "id": "c1",
                                "type": "function",
                                "function": {"name": "list_dir", "arguments": {"path": "."}},
                            }
                        ],
                    }
                }
            ]
        },
        {"choices": [{"message": {"content": "done coding"}}]},
        {"choices": [{"message": {"content": "review notes"}}]},
        {"choices": [{"message": {"content": "final answer"}}]},
    ]

    def transport(_url: str, _headers: dict[str, str], _body: bytes, _timeout_ms: int, _stream: bool) -> HttpResponse:
        item = responses.pop(0)
        return HttpResponse(status=200, json_body=item)

    provider = GLMProvider(cfg, transport=transport)
    tools = ToolRegistry(tmp_path, cfg)
    messages = [SessionMessage(role="user", content="start", created_at="2026-01-01T00:00:00Z")]
    deltas: list[str] = []
    tools_called: list[str] = []
    result = run_multi_agent_round(
        "hi",
        messages,
        provider,
        tools,
        cfg,
        AgentCallbacks(
            on_stage=lambda _s, _d: None,
            on_tool=lambda name, _o: tools_called.append(name),
            on_delta=lambda chunk: deltas.append(chunk),
        ),
    )
    assert result.final == "final answer"
    assert result.stages == ["planner", "coder", "reviewer", "orchestrator"]
    assert tools_called == ["list_dir"]
    assert "".join(deltas) == "final answer"
