import test from "node:test";
import assert from "node:assert/strict";
import { runCli } from "../../src/cli";

test("returns exit code 1 for invalid argument", () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const code = runCli(
    ["--unknown"],
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    "linux"
  );

  assert.equal(code, 1);
  assert.equal(writes.length, 0);
  assert.match(errors.join(""), /Unknown argument/);
});

test("empty arguments returns success and non-empty output", () => {
  const writes: string[] = [];
  const errors: string[] = [];
  const code = runCli(
    [],
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message),
      loadConfig: () => ({
        globalPath: "HOME/.minicli/config.json",
        projectPath: "CWD/.minicli/config.json",
        config: {
          model: "glm-4",
          timeoutMs: 30000
        },
        issues: []
      }),
      initDatabase: () => {}
    },
    "linux"
  );

  assert.equal(code, 0);
  assert.equal(errors.length, 0);
  assert.notEqual(writes.join("").trim().length, 0);
});
