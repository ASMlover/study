import test from "node:test";
import assert from "node:assert/strict";
import {
  SpawnSyncOptionsWithStringEncoding,
  SpawnSyncReturns
} from "node:child_process";
import { executeReadOnlyCommand } from "../../src/run-executor";

function mockResult(status: number): SpawnSyncReturns<string> {
  return {
    pid: 1,
    output: [null, "", ""],
    stdout: "",
    stderr: "",
    status,
    signal: null
  };
}

test("shell adapter uses platform-specific invocation with consistent options", () => {
  const invocations: Array<{
    command: string;
    args: readonly string[];
    cwd: string | URL | undefined;
    hasEnv: boolean;
    timeout: number | undefined;
    killSignal: number | NodeJS.Signals | undefined;
  }> = [];

  const spawn = (
    command: string,
    args: readonly string[],
    options: SpawnSyncOptionsWithStringEncoding
  ) => {
    invocations.push({
      command,
      args,
      cwd: options.cwd,
      hasEnv: Boolean(options.env && options.env.MINICLI_PLATFORM_TEST === "1"),
      timeout: options.timeout,
      killSignal: options.killSignal
    });
    return mockResult(0);
  };

  executeReadOnlyCommand("Get-Location", {
    platform: "win32",
    cwd: "C:/Workspace",
    env: { MINICLI_PLATFORM_TEST: "1" },
    timeoutMs: 1234,
    killSignal: "SIGKILL",
    spawn
  });

  executeReadOnlyCommand("pwd", {
    platform: "linux",
    cwd: "/workspace",
    env: { MINICLI_PLATFORM_TEST: "1" },
    timeoutMs: 4321,
    killSignal: "SIGTERM",
    spawn
  });

  assert.equal(invocations.length, 2);

  const windows = invocations[0];
  assert.equal(windows.command, "powershell.exe");
  assert.equal(windows.args[0], "-NoProfile");
  assert.equal(windows.args[1], "-NonInteractive");
  assert.equal(windows.args[2], "-EncodedCommand");
  assert.equal(typeof windows.args[3], "string");
  assert.equal(windows.cwd, "C:/Workspace");
  assert.equal(windows.hasEnv, true);
  assert.equal(windows.timeout, 1234);
  assert.equal(windows.killSignal, "SIGKILL");

  const linux = invocations[1];
  assert.equal(linux.command, "/bin/sh");
  assert.deepEqual(linux.args, ["-lc", "pwd"]);
  assert.equal(linux.cwd, "/workspace");
  assert.equal(linux.hasEnv, true);
  assert.equal(linux.timeout, 4321);
  assert.equal(linux.killSignal, "SIGTERM");
});
