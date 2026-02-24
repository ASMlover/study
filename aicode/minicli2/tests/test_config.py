"""Tests for config module."""

import pytest
import tempfile
import json
from pathlib import Path

from src.config import Config, ConfigManager


def test_config_defaults():
    """Test default configuration values."""
    config = Config()

    assert config.api_key == ""
    assert config.base_url == "https://open.bigmodel.cn/api/paas/v4"
    assert config.model == "glm-5"
    assert config.timeout_ms == 120000
    assert config.max_retries == 3
    assert config.tools_enabled == ["bash", "read", "write", "grep"]


def test_config_to_dict():
    """Test config serialization."""
    config = Config(api_key="test-key", model="test-model")
    data = config.to_dict()

    assert data["api_key"] == "test-key"
    assert data["model"] == "test-model"
    assert data["base_url"] == "https://open.bigmodel.cn/api/paas/v4"


def test_config_from_dict():
    """Test config deserialization."""
    data = {
        "api_key": "test-key",
        "base_url": "https://example.com",
        "model": "test-model",
        "timeout_ms": 60000,
        "max_retries": 5,
        "tools_enabled": ["bash"],
    }
    config = Config.from_dict(data)

    assert config.api_key == "test-key"
    assert config.base_url == "https://example.com"
    assert config.model == "test-model"
    assert config.timeout_ms == 60000
    assert config.max_retries == 5
    assert config.tools_enabled == ["bash"]


def test_config_manager_load():
    """Test config manager loading."""
    with tempfile.TemporaryDirectory() as tmpdir:
        config_file = Path(tmpdir) / "config.json"
        config_file.write_text(json.dumps({"api_key": "test-key"}))

        manager = ConfigManager(Path(tmpdir))
        config = manager.load()

        assert config.api_key == "test-key"


def test_config_manager_save():
    """Test config manager saving."""
    with tempfile.TemporaryDirectory() as tmpdir:
        manager = ConfigManager(Path(tmpdir))
        config = Config(api_key="new-key")
        manager.save(config)

        config_file = Path(tmpdir) / "config.json"
        assert config_file.exists()

        data = json.loads(config_file.read_text())
        assert data["api_key"] == "new-key"


def test_config_manager_get_set():
    """Test config get/set."""
    with tempfile.TemporaryDirectory() as tmpdir:
        manager = ConfigManager(Path(tmpdir))
        manager.load()

        manager.set("model", "custom-model")
        assert manager.get("model") == "custom-model"
