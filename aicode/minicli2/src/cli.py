"""CLI entry point for MiniCLI2."""

import typer

app = typer.Typer(
    name="minicli2",
    help="AI Agent CLI tool based on GLM-5 model",
    add_completion=False,
)


# Main entry point
@app.command()
def main():
    """Enter REPL interactive mode."""
    from src.commands import repl as repl_cmd
    repl_cmd.main()


@app.command()
def repl():
    """Enter REPL interactive mode."""
    from src.commands import repl as repl_cmd
    repl_cmd.main()


@app.command()
def chat(message: str):
    """Single-turn chat with the AI."""
    from src.commands import chat as chat_cmd
    chat_cmd.chat(message)


# Import subcommands here to avoid circular imports
from src.commands import repl as repl_cmd
from src.commands import session as session_cmd
from src.commands import config as config_cmd

app.add_typer(repl_cmd.app, name="repl")
app.add_typer(session_cmd.app, name="session")
app.add_typer(config_cmd.app, name="config")


if __name__ == "__main__":
    app()
