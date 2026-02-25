import test from "node:test";
import assert from "node:assert/strict";
import { MarkdownAnsiStreamRenderer } from "../../src/markdown";
import { stripAnsi } from "../../src/ansi";

test("markdown renderer formats headings, lists, links, and code fences", () => {
  const renderer = new MarkdownAnsiStreamRenderer({ colorize: true });
  const input = [
    "# Title",
    "- item one",
    "Normal with `code` and [link](https://example.com)",
    "```ts",
    "const x = 1;",
    "```"
  ].join("\n");

  const rendered = renderer.write(input);
  const tail = renderer.flush();
  const plain = stripAnsi(`${rendered}${tail}`);

  assert.match(plain, /Title/);
  assert.match(plain, /• item one/);
  assert.match(plain, /Normal with code and link \(https:\/\/example.com\)/);
  assert.match(plain, /┌ code/);
  assert.match(plain, /const x = 1;/);
  assert.match(plain, /└ end code/);
});

test("markdown renderer handles chunked stream safely", () => {
  const renderer = new MarkdownAnsiStreamRenderer({ colorize: false });
  const part1 = renderer.write("## He");
  const part2 = renderer.write("ader\n- a");
  const part3 = renderer.write("bc\n");
  const rest = renderer.flush();
  const text = `${part1}${part2}${part3}${rest}`;

  assert.match(text, /Header/);
  assert.match(text, /• abc/);
});
