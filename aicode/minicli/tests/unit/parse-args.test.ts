import test from "node:test";
import assert from "node:assert/strict";
import { parseArguments } from "../../src/cli";

test("parses --version", () => {
  assert.deepEqual(parseArguments(["--version"]), { mode: "version" });
});

test("parses empty args as repl mode", () => {
  assert.deepEqual(parseArguments([]), { mode: "repl" });
});

test("parses invalid args", () => {
  assert.deepEqual(parseArguments(["--bad"]), {
    mode: "invalid",
    invalidArg: "--bad"
  });
});
