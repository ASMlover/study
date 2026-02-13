import test from "node:test";
import assert from "node:assert/strict";
import { SpawnSyncReturns } from "node:child_process";
import {
  buildPlatformShellInvocation,
  executeReadOnlyCommand,
  isReadOnlyCommandWhitelisted,
  mapShellExitCode
} from "../../src/run-executor";

function mockResult(input: {
  status: number | null;
  stdout?: string;
  stderr?: string;
  error?: Error;
}): SpawnSyncReturns<string> {
  return {
    pid: 123,
    output: [null, input.stdout ?? "", input.stderr ?? ""],
    stdout: input.stdout ?? "",
    stderr: input.stderr ?? "",
    status: input.status,
    signal: null,
    error: input.error
  };
}

test("buildPlatformShellInvocation returns shell adapter by platform", () => {
  assert.deepEqual(buildPlatformShellInvocation("win32"), {
    command: "powershell.exe",
    args: ["-NoProfile", "-NonInteractive", "-Command"]
  });
  assert.deepEqual(buildPlatformShellInvocation("linux"), {
    command: "/bin/sh",
    args: ["-lc"]
  });
});

test("isReadOnlyCommandWhitelisted validates allowlist including mixed pipeline segments", () => {
  assert.equal(isReadOnlyCommandWhitelisted("pwd"), true);
  assert.equal(isReadOnlyCommandWhitelisted("pwd | wc -c"), true);
  assert.equal(isReadOnlyCommandWhitelisted("pwd && rm -rf ."), false);
  assert.equal(isReadOnlyCommandWhitelisted(""), false);
});

test("executeReadOnlyCommand captures stdout and maps successful exit code", () => {
  const result = executeReadOnlyCommand("pwd", {
    spawn: () =>
      mockResult({
        status: 0,
        stdout: "/tmp/work\n"
      })
  });

  assert.equal(result.ok, true);
  assert.equal(result.stdout, "/tmp/work\n");
  assert.equal(result.stderr, "");
  assert.equal(result.exitCode, 0);
});

test("mapShellExitCode maps missing status with process error to code 1", () => {
  const exitCode = mapShellExitCode(
    mockResult({
      status: null,
      error: new Error("spawn failed")
    })
  );

  assert.equal(exitCode, 1);
});

test("executeReadOnlyCommand preserves stderr output and keeps ok result", () => {
  const result = executeReadOnlyCommand("pwd", {
    spawn: () =>
      mockResult({
        status: 0,
        stderr: "warn\n"
      })
  });

  assert.equal(result.ok, true);
  assert.equal(result.stderr, "warn\n");
  assert.equal(result.exitCode, 0);
});

test("executeReadOnlyCommand truncates oversized output", () => {
  const result = executeReadOnlyCommand("pwd", {
    maxOutputLength: 5,
    spawn: () =>
      mockResult({
        status: 0,
        stdout: "abcdefghij"
      })
  });

  assert.equal(result.ok, true);
  assert.equal(result.stdout, "abcde...[truncated]");
  assert.equal(result.stdoutTruncated, true);
});
