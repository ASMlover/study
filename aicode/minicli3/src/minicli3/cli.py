"""CLI entrypoints for MiniCLI3."""

from __future__ import annotations

from pathlib import Path

import typer
from rich.console import Console

from minicli3.config import ConfigManager
from minicli3.repl import MiniCLIApp, run_repl
from minicli3.session import SessionManager

app = typer.Typer(help="Standalone GLM-5 agent CLI", add_completion=False)
console = Console()


@app.command()
def repl() -> None:
    """Start interactive REPL."""
    run_repl()


@app.command()
def chat(message: str) -> None:
    """Single-turn chat."""
    repl_app = MiniCLIApp(interactive=False)
    repl_app.handle_line(message)


@app.command()
def doctor() -> None:
    """Check local runtime setup."""
    cfg = ConfigManager().config
    console.print(f"project={Path.cwd()}")
    console.print(f"config={ConfigManager().config_path}")
    console.print(f"model={cfg.model}")
    console.print(f"api_key_set={'yes' if bool(cfg.api_key) else 'no'}")


config_app = typer.Typer(help="Manage local config")
session_app = typer.Typer(help="Manage sessions")
app.add_typer(config_app, name="config")
app.add_typer(session_app, name="session")


@config_app.command("list")
def config_list() -> None:
    cfg = ConfigManager().config
    for k, v in cfg.__dict__.items():
        console.print(f"{k}={v}")


@config_app.command("get")
def config_get(key: str) -> None:
    value = ConfigManager().get(key)
    console.print(f"{key}={value}")


@config_app.command("set")
def config_set(key: str, value: str) -> None:
    manager = ConfigManager()
    manager.set(key, value)
    console.print("[green]updated[/green]")


@config_app.command("reset")
def config_reset() -> None:
    manager = ConfigManager()
    manager.reset()
    console.print("[green]reset[/green]")


@config_app.command("path")
def config_path() -> None:
    console.print(str(ConfigManager().config_path))


@session_app.command("list")
def session_list() -> None:
    rows = SessionManager().list()
    for row in rows:
        console.print(f"{row['session_id']} {row['message_count']} {row['updated_at']}")


@session_app.command("new")
def session_new(session_id: str = typer.Argument(None)) -> None:
    rec = SessionManager().create(session_id)
    console.print(rec.session_id)


@session_app.command("switch")
def session_switch(session_id: str) -> None:
    rec = SessionManager().set_current(session_id)
    console.print(rec.session_id)


@session_app.command("delete")
def session_delete(session_id: str) -> None:
    ok = SessionManager().delete(session_id)
    console.print("deleted" if ok else "not-found")


@session_app.command("rename")
def session_rename(old_id: str, new_id: str) -> None:
    ok = SessionManager().rename(old_id, new_id)
    console.print("renamed" if ok else "failed")


@session_app.command("current")
def session_current() -> None:
    console.print(SessionManager().current().session_id)


@app.callback(invoke_without_command=True)
def root(ctx: typer.Context) -> None:
    if ctx.invoked_subcommand is None:
        run_repl()


if __name__ == "__main__":
    app()
