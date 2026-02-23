import test from "node:test";
import assert from "node:assert/strict";
import {
  createDefaultReplCommandRegistry,
  createReplSession,
  DEFAULT_MAX_INPUT_LENGTH
} from "../../src/repl";

test("27 commands are enumerable and executable via repl routing", async () => {
  const registry = createDefaultReplCommandRegistry();
  const commands = registry.list();
  assert.equal(commands.length, 27);

  const writes: string[] = [];
  const errors: string[] = [];
  const session = createReplSession(
    {
      stdout: (message) => writes.push(message),
      stderr: (message) => errors.push(message)
    },
    DEFAULT_MAX_INPUT_LENGTH,
    {
      commandRegistry: registry,
      executeRunCommand: () => ({
        ok: true,
        stdout: "cwd",
        stderr: "",
        exitCode: 0,
        stdoutTruncated: false,
        stderrTruncated: false
      })
    }
  );

  const samples: Record<string, string> = {
    "/help": "/help",
    "/login": "/login sk-test",
    "/logout": "/logout",
    "/model": "/model",
    "/config": "/config list",
    "/clear": "/clear all",
    "/status": "/status",
    "/approve": "/approve",
    "/version": "/version",
    "/new": "/new sprint",
    "/rename": "/rename sprint-updated",
    "/sessions": "/sessions",
    "/switch": "/switch #1",
    "/history": "/history",
    "/export": "/export --format json",
    "/run": "/run pwd",
    "/init": "/init",
    "/doctor": "/doctor",
    "/pwd": "/pwd",
    "/alias": "/alias ll /files",
    "/unalias": "/unalias ll",
    "/add": "/add missing.txt",
    "/drop": "/drop 1",
    "/files": "/files",
    "/grep": "/grep TODO",
    "/tree": "/tree",
    "/exit": "/exit"
  };

  let exitSeen = false;
  for (const command of commands) {
    const input = samples[command.metadata.name];
    assert.ok(input, `missing sample command line for ${command.metadata.name}`);
    const shouldExit = await session.onLine(input);
    if (command.metadata.name === "/exit") {
      assert.equal(shouldExit, true);
      exitSeen = true;
    } else {
      assert.equal(shouldExit, false);
    }
  }

  assert.equal(exitSeen, true);
  const combinedError = errors.join("");
  assert.doesNotMatch(combinedError, /Unknown command:/);
  assert.match(writes.join(""), /Available commands:/);
});
