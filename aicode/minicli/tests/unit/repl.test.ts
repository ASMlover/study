import test from "node:test";
import assert from "node:assert/strict";
import {
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH,
  HELP_TEXT,
  matchReplCommand,
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

  const shouldExitFromEmpty = session.onLine("   ");
  const shouldExitFromEcho = session.onLine("ping");

  assert.deepEqual(errors, []);
  assert.equal(shouldExitFromEmpty, false);
  assert.equal(shouldExitFromEcho, false);
  assert.equal(writes.join(""), "echo: ping\n");
});

test("matchReplCommand matches help and exit commands", () => {
  assert.deepEqual(matchReplCommand("/help"), { kind: "help" });
  assert.deepEqual(matchReplCommand("/exit"), { kind: "exit" });
});

test("matchReplCommand handles unknown command and non-command input", () => {
  assert.deepEqual(matchReplCommand("/unknown"), {
    kind: "unknown",
    token: "/unknown"
  });
  assert.deepEqual(matchReplCommand("hello"), { kind: "none" });
});

test("command matching is case-sensitive", () => {
  assert.deepEqual(matchReplCommand("/Help"), {
    kind: "unknown",
    token: "/Help"
  });
  assert.deepEqual(matchReplCommand("/EXIT"), {
    kind: "unknown",
    token: "/EXIT"
  });
});

test("repl session prints help content", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  const shouldExit = session.onLine("/help");

  assert.equal(shouldExit, false);
  assert.equal(writes.join(""), HELP_TEXT);
});

test("repl session marks /exit as exit signal", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  const shouldExit = session.onLine("/exit");

  assert.equal(shouldExit, true);
  assert.equal(writes.join(""), "Bye.\n");
});

test("repl session routes unknown slash command to stderr", () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: (message) => errors.push(message)
  });

  const shouldExit = session.onLine("/nope");

  assert.equal(shouldExit, false);
  assert.equal(writes.length, 0);
  assert.match(errors.join(""), /Unknown command: \/nope/);
});

test("repl command handling trims surrounding whitespace", () => {
  const writes: string[] = [];
  const session = createReplSession({
    stdout: (message) => writes.push(message),
    stderr: () => {}
  });

  const shouldExit = session.onLine("   /help   ");

  assert.equal(shouldExit, false);
  assert.equal(writes.join(""), HELP_TEXT);
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
