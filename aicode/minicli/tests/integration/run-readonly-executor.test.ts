import test, { TestContext } from "node:test";
import assert from "node:assert/strict";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

function escapeRegExp(input: string): string {
  return input.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

test("/run executes a platform read-only location command successfully", async (t: TestContext) => {
  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH
  );

  const command = process.platform === "win32" ? "/run Get-Location" : "/run pwd";
  const shouldExit = await session.onLine(command);

  assert.equal(shouldExit, false);
  if (errors.join("").includes("EPERM")) {
    t.skip("shell command execution is blocked in this environment");
    return;
  }
  const normalizedOutput = writes.join("").replace(/\\/g, "/");
  const normalizedCwd = process.cwd().replace(/\\/g, "/");
  assert.match(normalizedOutput, new RegExp(escapeRegExp(normalizedCwd)));
  assert.equal(errors.join(""), "");
});
