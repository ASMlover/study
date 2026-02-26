from __future__ import annotations

from minicli4_py.ansi import strip_ansi
from minicli4_py.markdown import MarkdownAnsiStreamRenderer, MarkdownRenderOptions


def test_markdown_renderer_formats_core_constructs() -> None:
    renderer = MarkdownAnsiStreamRenderer(MarkdownRenderOptions(colorize=True))
    input_text = "\n".join(
        [
            "# Title",
            "- item one",
            "Normal with `code` and [link](https://example.com)",
            "| col1 | col2 |",
            "| ---- | :--- |",
            "| v1 | v2 |",
            "```py",
            "x = 1",
            "y = 2",
            "```",
        ]
    )
    rendered = renderer.write(input_text)
    tail = renderer.flush()
    plain = strip_ansi(rendered + tail)
    assert "# Title" in plain
    assert "- item one" in plain
    assert "Normal with code and link <https://example.com>" in plain
    assert "| col1 | col2 |" in plain
    assert "| ---- | ---- |" in plain
    assert "[code:py]" in plain
    assert "  1 | x = 1" in plain
    assert "[/code]" in plain


def test_markdown_renderer_handles_chunked_stream() -> None:
    renderer = MarkdownAnsiStreamRenderer(MarkdownRenderOptions(colorize=False))
    part1 = renderer.write("## He")
    part2 = renderer.write("ader\n- a")
    part3 = renderer.write("bc\n")
    rest = renderer.flush()
    text = f"{part1}{part2}{part3}{rest}"
    assert "## Header" in text
    assert "- abc" in text
