"""Configuration management for MiniCLI3."""

from __future__ import annotations

import json
import os
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class AppConfig:
    """Application config stored in project-local directory."""

    api_key: str = ""
    base_url: str = "https://open.bigmodel.cn/api/paas/v4"
    model: str = "glm-5"
    timeout_ms: int = 120000
    max_retries: int = 3
    stream: bool = True
    temperature: float = 0.7
    max_tokens: int = 4096
    safe_mode: str = "restricted"
    allowed_paths: list[str] = field(default_factory=lambda: ["."])
    enabled_tools: list[str] = field(default_factory=lambda: ["read", "write", "grep", "list_dir", "run_shell"])
    shell_allowlist: list[str] = field(default_factory=lambda: ["git status", "git diff", "python -V", "pytest -q"])

    def validate(self) -> None:
        if self.model != "glm-5":
            raise ValueError("MiniCLI3 supports model='glm-5' only.")
        if self.safe_mode not in {"restricted"}:
            raise ValueError("safe_mode must be 'restricted'.")
        if self.timeout_ms <= 0:
            raise ValueError("timeout_ms must be > 0.")
        if self.max_retries < 0:
            raise ValueError("max_retries must be >= 0.")


class ConfigManager:
    """Load and store project-local config."""

    def __init__(self, project_root: Path | None = None):
        self.project_root = project_root or Path.cwd()
        self.data_dir = self.project_root / ".minicli3"
        self.config_path = self.data_dir / "config.json"
        self._config: AppConfig | None = None

    def _ensure_dirs(self) -> None:
        self.data_dir.mkdir(parents=True, exist_ok=True)

    def load(self) -> AppConfig:
        self._ensure_dirs()
        payload: dict[str, Any] = {}
        if self.config_path.exists():
            payload = json.loads(self.config_path.read_text(encoding="utf-8"))
        else:
            # Bootstrap a project-local config file as the default source of truth.
            self.save(AppConfig())
            payload = json.loads(self.config_path.read_text(encoding="utf-8"))

        env_api_key = os.getenv("GLM_API_KEY")
        if env_api_key:
            payload["api_key"] = env_api_key

        config = AppConfig(**{**asdict(AppConfig()), **payload})
        config.validate()
        self._config = config
        return config

    @property
    def config(self) -> AppConfig:
        if self._config is None:
            return self.load()
        return self._config

    def save(self, config: AppConfig | None = None) -> None:
        cfg = config or self.config
        cfg.validate()
        self._ensure_dirs()
        self.config_path.write_text(json.dumps(asdict(cfg), indent=2, ensure_ascii=False), encoding="utf-8")
        self._config = cfg

    def get(self, key: str) -> Any:
        return getattr(self.config, key, None)

    def set(self, key: str, raw_value: str) -> None:
        cfg = self.config
        if not hasattr(cfg, key):
            raise KeyError(f"Unknown config key: {key}")

        if key == "model" and raw_value != "glm-5":
            raise ValueError("Only glm-5 is supported.")

        current = getattr(cfg, key)
        if isinstance(current, bool):
            value = raw_value.lower() in {"1", "true", "yes", "on"}
        elif isinstance(current, int):
            value = int(raw_value)
        elif isinstance(current, float):
            value = float(raw_value)
        elif isinstance(current, list):
            value = [p.strip() for p in raw_value.split(",") if p.strip()]
        else:
            value = raw_value

        setattr(cfg, key, value)
        cfg.validate()
        self.save(cfg)

    def reset(self) -> None:
        self.save(AppConfig())
