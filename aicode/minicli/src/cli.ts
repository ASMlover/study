import { formatVersionOutput } from "./version";
import { startRepl } from "./repl";
import {
  formatConfigIssue,
  loadRuntimeConfig,
  LoadedRuntimeConfig,
  saveApiKeyToGlobalConfig,
  saveModelToProjectConfig
} from "./config";

export type CliMode = "version" | "repl" | "invalid";

export interface ParsedArgs {
  mode: CliMode;
  invalidArg?: string;
}

export interface CliIo {
  stdout: (message: string) => void;
  stderr: (message: string) => void;
  stdin?: NodeJS.ReadableStream;
  output?: NodeJS.WritableStream;
  loadConfig?: () => LoadedRuntimeConfig;
}

export function parseArguments(argv: string[]): ParsedArgs {
  if (argv.length === 0) {
    return { mode: "repl" };
  }

  if (argv.length === 1 && (argv[0] === "--version" || argv[0] === "-v")) {
    return { mode: "version" };
  }

  return { mode: "invalid", invalidArg: argv[0] };
}

export function resolveBinaryName(platform: NodeJS.Platform): string {
  return platform === "win32" ? "minicli.exe" : "minicli";
}

export function runCli(
  argv: string[],
  io: CliIo = {
    stdout: (message: string) => process.stdout.write(message),
    stderr: (message: string) => process.stderr.write(message),
    stdin: process.stdin,
    output: process.stdout
  },
  platform: NodeJS.Platform = process.platform
): number {
  const parsed = parseArguments(argv);

  if (parsed.mode === "version") {
    io.stdout(`${formatVersionOutput()}\n`);
    return 0;
  }

  if (parsed.mode === "repl") {
    const loadedConfig = io.loadConfig ? io.loadConfig() : loadRuntimeConfig();
    for (const issue of loadedConfig.issues) {
      io.stderr(`${formatConfigIssue(issue)}\n`);
    }

    if (io.stdin && io.output) {
      startRepl({
        input: io.stdin,
        output: io.output,
        stdout: io.stdout,
        stderr: io.stderr
      }, undefined, {
        config: loadedConfig.config,
        saveApiKey: (apiKey: string) =>
          saveApiKeyToGlobalConfig(loadedConfig.globalPath, apiKey),
        saveModel: (model: string) =>
          saveModelToProjectConfig(loadedConfig.projectPath, model)
      });
      io.stdout(`Starting ${resolveBinaryName(platform)} interactive shell.\n`);
      return 0;
    }

    io.stdout(`Starting ${resolveBinaryName(platform)} interactive shell.\n`);
    return 0;
  }

  io.stderr(`Unknown argument: ${parsed.invalidArg ?? "(none)"}\n`);
  return 1;
}
