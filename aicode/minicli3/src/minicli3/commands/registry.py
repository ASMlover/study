"""Slash command registry."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    from minicli3.repl import MiniCLIApp


CommandHandler = Callable[["MiniCLIApp", list[str]], Any]


@dataclass
class CommandSpec:
    name: str
    description: str
    usage: str
    handler: CommandHandler
    subcommands: list[str] = field(default_factory=list)


class CommandRegistry:
    def __init__(self):
        self._commands: dict[str, CommandSpec] = {}

    def register(self, spec: CommandSpec) -> None:
        self._commands[spec.name] = spec

    def get(self, name: str) -> CommandSpec | None:
        return self._commands.get(name)

    def list(self) -> list[CommandSpec]:
        return [self._commands[name] for name in sorted(self._commands.keys())]

    def names(self) -> list[str]:
        return sorted(self._commands.keys())
