import test from "node:test";
import assert from "node:assert/strict";
import { createReplSession, DEFAULT_MAX_INPUT_LENGTH } from "../../src/repl";

test("/run confirmation rejection prevents command execution", async () => {
  const errors: string[] = [];
  const executedCommands: string[] = [];
  const session = createReplSession(
    {
      stdout: () => {},
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      executeRunCommand: (command) => {
        executedCommands.push(command);
        return {
          ok: true,
          stdout: "",
          stderr: "",
          exitCode: 0,
          stdoutTruncated: false,
          stderrTruncated: false
        };
      }
    }
  );

  await session.onLine("/run pwd | wc -c");
  await session.onLine("no");

  assert.equal(executedCommands.length, 0);
  assert.match(errors.join(""), /\[run:confirm\] command cancelled\./);
});
