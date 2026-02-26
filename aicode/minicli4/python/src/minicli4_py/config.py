from __future__ import annotations

import json
import os
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Literal

from .ansi import detect_ansi_theme

SafeMode = Literal["strict", "balanced"]
CompletionMode = Literal["prefix", "contextual"]
ThemeMode = Literal["dark", "light"]
MotionMode = Literal["full", "minimal"]


@dataclass(slots=True)
class RuntimeConfig:
    api_key: str = ""
    base_url: str = "https://open.bigmodel.cn/api/paas/v4"
    model: Literal["glm-5"] = "glm-5"
    stream: bool = True
    timeout_ms: int = 120_000
    max_retries: int = 3
    temperature: float = 0.7
    max_tokens: int = 4096
    agent_max_rounds: int = 6
    safe_mode: SafeMode = "strict"
    theme: ThemeMode = field(default_factory=detect_ansi_theme)
    motion: MotionMode = "full"
    allowed_paths: list[str] = field(default_factory=lambda: ["."])
    shell_allowlist: list[str] = field(default_factory=lambda: ["pwd", "ls", "dir", "cat", "type", "rg"])
    slash_completion_mode: CompletionMode = "contextual"


@dataclass(slots=True)
class LoadedConfig:
    config: RuntimeConfig
    config_path: Path
    state_dir: Path


DEFAULT_CONFIG = RuntimeConfig()


def state_root(project_root: Path) -> Path:
    return project_root / ".minicli4"


def _config_path_for(project_root: Path) -> Path:
    return state_root(project_root) / "config.json"


def _ensure_state(project_root: Path) -> None:
    root = state_root(project_root)
    (root / "sessions").mkdir(parents=True, exist_ok=True)
    (root / "logs").mkdir(parents=True, exist_ok=True)
    (root / "cache").mkdir(parents=True, exist_ok=True)


def _coerce_config(raw: dict[str, object]) -> RuntimeConfig:
    values = asdict(DEFAULT_CONFIG)
    values.update(raw)
    values["model"] = "glm-5"
    values["motion"] = "minimal" if values.get("motion") == "minimal" else "full"
    theme = values.get("theme")
    values["theme"] = theme if theme in ("dark", "light") else detect_ansi_theme()
    env_key = os.environ.get("GLM_API_KEY", "").strip()
    if env_key:
        values["api_key"] = env_key
    return RuntimeConfig(**values)  # type: ignore[arg-type]


def load_config(project_root: Path | None = None) -> LoadedConfig:
    root = project_root or Path.cwd()
    _ensure_state(root)
    config_path = _config_path_for(root)
    if not config_path.exists():
        config_path.write_text(json.dumps(asdict(DEFAULT_CONFIG), indent=2), encoding="utf-8")

    parsed: dict[str, object] = {}
    try:
        loaded = json.loads(config_path.read_text(encoding="utf-8"))
        if isinstance(loaded, dict):
            parsed = loaded
    except (OSError, json.JSONDecodeError):
        parsed = {}

    config = _coerce_config(parsed)
    config.model = "glm-5"
    return LoadedConfig(config=config, config_path=config_path, state_dir=state_root(root))


def save_config(project_root: Path, config: RuntimeConfig) -> None:
    _ensure_state(project_root)
    normalized = asdict(config)
    normalized["model"] = "glm-5"
    _config_path_for(project_root).write_text(json.dumps(normalized, indent=2), encoding="utf-8")
