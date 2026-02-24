"""REPL command entry point."""

import typer
from src.repl import repl as run_repl

app = typer.Typer(name="repl", help="Enter REPL interactive mode")


def main():
    """Enter REPL interactive mode."""
    run_repl()


if __name__ == "__main__":
    app()
