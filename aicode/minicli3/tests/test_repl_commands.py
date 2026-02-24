from pathlib import Path
from uuid import uuid4

from minicli3.repl import MiniCLIApp


def local_test_dir() -> Path:
    root = Path("tests/.sandbox") / str(uuid4())
    root.mkdir(parents=True, exist_ok=True)
    return root


def test_builtin_command_count():
    app = MiniCLIApp(project_root=local_test_dir(), interactive=False)
    assert len(app.commands.names()) >= 20


def test_session_command_flow():
    root = local_test_dir()
    app = MiniCLIApp(project_root=root, interactive=False)
    assert app.handle_line("/session new demo") is True
    assert app.handle_line("/session switch demo") is True
    assert app.session.session_id == "demo"


def test_exit_command():
    app = MiniCLIApp(project_root=local_test_dir(), interactive=False)
    assert app.handle_line("/exit") is False


def test_tab_completion_applies_first_match():
    app = MiniCLIApp(project_root=local_test_dir(), interactive=False)
    out1 = app.apply_tab_completion("/")
    assert out1 is not None
    assert out1.startswith("/")
    assert out1.endswith(" ")

    out2 = app.apply_tab_completion("/he")
    assert out2 == "/help "


class _FakeProvider:
    def __init__(self):
        self.calls = 0

    def chat(self, messages, tools=None, stream=True, temperature=0.7, max_tokens=4096):
        self.calls += 1
        # 1st call: planning response that requests write tool.
        if self.calls == 1 and stream is False:
            return {
                "choices": [
                    {
                        "message": {
                            "content": "",
                            "tool_calls": [
                                {
                                    "id": "call_1",
                                    "type": "function",
                                    "function": {
                                        "name": "write",
                                        "arguments": "{\"path\":\"out.txt\",\"content\":\"hello from tool\"}",
                                    },
                                }
                            ],
                        }
                    }
                ]
            }
        # 2nd call: planning loop done (no more tool calls).
        if self.calls == 2 and stream is False:
            return {"choices": [{"message": {"content": "planned", "tool_calls": []}}]}
        # 3rd call: final streamed answer.
        if self.calls == 3 and stream is True:
            def _gen():
                yield {"choices": [{"delta": {"content": "done"}}]}
            return _gen()
        raise AssertionError("unexpected fake provider call flow")


def test_agent_tool_call_can_write_file_in_current_project_root():
    root = local_test_dir()
    app = MiniCLIApp(project_root=root, interactive=False)
    app._provider = _FakeProvider()
    app.handle_line("create out.txt")
    target = root / "out.txt"
    assert target.exists()
    assert target.read_text(encoding="utf-8") == "hello from tool"
