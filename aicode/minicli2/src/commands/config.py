"""Configuration commands."""

import typer
from rich.console import Console
from rich.table import Table

from src.config import get_config_manager

console = Console()

app = typer.Typer(name="config", help="Manage configuration")


@app.command()
def get(key: str = typer.Argument(..., help="Configuration key")):
    """Get configuration value."""
    manager = get_config_manager()
    value = manager.get(key)

    if value is not None:
        console.print(f"[cyan]{key}:[/cyan] {value}")
    else:
        console.print(f"[yellow]Key '{key}' not found.[/yellow]")


@app.command()
def set(key: str = typer.Argument(..., help="Configuration key"), value: str = typer.Argument(..., help="Configuration value")):
    """Set configuration value."""
    manager = get_config_manager()
    manager.set(key, value)
    console.print(f"[green]Set {key} = {value}[/green]")


@app.command()
def list():
    """List all configuration."""
    manager = get_config_manager()
    config = manager.config

    table = Table(title="Configuration")
    table.add_column("Key", style="cyan")
    table.add_column("Value", style="magenta")

    config_data = config.to_dict()
    for key, value in config_data.items():
        table.add_row(key, str(value))

    console.print(table)


@app.command()
def show():
    """Show current configuration."""
    manager = get_config_manager()
    config = manager.config

    console.print("[bold]Current Configuration:[/bold]")
    console.print(f"[cyan]API Key:[/cyan] {'*' * 8}{config.api_key[-4:] if config.api_key else ''}")
    console.print(f"[cyan]Base URL:[/cyan] {config.base_url}")
    console.print(f"[cyan]Model:[/cyan] {config.model}")
    console.print(f"[cyan]Timeout (ms):[/cyan] {config.timeout_ms}")
    console.print(f"[cyan]Max Retries:[/cyan] {config.max_retries}")
    console.print(f"[cyan]Tools Enabled:[/cyan] {', '.join(config.tools_enabled)}")


if __name__ == "__main__":
    app()
