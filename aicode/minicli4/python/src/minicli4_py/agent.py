from __future__ import annotations

from collections.abc import Callable, Generator
from dataclasses import asdict
from dataclasses import dataclass
from typing import Any

from .config import RuntimeConfig
from .provider import GLMProvider
from .session import SessionMessage
from .tools import ToolRegistry


@dataclass(slots=True)
class AgentCallbacks:
    on_stage: Callable[[str, str], None]
    on_tool: Callable[[str, str], None]
    on_delta: Callable[[str], None]


@dataclass(slots=True)
class AgentResult:
    final: str
    stages: list[str]


def _extract_assistant(response: dict[str, Any]) -> tuple[str, list[dict[str, Any]]]:
    choices = response.get("choices", [])
    msg = choices[0].get("message", {}) if choices and isinstance(choices[0], dict) else {}
    content = str(msg.get("content", ""))
    calls = msg.get("tool_calls", [])
    tool_calls = calls if isinstance(calls, list) else []
    return content, tool_calls


def _as_chat_messages(messages: list[SessionMessage]) -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for m in messages:
        item: dict[str, Any] = {"role": m.role, "content": m.content}
        if m.tool_call_id:
            item["tool_call_id"] = m.tool_call_id
        if m.name:
            item["name"] = m.name
        out.append(item)
    return out


def run_multi_agent_round(
    input_text: str,
    session_messages: list[SessionMessage],
    provider: GLMProvider,
    tools: ToolRegistry,
    config: RuntimeConfig,
    callbacks: AgentCallbacks,
) -> AgentResult:
    stages: list[str] = []
    working = [*_as_chat_messages(session_messages), {"role": "user", "content": input_text}]

    callbacks.on_stage("planner", "planning")
    stages.append("planner")
    planner = provider.chat(
        {
            "messages": [
                {"role": "system", "content": "You are planner agent. Return concise execution plan for coder and reviewer."},
                *working,
            ],
            "stream": False,
            "temperature": config.temperature,
            "max_tokens": min(config.max_tokens, 512),
        }
    )
    if isinstance(planner, Generator):
        raise RuntimeError("planner should not stream")
    plan_text, _ = _extract_assistant(planner)

    callbacks.on_stage("coder", "tool planning and execution")
    stages.append("coder")
    coder_context: list[dict[str, Any]] = [
        {"role": "system", "content": "You are coder agent. Use tools when needed and produce actionable answer."},
        {"role": "system", "content": f"planner_notes:\n{plan_text}"},
        *working,
    ]
    rounds = 0
    while rounds < config.agent_max_rounds:
        rounds += 1
        coding = provider.chat(
            {
                "messages": coder_context,
                "tools": tools.definitions(),
                "stream": False,
                "temperature": config.temperature,
                "max_tokens": config.max_tokens,
            }
        )
        if isinstance(coding, Generator):
            raise RuntimeError("coder planning should not stream")
        parsed_content, parsed_calls = _extract_assistant(coding)
        coder_context.append({"role": "assistant", "content": parsed_content})
        if len(parsed_calls) == 0:
            break
        for call in parsed_calls:
            fn = call.get("function", {})
            tool_name = str(fn.get("name", ""))
            result = tools.execute(tool_name, fn.get("arguments"), str([asdict(m) for m in session_messages]))
            callbacks.on_tool(tool_name, result.output)
            coder_context.append(
                {
                    "role": "tool",
                    "tool_call_id": call.get("id"),
                    "name": tool_name,
                    "content": result.output,
                }
            )
            if result.requires_approval:
                coder_context.append({"role": "assistant", "content": "Tool execution denied pending approval in strict mode."})

    callbacks.on_stage("reviewer", "quality review")
    stages.append("reviewer")
    reviewer = provider.chat(
        {
            "messages": [
                {"role": "system", "content": "You are reviewer agent. Improve clarity and safety before final user response."},
                *coder_context,
            ],
            "stream": False,
            "temperature": config.temperature,
            "max_tokens": min(config.max_tokens, 1024),
        }
    )
    reviewer_text = ""
    if not isinstance(reviewer, Generator):
        reviewer_text, _ = _extract_assistant(reviewer)

    callbacks.on_stage("orchestrator", "final streaming reply")
    stages.append("orchestrator")
    final_req = provider.chat(
        {
            "messages": [
                {"role": "system", "content": "You are the final assistant. Provide concise, user-facing answer in markdown."},
                *coder_context,
                {"role": "system", "content": f"reviewer_notes:\n{reviewer_text}"},
            ],
            "stream": config.stream,
            "temperature": config.temperature,
            "max_tokens": config.max_tokens,
        }
    )

    final = ""
    if isinstance(final_req, Generator):
        for chunk in final_req:
            choices = chunk.get("choices", [])
            delta = choices[0].get("delta", {}) if choices and isinstance(choices[0], dict) else {}
            text = str(delta.get("content", "") or delta.get("reasoning_content", ""))
            if text:
                final += text
                callbacks.on_delta(text)
    else:
        final, _ = _extract_assistant(final_req)
        callbacks.on_delta(final)

    return AgentResult(final=final, stages=stages)
