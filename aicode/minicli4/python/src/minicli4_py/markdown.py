from __future__ import annotations

import re
from dataclasses import dataclass

from .ansi import ansi


@dataclass(slots=True)
class MarkdownRenderOptions:
    colorize: bool = False


def _style(text: str, fn, colorize: bool) -> str:
    return fn(text) if colorize else text


def _render_inline(line: str, colorize: bool) -> str:
    code_tokens: list[str] = []

    def replace_code(match: re.Match[str]) -> str:
        token = f"\x00CODE{len(code_tokens)}\x00"
        code_tokens.append(_style(match.group(1), ansi.code, colorize))
        return token

    out = re.sub(r"`([^`]+)`", replace_code, line)
    out = re.sub(
        r"\[([^\]]+)\]\(([^)]+)\)",
        lambda m: f"{_style(m.group(1), ansi.cyan, colorize)}{_style(f' <{m.group(2)}>', ansi.gray, colorize)}",
        out,
    )
    out = re.sub(r"\*\*([^*]+)\*\*", lambda m: _style(m.group(1), ansi.bold, colorize), out)
    out = re.sub(r"\*([^*]+)\*", lambda m: _style(m.group(1), ansi.italic, colorize), out)
    out = re.sub(r"^#{1,6}\s+", "", out)
    out = re.sub(r"\x00CODE(\d+)\x00", lambda m: code_tokens[int(m.group(1))] if int(m.group(1)) < len(code_tokens) else "", out)
    return out


class MarkdownAnsiStreamRenderer:
    def __init__(self, options: MarkdownRenderOptions | None = None) -> None:
        self.options = options or MarkdownRenderOptions()
        self.remainder = ""
        self.in_fence = False
        self.fence_lang = ""
        self.code_line = 1

    def _render_line(self, line: str) -> str:
        colorize = self.options.colorize
        fence = re.match(r"^\s*```\s*([a-zA-Z0-9_-]+)?\s*$", line)
        if fence:
            if not self.in_fence:
                self.in_fence = True
                self.fence_lang = fence.group(1) or "text"
                self.code_line = 1
                return _style(f"[code:{self.fence_lang}]", ansi.magenta, colorize)
            self.in_fence = False
            self.fence_lang = ""
            self.code_line = 1
            return _style("[/code]", ansi.magenta, colorize)
        if self.in_fence:
            numbered = f"{self.code_line:>3} | {line}"
            self.code_line += 1
            return _style(numbered, ansi.code, colorize)
        heading = re.match(r"^(#{1,6})\s+(.+)$", line)
        if heading:
            marker = _style(heading.group(1), ansi.yellow, colorize)
            return f"{marker} {_style(_render_inline(heading.group(2), colorize), ansi.bold, colorize)}"
        if re.match(r"^>\s?", line):
            return f"{_style('|', ansi.gray, colorize)} {_render_inline(re.sub(r'^>\s?', '', line), colorize)}"
        ordered = re.match(r"^\s*(\d+)\.\s+(.+)$", line)
        if ordered:
            return f"{_style(f'{ordered.group(1)}.', ansi.blue, colorize)} {_render_inline(ordered.group(2), colorize)}"
        unordered = re.match(r"^\s*[-*+]\s+(.+)$", line)
        if unordered:
            return f"{_style('-', ansi.blue, colorize)} {_render_inline(unordered.group(1), colorize)}"
        table_row = re.match(r"^\s*\|(.+)\|\s*$", line)
        if table_row:
            raw = [cell.strip() for cell in table_row.group(1).split("|")]
            is_div = all(re.match(r"^:?-{3,}:?$", cell) for cell in raw)
            if is_div:
                return _style(f"| {' | '.join(cell.replace(':', '-') for cell in raw)} |", ansi.gray, colorize)
            rendered = [_render_inline(cell, colorize) for cell in raw]
            sep = f" {_style('|', ansi.gray, colorize)} "
            return f"{_style('|', ansi.gray, colorize)} {sep.join(rendered)} {_style('|', ansi.gray, colorize)}"
        return _render_inline(line, colorize)

    def write(self, chunk: str) -> str:
        self.remainder += chunk
        out = ""
        while "\n" in self.remainder:
            line, self.remainder = self.remainder.split("\n", 1)
            out += f"{self._render_line(line)}\n"
        return out

    def flush(self) -> str:
        if not self.remainder:
            return ""
        line = self.remainder
        self.remainder = ""
        return self._render_line(line)
