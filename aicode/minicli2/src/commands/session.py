"""Session management commands."""

import typer
from rich.console import Console
from rich.table import Table

from src.session import get_session_manager, Session

console = Console()

app = typer.Typer(name="session", help="Manage chat sessions")


@app.command()
def list():
    """List all sessions."""
    manager = get_session_manager()
    sessions = manager.list_sessions()

    if not sessions:
        console.print("[yellow]No sessions found.[/yellow]")
        return

    table = Table(title="Sessions")
    table.add_column("ID", style="cyan")
    table.add_column("Messages", style="magenta")
    table.add_column("Created", style="green")
    table.add_column("Updated", style="yellow")

    for session in sessions:
        table.add_row(
            session["session_id"],
            str(session["message_count"]),
            session.get("created_at", "")[:19],
            session.get("updated_at", "")[:19],
        )

    console.print(table)


@app.command()
def new(session_id: str = typer.Option(None, help="Session ID")):
    """Create a new session."""
    manager = get_session_manager()
    session = manager.create_session(session_id)
    console.print(f"[green]Created new session: {session.session_id}[/green]")


@app.command()
def switch(session_id: str = typer.Argument(..., help="Session ID to switch to")):
    """Switch to a different session."""
    manager = get_session_manager()
    session = manager.load_session(session_id)

    if session is None:
        console.print(f"[yellow]Session '{session_id}' not found. Creating new.[/yellow]")
        session = manager.create_session(session_id)

    manager.set_current_session(session)
    console.print(f"[green]Switched to session: {session_id}[/green]")


@app.command()
def clear():
    """Clear current session history."""
    manager = get_session_manager()
    session = manager.get_current_session()

    if session:
        manager.clear_current_session()
        console.print(f"[green]Cleared session: {session.session_id}[/green]")
    else:
        console.print("[yellow]No active session.[/yellow]")


@app.command()
def show():
    """Show current session info."""
    manager = get_session_manager()
    session = manager.get_current_session()

    if session:
        console.print(f"[cyan]Session ID:[/cyan] {session.session_id}")
        console.print(f"[cyan]Messages:[/cyan] {len(session.messages)}")
        console.print(f"[cyan]Created:[/cyan] {session.created_at}")
        console.print(f"[cyan]Updated:[/cyan] {session.updated_at}")
    else:
        console.print("[yellow]No active session.[/yellow]")


if __name__ == "__main__":
    app()
