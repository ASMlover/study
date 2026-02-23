import { formatVersionOutput } from "./version";
import { startRepl } from "./repl";
import {
  formatConfigIssue,
  loadRuntimeConfig,
  LoadedRuntimeConfig,
  resetManagedValueInProjectConfig,
  saveManagedValueToProjectConfig,
  saveApiKeyToGlobalConfig,
  saveModelToProjectConfig
} from "./config";
import { DatabaseConnection, initializeDatabase } from "./db";
import { createRepositories } from "./repository";

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
  initDatabase?: () => { databasePath: string } | void;
  openDatabase?: (databasePath: string) => DatabaseConnection;
}

function openSQLite(databasePath: string): DatabaseConnection {
  const sqliteModule = require("node:sqlite") as {
    DatabaseSync: new (dbPath: string) => DatabaseConnection;
  };
  return new sqliteModule.DatabaseSync(databasePath);
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
    let connection: DatabaseConnection | undefined;
    try {
      const initResult = io.initDatabase ? io.initDatabase() : initializeDatabase();
      const databasePath = initResult?.databasePath;
      if (databasePath) {
        const openDatabase = io.openDatabase ?? openSQLite;
        connection = openDatabase(databasePath);
      }
    } catch (error) {
      const e = error as Error;
      io.stderr(`[db:error] ${e.message}\n`);
      return 1;
    }

    try {
      const repositories = connection ? createRepositories(connection) : undefined;

      const loadedConfig = io.loadConfig ? io.loadConfig() : loadRuntimeConfig();
      for (const issue of loadedConfig.issues) {
        io.stderr(`${formatConfigIssue(issue)}\n`);
      }

      if (io.stdin && io.output) {
        const rl = startRepl({
          input: io.stdin,
          output: io.output,
          stdout: io.stdout,
          stderr: io.stderr
        }, undefined, {
          config: loadedConfig.config,
          saveApiKey: (apiKey: string) =>
            saveApiKeyToGlobalConfig(loadedConfig.globalPath, apiKey),
          saveModel: (model: string) =>
            saveModelToProjectConfig(loadedConfig.projectPath, model),
          saveConfigValue: (key, value) =>
            saveManagedValueToProjectConfig(loadedConfig.projectPath, key, value),
          resetConfigValue: (key) =>
            resetManagedValueInProjectConfig(loadedConfig.projectPath, key),
          sessionRepository: repositories?.sessions,
          messageRepository: repositories?.messages,
          commandHistoryRepository: repositories?.commandHistory,
          runAuditRepository: repositories?.runAudit
        });
        if (connection) {
          rl.on("close", () => {
            connection?.close();
            connection = undefined;
          });
        }
        io.stdout(`Starting ${resolveBinaryName(platform)} interactive shell.\n`);
        return 0;
      }

      io.stdout(`Starting ${resolveBinaryName(platform)} interactive shell.\n`);
      connection?.close();
      connection = undefined;
      return 0;
    } catch (error) {
      const e = error as Error;
      io.stderr(`[runtime:error] ${e.message}\n`);
      connection?.close();
      connection = undefined;
      return 1;
    }
  }

  io.stderr(`Unknown argument: ${parsed.invalidArg ?? "(none)"}\n`);
  return 1;
}
