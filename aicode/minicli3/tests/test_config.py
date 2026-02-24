from pathlib import Path
from uuid import uuid4

import pytest

from minicli3.config import ConfigManager


def local_test_dir() -> Path:
    root = Path("tests/.sandbox") / str(uuid4())
    root.mkdir(parents=True, exist_ok=True)
    return root


def test_model_is_locked_to_glm5():
    root = local_test_dir()
    manager = ConfigManager(root)
    cfg = manager.load()
    assert cfg.model == "glm-5"
    with pytest.raises(ValueError):
        manager.set("model", "gpt-4")


def test_config_persist_roundtrip():
    root = local_test_dir()
    manager = ConfigManager(root)
    manager.set("timeout_ms", "9000")
    reloaded = ConfigManager(root).load()
    assert reloaded.timeout_ms == 9000
