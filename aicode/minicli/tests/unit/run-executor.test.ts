import test from "node:test";
import assert from "node:assert/strict";
import { SpawnSyncReturns } from "node:child_process";
import {
  buildPlatformShellInvocation,
  createShellAdapter,
  detectShellPlatform,
  escapeCommandForShell,
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
    args: ["-NoProfile", "-NonInteractive", "-EncodedCommand"]
  });
  assert.deepEqual(buildPlatformShellInvocation("linux"), {
    command: "/bin/sh",
    args: ["-lc"]
  });
});

test("detectShellPlatform detects windows vs posix", () => {
  assert.equal(detectShellPlatform("win32"), "windows");
  assert.equal(detectShellPlatform("linux"), "posix");
  assert.equal(detectShellPlatform("darwin"), "posix");
});

test("escapeCommandForShell encodes windows command and strips posix NUL bytes", () => {
  const windowsEncoded = escapeCommandForShell(
    "Write-Output \"hello world\"",
    "win32"
  );
  assert.equal(
    windowsEncoded,
    Buffer.from("Write-Output \"hello world\"", "utf16le").toString("base64")
  );
  assert.equal(escapeCommandForShell("pwd\0 && whoami", "linux"), "pwd && whoami");
});

test("isReadOnlyCommandWhitelisted validates allowlist including mixed pipeline segments", () => {
  assert.equal(isReadOnlyCommandWhitelisted("pwd"), true);
  assert.equal(isReadOnlyCommandWhitelisted("pwd | wc -c"), true);
  assert.equal(isReadOnlyCommandWhitelisted("pwd && rm -rf ."), false);
  assert.equal(isReadOnlyCommandWhitelisted(""), false);
});

test("executeReadOnlyCommand captures stdout and maps successful exit code", () => {
  let capturedCommand = "";
  let capturedArgs: readonly string[] = [];
  let capturedCwd: string | URL | undefined;
  let capturedEnv: NodeJS.ProcessEnv | undefined;
  let capturedTimeout: number | undefined;
  let capturedKillSignal: NodeJS.Signals | number | undefined;
  const result = executeReadOnlyCommand("pwd", {
    platform: "linux",
    cwd: "/tmp/work",
    env: { MINICLI_TEST: "1" },
    timeoutMs: 42,
    killSignal: "SIGKILL",
    spawn: (command, args, options) => {
      capturedCommand = command;
      capturedArgs = args;
      capturedCwd = options.cwd;
      capturedEnv = options.env;
      capturedTimeout = options.timeout;
      capturedKillSignal = options.killSignal;
      return mockResult({
        status: 0,
        stdout: "/tmp/work\n"
      });
    }
  });

  assert.equal(result.ok, true);
  assert.equal(result.stdout, "/tmp/work\n");
  assert.equal(result.stderr, "");
  assert.equal(result.exitCode, 0);
  assert.equal(capturedCommand, "/bin/sh");
  assert.deepEqual(capturedArgs, ["-lc", "pwd"]);
  assert.equal(capturedCwd, "/tmp/work");
  assert.equal(capturedEnv?.MINICLI_TEST, "1");
  assert.equal(capturedTimeout, 42);
  assert.equal(capturedKillSignal, "SIGKILL");
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

test("mapShellExitCode maps signal-only result to synthetic signal code", () => {
  const exitCode = mapShellExitCode({
    ...mockResult({ status: null }),
    signal: "SIGTERM"
  });
  assert.equal(exitCode, 128);
});

test("executeReadOnlyCommand appends signal information to stderr", () => {
  const result = executeReadOnlyCommand("pwd", {
    spawn: () => ({
      ...mockResult({ status: null }),
      signal: "SIGTERM"
    })
  });

  assert.equal(result.ok, true);
  assert.match(result.stderr, /terminated by signal SIGTERM/);
  assert.equal(result.exitCode, 128);
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

test("createShellAdapter builds escaped windows command args", () => {
  const adapter = createShellAdapter("win32");
  const args = adapter.buildArgs("Write-Output \"hello\"");
  assert.equal(args[0], "-NoProfile");
  assert.equal(args[1], "-NonInteractive");
  assert.equal(args[2], "-EncodedCommand");
  assert.equal(args[3], escapeCommandForShell("Write-Output \"hello\"", "win32"));
});
