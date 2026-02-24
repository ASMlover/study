"""Configuration management for MiniCLI2."""

import json
import os
from pathlib import Path
from typing import Optional
from dataclasses import dataclass, field, asdict


@dataclass
class Config:
    """Configuration for MiniCLI2."""

    api_key: str = ""
    base_url: str = "https://open.bigmodel.cn/api/paas/v4"
    model: str = "glm-5"
    timeout_ms: int = 120000
    max_retries: int = 3
    tools_enabled: list[str] = field(default_factory=lambda: ["bash", "read", "write", "grep"])

    def to_dict(self) -> dict:
        """Convert config to dictionary."""
        return asdict(self)

    @classmethod
    def from_dict(cls, data: dict) -> "Config":
        """Create config from dictionary."""
        return cls(**data)


class ConfigManager:
    """Manages configuration for MiniCLI2."""

    DEFAULT_CONFIG = Config()

    def __init__(self, config_dir: Optional[Path] = None):
        """Initialize config manager.

        Args:
            config_dir: Path to config directory. Defaults to .minicli2 in current directory.
        """
        if config_dir is None:
            config_dir = Path.cwd() / ".minicli2"
        self.config_dir = config_dir
        self.config_file = config_dir / "config.json"
        self._config: Optional[Config] = None

    def _ensure_config_dir(self) -> None:
        """Ensure config directory exists."""
        self.config_dir.mkdir(parents=True, exist_ok=True)

    def load(self) -> Config:
        """Load configuration from file and environment variables.

        Priority:
        1. Environment variables (GLM_API_KEY)
        2. Config file
        3. Default values
        """
        self._ensure_config_dir()

        # Load from file
        config_data = {}
        if self.config_file.exists():
            with open(self.config_file, "r", encoding="utf-8") as f:
                config_data = json.load(f)

        # Override with environment variables
        if api_key := os.getenv("GLM_API_KEY"):
            config_data["api_key"] = api_key

        # Merge with defaults
        config = self.DEFAULT_CONFIG.to_dict()
        config.update(config_data)

        self._config = Config.from_dict(config)
        return self._config

    def save(self, config: Optional[Config] = None) -> None:
        """Save configuration to file.

        Args:
            config: Configuration to save. Uses current config if not provided.
        """
        if config is None:
            config = self._config or self.DEFAULT_CONFIG

        self._ensure_config_dir()

        with open(self.config_file, "w", encoding="utf-8") as f:
            json.dump(config.to_dict(), f, indent=2, ensure_ascii=False)

        self._config = config

    def get(self, key: str, default: Optional[str] = None) -> Optional[str]:
        """Get configuration value by key.

        Args:
            key: Configuration key.
            default: Default value if key not found.

        Returns:
            Configuration value or default.
        """
        if self._config is None:
            self.load()

        value = getattr(self._config, key, default)
        return str(value) if value is not None else default

    def set(self, key: str, value: str) -> None:
        """Set configuration value.

        Args:
            key: Configuration key.
            value: Configuration value.
        """
        if self._config is None:
            self.load()

        if hasattr(self._config, key):
            # Handle list type for tools_enabled
            if key == "tools_enabled":
                setattr(self._config, key, value.split(","))
            else:
                # Try to convert to appropriate type
                current_value = getattr(self._config, key)
                if isinstance(current_value, int):
                    setattr(self._config, key, int(value))
                elif isinstance(current_value, float):
                    setattr(self._config, key, float(value))
                else:
                    setattr(self._config, key, value)

            self.save()

    @property
    def config(self) -> Config:
        """Get current configuration."""
        if self._config is None:
            self.load()
        return self._config


# Global config manager instance
_config_manager: Optional[ConfigManager] = None


def get_config_manager() -> ConfigManager:
    """Get global config manager instance."""
    global _config_manager
    if _config_manager is None:
        _config_manager = ConfigManager()
    return _config_manager


def get_config() -> Config:
    """Get current configuration."""
    return get_config_manager().config
