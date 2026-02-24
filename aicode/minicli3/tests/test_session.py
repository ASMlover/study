from pathlib import Path
from uuid import uuid4

from minicli3.session import SessionManager


def local_test_dir() -> Path:
    root = Path("tests/.sandbox") / str(uuid4())
    root.mkdir(parents=True, exist_ok=True)
    return root


def test_session_create_switch_rename_delete():
    root = local_test_dir()
    manager = SessionManager(root)
    created = manager.create("alpha")
    assert created.session_id == "alpha"

    switched = manager.set_current("beta")
    assert switched.session_id == "beta"

    assert manager.rename("beta", "gamma") is True
    assert manager.delete("gamma") is True
