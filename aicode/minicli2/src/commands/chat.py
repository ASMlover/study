"""Single-turn chat command."""

import typer
import typing
from rich.console import Console

from src.provider import get_provider
from src.session import get_current_session, get_session_manager
from src.tools.registry import get_tool_registry

console = Console()

app = typer.Typer(name="chat", help="Single-turn chat with the AI")


def chat(message: str) -> None:
    """Send a single message to the AI and print the response.

    Args:
        message: User message.
    """
    # Get session and provider
    session = get_current_session()
    provider = get_provider()
    tool_registry = get_tool_registry()

    # Add user message to session
    session.add_message("user", message)

    # Get enabled tools
    tools = tool_registry.get_enabled_tools()

    # Get response with streaming
    try:
        response = provider.chat(
            messages=session.messages,
            tools=tools if tools else None,
            stream=True,
        )

        if isinstance(response, typing.Generator):
            # Async generator (streaming)
            content = ""
            console.print("[bold blue]Assistant:[/bold blue] ", end="")
            for chunk in response:
                if "choices" in chunk and chunk["choices"]:
                    delta = chunk["choices"][0].get("delta", {})
                    # GLM uses reasoning_content for thinking, content for final answer
                    part = delta.get("content") or delta.get("reasoning_content", "")
                    if part:
                        content += part
                        console.print(part, end="")
            console.print()

            # Add assistant message to session
            session.add_message("assistant", content)
        else:
            # Sync response
            if "choices" in response and response["choices"]:
                # GLM may return reasoning_content or content
                message = response["choices"][0]["message"]
                content = message.get("content") or message.get("reasoning_content", "")
                console.print(f"[bold blue]Assistant:[/bold blue] {content}")

                # Add assistant message to session
                session.add_message("assistant", content)

                # Handle tool calls
                tool_calls = response["choices"][0]["message"].get("tool_calls", [])
                if tool_calls:
                    for tool_call in tool_calls:
                        func = tool_call.get("function", {})
                        name = func.get("name")
                        args = func.get("arguments", "{}")

                        import json
                        try:
                            arguments = json.loads(args)
                        except json.JSONDecodeError:
                            arguments = {}

                        console.print(f"[yellow]Tool call: {name}[/yellow]")

                        # Execute tool
                        result = tool_registry.execute(name, arguments)
                        console.print(f"[dim]{result}[/dim]")

                        # Add tool result to session
                        session.add_tool_result(tool_call["id"], result)

                        # Continue conversation with tool result (streaming)
                        response = provider.chat(
                            messages=session.messages,
                            tools=tools if tools else None,
                            stream=True,
                        )

                        if isinstance(response, typing.Generator):
                            content = ""
                            console.print("[bold blue]Assistant:[/bold blue] ", end="")
                            for chunk in response:
                                if "choices" in chunk and chunk["choices"]:
                                    delta = chunk["choices"][0].get("delta", {})
                                    part = delta.get("content") or delta.get("reasoning_content", "")
                                    if part:
                                        content += part
                                        console.print(part, end="")
                            console.print()
                            session.add_message("assistant", content)
                        elif "choices" in response and response["choices"]:
                            # GLM may return reasoning_content or content
                            message = response["choices"][0]["message"]
                            content = message.get("content") or message.get("reasoning_content", "")
                            console.print(f"[bold blue]Assistant:[/bold blue] {content}")
                            session.add_message("assistant", content)

        # Save session
        get_session_manager().save_current_session()

    except Exception as e:
        console.print(f"[bold red]Error:[/bold red] {e}")


@app.command()
def main(message: str = typer.Argument(..., help="Message to send")):
    """Single-turn chat with the AI."""
    chat(message)


if __name__ == "__main__":
    app()
