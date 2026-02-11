import test from "node:test";
import assert from "node:assert/strict";
import {
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH,
  OutputBuffer,
  parseReplLine
} from "../../src/repl";

test("parseReplLine parses regular input", () => {
  const parsed = parseReplLine("hello world");
  assert.deepEqual(parsed, {
    kind: "message",
    text: "hello world",
    truncated: false
  });
});

test("parseReplLine ignores empty and whitespace-only input", () => {
  assert.deepEqual(parseReplLine(""), {
    kind: "empty",
    text: "",
    truncated: false
  });
  assert.deepEqual(parseReplLine("    "), {
    kind: "empty",
    text: "",
    truncated: false
  });
});

test("parseReplLine truncates overlong input", () => {
  const input = "x".repeat(DEFAULT_MAX_INPUT_LENGTH + 20);
  const parsed = parseReplLine(input);
  assert.equal(parsed.kind, "message");
  assert.equal(parsed.text.length, DEFAULT_MAX_INPUT_LENGTH);
  assert.equal(parsed.truncated, true);
});

test("repl session ignores empty line and echoes non-empty line", () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: (message) => errors.push(message)
  });

  session.onLine("   ");
  session.onLine("ping");

  assert.deepEqual(errors, []);
  assert.equal(writes.join(""), "echo: ping\n");
});

test("repl session handles EOF close event", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  session.onClose();
  assert.match(writes.join(""), /EOF received\. Bye\./);
});

test("repl session handles SIGINT safely", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  session.onSigint();
  assert.match(writes.join(""), /\^C/);
  assert.match(writes.join(""), /Interrupted\. Bye\./);
});

test("output buffer flushes by threshold and manual flush", () => {
  const writes: string[] = [];
  const buffer = new OutputBuffer((message) => writes.push(message), 5);

  buffer.append("ab");
  assert.equal(writes.length, 0);

  buffer.append("cde");
  assert.equal(writes.join(""), "abcde");

  buffer.append("xy");
  buffer.flush();
  assert.equal(writes.join(""), "abcdexy");
});
