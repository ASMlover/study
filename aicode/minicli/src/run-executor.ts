import {
  spawnSync,
  SpawnSyncOptionsWithStringEncoding,
  SpawnSyncReturns
} from "node:child_process";

export const DEFAULT_RUN_OUTPUT_LIMIT = 4096;
export const DEFAULT_RUN_TIMEOUT_MS = 5000;

export const DEFAULT_READ_ONLY_COMMAND_ALLOWLIST = [
  "pwd",
  "get-location",
  "ls",
  "dir",
  "cat",
  "type",
  "head",
  "tail",
  "wc",
  "whoami"
] as const;

export interface ShellInvocation {
  command: string;
  args: string[];
}

export interface RunExecutorOptions {
  cwd?: string;
  platform?: NodeJS.Platform;
  maxOutputLength?: number;
  timeoutMs?: number;
  allowlist?: readonly string[];
  spawn?: (
    command: string,
    args: readonly string[],
    options: SpawnSyncOptionsWithStringEncoding
  ) => SpawnSyncReturns<string>;
}

export interface RunCommandResult {
  ok: boolean;
  stdout: string;
  stderr: string;
  exitCode: number;
  stdoutTruncated: boolean;
  stderrTruncated: boolean;
  error?: string;
}

export function buildPlatformShellInvocation(
  platform: NodeJS.Platform = process.platform
): ShellInvocation {
  if (platform === "win32") {
    return {
      command: "powershell.exe",
      args: ["-NoProfile", "-NonInteractive", "-Command"]
    };
  }

  return {
    command: "/bin/sh",
    args: ["-lc"]
  };
}

function normalizeAllowedToken(token: string): string {
  return token.trim().toLowerCase();
}

function splitCommandSegments(command: string): string[] {
  return command
    .split(/&&|\|\||;|\|/g)
    .map((segment) => segment.trim())
    .filter((segment) => segment.length > 0);
}

function extractLeadingToken(segment: string): string | undefined {
  const trimmed = segment.trim();
  if (trimmed.length === 0) {
    return undefined;
  }

  const match = trimmed.match(/^([^\s]+)/);
  if (!match) {
    return undefined;
  }

  return normalizeAllowedToken(match[1]);
}

export function isReadOnlyCommandWhitelisted(
  command: string,
  allowlist: readonly string[] = DEFAULT_READ_ONLY_COMMAND_ALLOWLIST
): boolean {
  const normalizedAllowlist = new Set(allowlist.map(normalizeAllowedToken));
  const segments = splitCommandSegments(command);
  if (segments.length === 0) {
    return false;
  }

  for (const segment of segments) {
    const token = extractLeadingToken(segment);
    if (!token || !normalizedAllowlist.has(token)) {
      return false;
    }
  }
  return true;
}

export function mapShellExitCode(result: SpawnSyncReturns<string>): number {
  if (typeof result.status === "number") {
    return result.status;
  }
  return result.error ? 1 : 0;
}

function truncateOutput(
  value: string,
  maxLength: number
): { text: string; truncated: boolean } {
  if (value.length <= maxLength) {
    return { text: value, truncated: false };
  }
  return {
    text: `${value.slice(0, maxLength)}...[truncated]`,
    truncated: true
  };
}

export function executeReadOnlyCommand(
  command: string,
  options?: RunExecutorOptions
): RunCommandResult {
  const normalizedCommand = command.trim();
  if (normalizedCommand.length === 0) {
    return {
      ok: false,
      stdout: "",
      stderr: "",
      exitCode: 1,
      stdoutTruncated: false,
      stderrTruncated: false,
      error: "Usage: /run <command>"
    };
  }

  const allowlist = options?.allowlist ?? DEFAULT_READ_ONLY_COMMAND_ALLOWLIST;
  if (!isReadOnlyCommandWhitelisted(normalizedCommand, allowlist)) {
    return {
      ok: false,
      stdout: "",
      stderr: "",
      exitCode: 1,
      stdoutTruncated: false,
      stderrTruncated: false,
      error: "Command is not allowed in read-only mode."
    };
  }

  const maxOutputLength = options?.maxOutputLength ?? DEFAULT_RUN_OUTPUT_LIMIT;
  const timeoutMs = options?.timeoutMs ?? DEFAULT_RUN_TIMEOUT_MS;
  const cwd = options?.cwd ?? process.cwd();
  const platform = options?.platform ?? process.platform;
  const invoke = options?.spawn ?? spawnSync;
  const shell = buildPlatformShellInvocation(platform);

  const result = invoke(shell.command, [...shell.args, normalizedCommand], {
    cwd,
    encoding: "utf8",
    timeout: timeoutMs,
    windowsHide: true,
    maxBuffer: Math.max(maxOutputLength * 2, 1024)
  });

  const stdout = result.stdout ?? "";
  const stderrFromProcess = result.stderr ?? "";
  const spawnedError = result.error ? `${result.error.message}\n` : "";
  const stderr = `${stderrFromProcess}${spawnedError}`;

  const truncatedStdout = truncateOutput(stdout, maxOutputLength);
  const truncatedStderr = truncateOutput(stderr, maxOutputLength);

  return {
    ok: true,
    stdout: truncatedStdout.text,
    stderr: truncatedStderr.text,
    exitCode: mapShellExitCode(result),
    stdoutTruncated: truncatedStdout.truncated,
    stderrTruncated: truncatedStderr.truncated
  };
}
