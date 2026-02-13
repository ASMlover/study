import test from "node:test";
import assert from "node:assert/strict";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

test("/run is wired with risk classifier and blocks high-risk command", async () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH
  );

  const shouldExit = await session.onLine("/run rm -rf .");

  assert.equal(shouldExit, false);
  assert.equal(writes.join(""), "");
  assert.match(errors.join(""), /\[run:risk\] high risk command blocked\./);
});
