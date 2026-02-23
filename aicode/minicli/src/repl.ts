import fs from "node:fs";
import path from "node:path";
import readline from "node:readline";
import {
  CommandParameterSchema,
  CommandRegistry,
  CommandSchemaRegistration,
  registerCommandSchemas
} from "./command-registry";
import {
  isSupportedModelName,
  MutableManagedConfigKey,
  maskSecret,
  ManagedConfigKey,
  RuntimeConfig
} from "./config";
import {
  buildChatRequest,
  ChatMessage,
  GLMOpenAIProvider,
  LLMProvider,
  MockLLMProvider
} from "./provider";
import {
  CommandHistoryRepository,
  MessageRepository,
  RunAuditApprovalStatus,
  RunAuditRecord,
  RunAuditRiskLevel,
  SessionRecord,
  SessionRepository
} from "./repository";
import {
  DEFAULT_RUN_OUTPUT_LIMIT,
  RunCommandResult,
  executeReadOnlyCommand
} from "./run-executor";
import { classifyRunCommandRisk, RunRiskAssessment } from "./run-risk";
import { formatVersionOutput } from "./version";

export const DEFAULT_MAX_INPUT_LENGTH = 1024;
export const DEFAULT_OUTPUT_BUFFER_LIMIT = 4096;
export const DEFAULT_MAX_REPLY_LENGTH = 2048;
export const DEFAULT_MAX_HISTORY_PREVIEW_LENGTH = 120;
export const EMPTY_REPLY_PLACEHOLDER = "[empty reply]";
export const MAX_COMPLETION_USAGE_FREQUENCY = 2_147_483_647;
export const DEFAULT_RUN_CONFIRMATION_TIMEOUT_MS = 15_000;
export const DEFAULT_REQUEST_TOKEN_BUDGET = 4_000;
export const DEFAULT_GREP_MATCH_LIMIT = 20;
export const DEFAULT_GREP_IGNORED_DIRECTORY_NAMES = [
  ".git",
  ".minicli",
  "build",
  "node_modules"
] as const;
export const DEFAULT_TREE_DEPTH_LIMIT = 3;
export const DEFAULT_TREE_IGNORED_DIRECTORY_NAMES =
  DEFAULT_GREP_IGNORED_DIRECTORY_NAMES;

const MANAGED_CONFIG_KEYS: readonly ManagedConfigKey[] = [
  "model",
  "timeoutMs",
  "apiKey",
  "runConfirmationTimeoutMs",
  "requestTokenBudget"
] as const;

const READ_ONLY_CONFIG_KEYS = new Set<ManagedConfigKey>(["apiKey"]);

export type RunConfirmationDecision = "confirm" | "reject" | "invalid";

export type KnownReplCommandKind =
  | "help"
  | "exit"
  | "login"
  | "logout"
  | "model"
  | "config"
  | "clear"
  | "status"
  | "approve"
  | "version"
  | "new"
  | "rename"
  | "sessions"
  | "switch"
  | "history"
  | "export"
  | "run"
  | "init"
  | "doctor"
  | "pwd"
  | "alias"
  | "unalias"
  | "add"
  | "drop"
  | "files"
  | "grep"
  | "tree";

type ReplCommandRegistration = CommandSchemaRegistration<
  KnownReplCommandKind,
  KnownReplCommandKind
>;

function createReplCommandSchema(
  kind: KnownReplCommandKind,
  name: string,
  usage: string,
  description: string,
  acceptsArgs: boolean,
  permission: string,
  aliases: readonly string[] = [],
  parameters: readonly CommandParameterSchema[] = []
): unknown {
  return {
    kind,
    name,
    usage,
    description,
    acceptsArgs,
    permission,
    aliases,
    parameters,
    handler: kind,
    examples: [usage]
  };
}

const DEFAULT_REPL_COMMAND_SCHEMAS: readonly unknown[] = [
  createReplCommandSchema(
    "help",
    "/help",
    "/help",
    "Show this help message",
    false,
    "public"
  ),
  createReplCommandSchema(
    "exit",
    "/exit",
    "/exit",
    "Exit MiniCLI",
    false,
    "public",
    ["/quit"]
  ),
  createReplCommandSchema(
    "login",
    "/login",
    "/login <apiKey>",
    "Save API key to global config",
    true,
    "config_write"
  ),
  createReplCommandSchema(
    "logout",
    "/logout",
    "/logout",
    "Clear API key from current runtime",
    false,
    "config_write"
  ),
  createReplCommandSchema(
    "model",
    "/model",
    "/model [name]",
    "Show or set current model",
    true,
    "state_write"
  ),
  createReplCommandSchema(
    "clear",
    "/clear",
    "/clear [session|all]",
    "Clear current conversation context",
    true,
    "state_write",
    ["/cls"],
    [
      {
        name: "scope",
        type: "string",
        required: false
      }
    ]
  ),
  createReplCommandSchema(
    "config",
    "/config",
    "/config <get|set|list|reset> [key] [value]",
    "Manage runtime configuration keys",
    true,
    "config_write"
  ),
  createReplCommandSchema(
    "status",
    "/status",
    "/status",
    "Show runtime status summary",
    false,
    "public"
  ),
  createReplCommandSchema(
    "approve",
    "/approve",
    "/approve",
    "Approve pending guarded execution",
    false,
    "tool_control"
  ),
  createReplCommandSchema(
    "version",
    "/version",
    "/version",
    "Show MiniCLI version",
    false,
    "public",
    ["/ver"]
  ),
  createReplCommandSchema(
    "new",
    "/new",
    "/new [title]",
    "Create and switch to a new session",
    true,
    "session_write"
  ),
  createReplCommandSchema(
    "rename",
    "/rename",
    "/rename <title>",
    "Rename the current session",
    true,
    "session_write"
  ),
  createReplCommandSchema(
    "sessions",
    "/sessions",
    "/sessions [--limit N] [--offset N] [--q keyword]",
    "List sessions",
    true,
    "session_read"
  ),
  createReplCommandSchema(
    "switch",
    "/switch",
    "/switch <#id|index>",
    "Switch to a specific session",
    true,
    "session_write"
  ),
  createReplCommandSchema(
    "history",
    "/history",
    "/history [--limit N] [--offset N] [--audit] [--status <not_required|approved|rejected|timeout>]",
    "Show messages in current session",
    true,
    "session_read"
  ),
  createReplCommandSchema(
    "export",
    "/export",
    "/export [--format json|md] [--out path] [--force]",
    "Export current session to JSON or Markdown file",
    true,
    "session_read"
  ),
  createReplCommandSchema(
    "run",
    "/run",
    "/run <command>",
    "Execute a read-only shell command",
    true,
    "tool_execute"
  ),
  createReplCommandSchema(
    "init",
    "/init",
    "/init [path]",
    "Initialize MiniCLI project files (placeholder)",
    true,
    "project_write"
  ),
  createReplCommandSchema(
    "doctor",
    "/doctor",
    "/doctor",
    "Run environment diagnostics (placeholder)",
    false,
    "public"
  ),
  createReplCommandSchema(
    "pwd",
    "/pwd",
    "/pwd",
    "Print current working directory",
    false,
    "public"
  ),
  createReplCommandSchema(
    "alias",
    "/alias",
    "/alias <name> <command>",
    "Create command alias (placeholder)",
    true,
    "config_write"
  ),
  createReplCommandSchema(
    "unalias",
    "/unalias",
    "/unalias <name>",
    "Remove command alias (placeholder)",
    true,
    "config_write"
  ),
  createReplCommandSchema(
    "add",
    "/add",
    "/add <path>",
    "Add a text file into context collection",
    true,
    "context_write"
  ),
  createReplCommandSchema(
    "drop",
    "/drop",
    "/drop <path|index> [more paths or indexes]",
    "Remove file(s) from context collection",
    true,
    "context_write"
  ),
  createReplCommandSchema(
    "files",
    "/files",
    "/files [--limit N] [--q keyword]",
    "List context files",
    true,
    "context_read"
  ),
  createReplCommandSchema(
    "grep",
    "/grep",
    "/grep <pattern> [--limit N]",
    "Search project text by regex pattern",
    true,
    "context_read"
  ),
  createReplCommandSchema(
    "tree",
    "/tree",
    "/tree [path] [--depth N]",
    "Show project directory tree",
    true,
    "context_read"
  )
] as const;

const REPL_COMMAND_HANDLER_KEYS: ReadonlySet<KnownReplCommandKind> = new Set([
  "help",
  "exit",
  "login",
  "logout",
  "model",
  "config",
  "clear",
  "status",
  "approve",
  "version",
  "new",
  "rename",
  "sessions",
  "switch",
  "history",
  "export",
  "run",
  "init",
  "doctor",
  "pwd",
  "alias",
  "unalias",
  "add",
  "drop",
  "files",
  "grep",
  "tree"
]);

export function createDefaultReplCommandRegistry(): CommandRegistry<KnownReplCommandKind> {
  const registry = new CommandRegistry<KnownReplCommandKind>();
  registerCommandSchemas<KnownReplCommandKind, KnownReplCommandKind>(
    registry,
    DEFAULT_REPL_COMMAND_SCHEMAS,
    REPL_COMMAND_HANDLER_KEYS
  );
  return registry;
}

function formatHelpText(
  registry: CommandRegistry<KnownReplCommandKind>
): string {
  const lines = registry.list().map((command) => {
    return `${command.metadata.usage} [perm:${command.permission ?? "public"}] ${command.metadata.description}`;
  });
  return `Available commands:\n${lines.join("\n")}\n`;
}

const DEFAULT_COMMAND_REGISTRY = createDefaultReplCommandRegistry();
export const HELP_TEXT = formatHelpText(DEFAULT_COMMAND_REGISTRY);

export function completeReplCommandPrefix(
  prefix: string,
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY,
  usageFrequency: Readonly<Record<string, number>> = {}
): string[] {
  if (!prefix.startsWith("/")) {
    return [];
  }

  const matches: string[] = [];
  for (const command of registry.list()) {
    const tokens = [command.metadata.name, ...(command.aliases ?? [])];
    const hasMatch = tokens.some((token) => token.startsWith(prefix));
    if (hasMatch) {
      matches.push(command.metadata.name);
    }
  }

  return sortCompletionCandidatesByUsageFrequency(matches, usageFrequency);
}

export function formatCompletionCandidates(candidates: string[]): string {
  return `Completions:\n${candidates.join("\n")}\n`;
}

export interface TabCompletionResult {
  line: string;
  cursor: number;
  accepted: boolean;
  candidates: string[];
}

interface InlineCompletionState {
  key: string;
  candidates: string[];
  selectedIndex: number;
  navigated: boolean;
}

interface TokenSpan {
  value: string;
  start: number;
  end: number;
}

interface ReplCompletionContext {
  mode: "command" | "argument";
  commandToken: string;
  commandStart: number;
  commandEnd: number;
  prefix: string;
  tokenStart: number;
  tokenEnd: number;
  args: string[];
  argumentIndex: number;
}

function resolveCommandUsageFrequency(
  commandName: string,
  usageFrequency: Readonly<Record<string, number>>
): number {
  const raw = usageFrequency[commandName];
  if (!Number.isFinite(raw) || raw <= 0) {
    return 0;
  }
  return Math.floor(raw);
}

export function sortCompletionCandidatesByUsageFrequency(
  candidates: string[],
  usageFrequency: Readonly<Record<string, number>>
): string[] {
  return candidates
    .map((candidate, index) => ({
      candidate,
      index,
      frequency: resolveCommandUsageFrequency(candidate, usageFrequency)
    }))
    .sort((left, right) => {
      if (left.frequency !== right.frequency) {
        return right.frequency - left.frequency;
      }
      return left.index - right.index;
    })
    .map((item) => item.candidate);
}

export function incrementCompletionUsageFrequency(
  usageFrequency: Record<string, number>,
  commandName: string,
  maxFrequency: number = MAX_COMPLETION_USAGE_FREQUENCY
): number {
  const previous = resolveCommandUsageFrequency(commandName, usageFrequency);
  const next = Math.min(previous + 1, maxFrequency);
  usageFrequency[commandName] = next;
  return next;
}

function resolveReplCompletionContext(
  line: string,
  cursor: number
): ReplCompletionContext | undefined {
  const boundedCursor = Math.max(0, Math.min(cursor, line.length));
  const leadingWhitespaceLength = line.match(/^\s*/)?.[0].length ?? 0;
  const commandStart = leadingWhitespaceLength;

  if (commandStart >= line.length || line[commandStart] !== "/") {
    return undefined;
  }

  let commandEnd = line.length;
  for (let index = commandStart; index < line.length; index += 1) {
    if (/\s/.test(line[index])) {
      commandEnd = index;
      break;
    }
  }

  if (boundedCursor < commandStart) {
    return undefined;
  }

  const commandToken = line.slice(commandStart, commandEnd);
  if (boundedCursor <= commandEnd) {
    return {
      mode: "command",
      commandToken,
      commandStart,
      commandEnd,
      prefix: line.slice(commandStart, boundedCursor),
      tokenStart: commandStart,
      tokenEnd: commandEnd,
      args: [],
      argumentIndex: 0
    };
  }

  const argsArea = line.slice(commandEnd);
  const spans = tokenizeNonWhitespace(argsArea, commandEnd);
  const active = resolveActiveTokenSpan(spans, boundedCursor);
  const activeSpan = active.span;
  const tokenStart = activeSpan?.start ?? boundedCursor;
  const tokenEnd = activeSpan?.end ?? boundedCursor;
  const prefix = activeSpan
    ? line.slice(activeSpan.start, Math.min(activeSpan.end, boundedCursor))
    : "";

  return {
    mode: "argument",
    commandToken,
    commandStart,
    commandEnd,
    prefix,
    tokenStart,
    tokenEnd,
    args: spans.map((span) => span.value),
    argumentIndex: active.index
  };
}

function tokenizeNonWhitespace(line: string, offset = 0): TokenSpan[] {
  const spans: TokenSpan[] = [];
  const matcher = /\S+/g;
  let match = matcher.exec(line);
  while (match !== null) {
    const value = match[0];
    const start = offset + match.index;
    spans.push({
      value,
      start,
      end: start + value.length
    });
    match = matcher.exec(line);
  }
  return spans;
}

function resolveActiveTokenSpan(
  spans: TokenSpan[],
  cursor: number
): { span?: TokenSpan; index: number } {
  for (let index = 0; index < spans.length; index += 1) {
    const span = spans[index];
    if (cursor >= span.start && cursor <= span.end) {
      return { span, index };
    }
  }

  let nextIndex = 0;
  for (const span of spans) {
    if (span.end <= cursor) {
      nextIndex += 1;
    }
  }
  return { index: nextIndex };
}

function listFilePathCandidates(
  prefix: string,
  cwd: string = process.cwd(),
  directoriesOnly = false
): string[] {
  const normalizedPrefix = prefix.trim();
  const candidateRoot =
    normalizedPrefix.length === 0
      ? cwd
      : path.resolve(cwd, path.dirname(normalizedPrefix));
  const baseName =
    normalizedPrefix.length === 0 ? "" : path.basename(normalizedPrefix);

  let entries: fs.Dirent[];
  try {
    entries = fs.readdirSync(candidateRoot, { withFileTypes: true });
  } catch {
    return [];
  }

  const values: string[] = [];
  for (const entry of entries) {
    if (directoriesOnly && !entry.isDirectory()) {
      continue;
    }
    if (!entry.name.startsWith(baseName)) {
      continue;
    }
    const absolute = path.join(candidateRoot, entry.name);
    const relative = path.relative(cwd, absolute);
    if (relative.length === 0) {
      continue;
    }
    values.push(path.normalize(relative));
  }
  return values.sort((left, right) => left.localeCompare(right));
}

function filterOptionCandidates(
  args: string[],
  argumentIndex: number,
  options: readonly string[]
): string[] {
  const currentValue = args[argumentIndex] ?? "";
  const seen = new Set(
    args.filter((item, index) => item.startsWith("--") && index !== argumentIndex)
  );
  return options.filter((option) => option === currentValue || !seen.has(option));
}

function completeConfigArgument(
  args: string[],
  argumentIndex: number
): string[] {
  const subcommands = ["get", "set", "list", "reset"] as const;
  const keys = [
    "model",
    "timeoutMs",
    "apiKey",
    "runConfirmationTimeoutMs",
    "requestTokenBudget"
  ] as const;
  const modelValues = ["mock-mini", "glm-4", "glm-4-air"] as const;

  if (argumentIndex === 0) {
    return [...subcommands];
  }

  const subcommand = args[0];
  if (subcommand === "get" || subcommand === "set" || subcommand === "reset") {
    if (argumentIndex === 1) {
      return [...keys];
    }
  }

  if (subcommand === "set" && argumentIndex === 2) {
    const key = args[1];
    if (key === "model") {
      return [...modelValues];
    }
    if (key === "timeoutMs" || key === "runConfirmationTimeoutMs") {
      return ["5000", "15000", "30000"];
    }
    if (key === "requestTokenBudget") {
      return ["2000", "4000", "8000"];
    }
  }
  return [];
}

function completeReplArgumentCandidates(
  context: ReplCompletionContext,
  commandName: string
): string[] {
  if (context.mode !== "argument") {
    return [];
  }
  const args = context.args;
  const argumentIndex = context.argumentIndex;

  switch (commandName) {
    case "/clear":
      return argumentIndex === 0 ? ["session", "all"] : [];
    case "/sessions":
      return filterOptionCandidates(args, argumentIndex, [
        "--limit",
        "--offset",
        "--q"
      ]);
    case "/history":
      return filterOptionCandidates(args, argumentIndex, [
        "--limit",
        "--offset",
        "--audit",
        "--status"
      ]);
    case "/files":
      return filterOptionCandidates(args, argumentIndex, ["--limit", "--q"]);
    case "/grep":
      return argumentIndex > 0
        ? filterOptionCandidates(args, argumentIndex, ["--limit"])
        : [];
    case "/tree":
      if ((args[argumentIndex] ?? "").startsWith("--")) {
        return filterOptionCandidates(args, argumentIndex, ["--depth"]);
      }
      if (argumentIndex === 0) {
        return listFilePathCandidates(context.prefix, process.cwd(), true);
      }
      return filterOptionCandidates(args, argumentIndex, ["--depth"]);
    case "/add":
      return argumentIndex === 0
        ? listFilePathCandidates(context.prefix, process.cwd(), false)
        : [];
    case "/init":
      return argumentIndex === 0
        ? listFilePathCandidates(context.prefix, process.cwd(), true)
        : [];
    case "/config":
      return completeConfigArgument(args, argumentIndex);
    case "/export":
      if (argumentIndex > 0 && args[argumentIndex - 1] === "--format") {
        return ["json", "md"];
      }
      if (argumentIndex > 0 && args[argumentIndex - 1] === "--out") {
        return listFilePathCandidates(context.prefix, process.cwd(), false);
      }
      return filterOptionCandidates(args, argumentIndex, [
        "--format",
        "--out",
        "--force"
      ]);
    default:
      return [];
  }
}

function dedupeCandidates(candidates: readonly string[]): string[] {
  return [...new Set(candidates)];
}

function filterCompletionCandidatesByPrefix(
  candidates: readonly string[],
  prefix: string
): string[] {
  if (prefix.length === 0) {
    return [...candidates];
  }
  return candidates.filter((candidate) => candidate.startsWith(prefix));
}

function completeReplAtCursor(
  line: string,
  cursor: number,
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY,
  usageFrequency: Readonly<Record<string, number>> = {}
): { context?: ReplCompletionContext; candidates: string[] } {
  const context = resolveReplCompletionContext(line, cursor);
  if (!context) {
    return {
      context: undefined,
      candidates: []
    };
  }

  if (context.mode === "command") {
    return {
      context,
      candidates: completeReplCommandPrefix(context.prefix, registry, usageFrequency)
    };
  }

  const resolvedCommand = registry.resolve(context.commandToken);
  if (!resolvedCommand) {
    return {
      context,
      candidates: []
    };
  }

  const rawCandidates = completeReplArgumentCandidates(
    context,
    resolvedCommand.metadata.name
  );
  return {
    context,
    candidates: dedupeCandidates(
      filterCompletionCandidatesByPrefix(rawCandidates, context.prefix)
    )
  };
}

function applyResolvedCompletionCandidate(
  line: string,
  context: ReplCompletionContext,
  candidate: string
): { line: string; cursor: number } {
  const beforeToken = line.slice(0, context.tokenStart);
  const afterToken = line.slice(context.tokenEnd);
  const hasWhitespaceSeparator = /^\s/.test(afterToken[0] ?? "");
  const lineWithToken = `${beforeToken}${candidate}${afterToken}`;
  const nextLine =
    afterToken.length === 0
      ? `${lineWithToken} `
      : hasWhitespaceSeparator
        ? lineWithToken
        : `${beforeToken}${candidate} ${afterToken}`;
  const nextCursor =
    afterToken.length === 0
      ? beforeToken.length + candidate.length + 1
      : beforeToken.length + candidate.length;
  return {
    line: nextLine,
    cursor: nextCursor
  };
}

export function acceptReplTabCompletion(
  line: string,
  cursor: number,
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY,
  usageFrequency: Readonly<Record<string, number>> = {}
): TabCompletionResult {
  const boundedCursor = Math.max(0, Math.min(cursor, line.length));
  const resolved = completeReplAtCursor(
    line,
    boundedCursor,
    registry,
    usageFrequency
  );
  const context = resolved.context;
  if (!context) {
    return {
      line,
      cursor: boundedCursor,
      accepted: false,
      candidates: []
    };
  }

  const candidates = resolved.candidates;
  if (candidates.length !== 1) {
    return {
      line,
      cursor: boundedCursor,
      accepted: false,
      candidates
    };
  }

  const completed = applyResolvedCompletionCandidate(
    line,
    context,
    candidates[0]
  );
  return {
    line: completed.line,
    cursor: completed.cursor,
    accepted: true,
    candidates
  };
}

function getInlineCompletionContextKey(context: ReplCompletionContext): string {
  return [
    context.mode,
    context.commandToken,
    String(context.argumentIndex),
    context.prefix,
    String(context.tokenStart),
    String(context.tokenEnd)
  ].join("|");
}

export function resolveInlineTabCompletions(
  line: string,
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY,
  usageFrequency: Readonly<Record<string, number>> = {}
): string {
  let resolved = "";
  let completionState: InlineCompletionState | undefined;
  let index = 0;

  while (index < line.length) {
    if (line.startsWith("\u001b[A", index)) {
      if (completionState) {
        const nextIndex =
          (completionState.selectedIndex - 1 + completionState.candidates.length) %
          completionState.candidates.length;
        completionState = {
          ...completionState,
          selectedIndex: nextIndex,
          navigated: true
        };
      }
      index += 3;
      continue;
    }

    if (line.startsWith("\u001b[B", index)) {
      if (completionState) {
        const nextIndex =
          (completionState.selectedIndex + 1) %
          completionState.candidates.length;
        completionState = {
          ...completionState,
          selectedIndex: nextIndex,
          navigated: true
        };
      }
      index += 3;
      continue;
    }

    if (line[index] === "\u001b") {
      completionState = undefined;
      index += 1;
      continue;
    }

    if (line[index] === "\t") {
      const completion = completeReplAtCursor(
        resolved,
        resolved.length,
        registry,
        usageFrequency
      );
      const context = completion.context;
      if (!context) {
        completionState = undefined;
        index += 1;
        continue;
      }

      const candidates = completion.candidates;
      if (candidates.length === 0) {
        completionState = undefined;
        index += 1;
        continue;
      }

      if (candidates.length === 1) {
        const completed = applyResolvedCompletionCandidate(
          resolved,
          context,
          candidates[0]
        );
        resolved = completed.line;
        completionState = undefined;
        index += 1;
        continue;
      }

      const contextKey = getInlineCompletionContextKey(context);
      const canAcceptSelection =
        completionState?.key === contextKey &&
        completionState.navigated === true;
      if (canAcceptSelection) {
        const activeState = completionState;
        const selected =
          activeState?.candidates[activeState.selectedIndex] ?? candidates[0];
        const completed = applyResolvedCompletionCandidate(
          resolved,
          context,
          selected
        );
        resolved = completed.line;
        completionState = undefined;
      } else {
        completionState = {
          key: contextKey,
          candidates,
          selectedIndex: 0,
          navigated: false
        };
      }
      index += 1;
      continue;
    }

    resolved += line[index];
    completionState = undefined;
    index += 1;
  }

  return resolved;
}

export function createReplReadlineCompleter(
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY,
  usageFrequency: Readonly<Record<string, number>> = {}
): (line: string) => [string[], string] {
  return (line: string) => {
    const completion = completeReplAtCursor(
      line,
      line.length,
      registry,
      usageFrequency
    );
    const context = completion.context;
    if (!context) {
      return [[], ""];
    }

    const accepted = acceptReplTabCompletion(
      line,
      line.length,
      registry,
      usageFrequency
    );
    if (accepted.accepted) {
      const completedToken = accepted.candidates[0];
      return [[completedToken], context.prefix];
    }

    return [completion.candidates, context.prefix];
  };
}

export interface ParsedReplLine {
  kind: "empty" | "message";
  text: string;
  truncated: boolean;
}

export interface ReplWriters {
  stdout: (message: string) => void;
  stderr: (message: string) => void;
}

export interface ReplRuntime extends ReplWriters {
  input: NodeJS.ReadableStream;
  output: NodeJS.WritableStream;
}

export function shouldUseTerminalMode(runtime: ReplRuntime): boolean {
  const inputIsTTY = (runtime.input as Partial<NodeJS.ReadStream>).isTTY === true;
  const outputIsTTY =
    (runtime.output as Partial<NodeJS.WriteStream>).isTTY === true;
  return inputIsTTY || outputIsTTY;
}

export class OutputBuffer {
  private chunks: string[] = [];
  private size = 0;

  constructor(
    private readonly write: (message: string) => void,
    private readonly limit: number = DEFAULT_OUTPUT_BUFFER_LIMIT
  ) {}

  append(message: string): void {
    this.chunks.push(message);
    this.size += message.length;
    if (this.size >= this.limit) {
      this.flush();
    }
  }

  flush(): void {
    if (this.size === 0) {
      return;
    }
    this.write(this.chunks.join(""));
    this.chunks = [];
    this.size = 0;
  }
}

export interface ReplSession {
  onLine: (line: string) => Promise<boolean>;
  onClose: () => void;
  onSigint: () => void;
}

export interface ReplSessionOptions {
  provider?: LLMProvider;
  config?: RuntimeConfig;
  saveApiKey?: (apiKey: string) => void;
  saveModel?: (model: string) => void;
  saveConfigValue?: (key: MutableManagedConfigKey, value: string | number) => void;
  resetConfigValue?: (key: MutableManagedConfigKey) => void;
  maxReplyLength?: number;
  requestTokenBudget?: number;
  sessionRepository?: Pick<SessionRepository, "createSession" | "listSessions">;
  messageRepository?: Pick<
    MessageRepository,
    "createMessage" | "listMessagesBySession"
  >;
  commandHistoryRepository?: Pick<
    CommandHistoryRepository,
    "recordCommand" | "listUsageFrequency"
  >;
  runAuditRepository?: Pick<RunAuditRepositoryLike, "recordAudit" | "listAudits">;
  completionUsageFrequency?: Record<string, number>;
  commandRegistry?: CommandRegistry<KnownReplCommandKind>;
  now?: () => Date;
  runConfirmationTimeoutMs?: number;
  executeRunCommand?: (command: string, cwd: string) => RunCommandResult;
}

interface RunAuditRepositoryLike {
  recordAudit: (input: {
    command: string;
    riskLevel: RunAuditRiskLevel;
    approvalStatus: RunAuditApprovalStatus;
    executed: boolean;
    exitCode?: number | null;
    stdout?: string;
    stderr?: string;
  }) => RunAuditRecord;
  listAudits: (params?: {
    limit?: number;
    offset?: number;
    approvalStatus?: RunAuditApprovalStatus;
  }) => RunAuditRecord[];
}

interface PendingRunConfirmation {
  command: string;
  risk: RunRiskAssessment;
  requestedAtMs: number;
}

export function parseRunConfirmationDecision(
  input: string
): RunConfirmationDecision {
  const normalized = input.trim().toLowerCase();
  if (normalized === "y" || normalized === "yes") {
    return "confirm";
  }
  if (normalized === "n" || normalized === "no") {
    return "reject";
  }
  return "invalid";
}

export function formatRunConfirmationPrompt(
  pending: PendingRunConfirmation,
  timeoutMs: number
): string {
  const matched = pending.risk.matchedRules.join(", ");
  const matchedSuffix = matched.length > 0 ? ` matched: ${matched}.` : "";
  const timeoutSeconds = Math.max(1, Math.ceil(timeoutMs / 1000));
  if (pending.risk.level === "high") {
    return `[run:confirm] DANGER: high-risk command requires explicit approval.${matchedSuffix}\nCommand: ${pending.command}\nReply 'yes' to execute, or 'no' to cancel (timeout ${timeoutSeconds}s).\n`;
  }
  return `[run:confirm] medium-risk command requires confirmation.${matchedSuffix}\nCommand: ${pending.command}\nReply 'yes' to execute, or 'no' to cancel (timeout ${timeoutSeconds}s).\n`;
}

export type ReplCommandMatch =
  | { kind: "help" }
  | { kind: "exit" }
  | { kind: "login"; args: string[] }
  | { kind: "logout" }
  | { kind: "model"; args: string[] }
  | { kind: "config"; args: string[] }
  | { kind: "clear"; args: string[] }
  | { kind: "status" }
  | { kind: "approve" }
  | { kind: "version" }
  | { kind: "new"; args: string[] }
  | { kind: "rename"; args: string[] }
  | { kind: "sessions"; args: string[] }
  | { kind: "switch"; args: string[] }
  | { kind: "history"; args: string[] }
  | { kind: "export"; args: string[] }
  | { kind: "run"; args: string[] }
  | { kind: "init"; args: string[] }
  | { kind: "doctor" }
  | { kind: "pwd" }
  | { kind: "alias"; args: string[] }
  | { kind: "unalias"; args: string[] }
  | { kind: "add"; args: string[] }
  | { kind: "drop"; args: string[] }
  | { kind: "files"; args: string[] }
  | { kind: "grep"; args: string[] }
  | { kind: "tree"; args: string[] }
  | { kind: "unknown"; token: string }
  | { kind: "none" };

function matchResolvedReplCommand(
  kind: KnownReplCommandKind,
  args: string[]
): ReplCommandMatch {
  if (kind === "help") {
    return { kind: "help" };
  }
  if (kind === "exit") {
    return { kind: "exit" };
  }
  if (kind === "login") {
    return { kind: "login", args };
  }
  if (kind === "logout") {
    return { kind: "logout" };
  }
  if (kind === "model") {
    return { kind: "model", args };
  }
  if (kind === "config") {
    return { kind: "config", args };
  }
  if (kind === "clear") {
    return { kind: "clear", args };
  }
  if (kind === "status") {
    return { kind: "status" };
  }
  if (kind === "approve") {
    return { kind: "approve" };
  }
  if (kind === "version") {
    return { kind: "version" };
  }
  if (kind === "new") {
    return { kind: "new", args };
  }
  if (kind === "rename") {
    return { kind: "rename", args };
  }
  if (kind === "sessions") {
    return { kind: "sessions", args };
  }
  if (kind === "switch") {
    return { kind: "switch", args };
  }
  if (kind === "run") {
    return { kind: "run", args };
  }
  if (kind === "export") {
    return { kind: "export", args };
  }
  if (kind === "init") {
    return { kind: "init", args };
  }
  if (kind === "doctor") {
    return { kind: "doctor" };
  }
  if (kind === "pwd") {
    return { kind: "pwd" };
  }
  if (kind === "alias") {
    return { kind: "alias", args };
  }
  if (kind === "unalias") {
    return { kind: "unalias", args };
  }
  if (kind === "add") {
    return { kind: "add", args };
  }
  if (kind === "drop") {
    return { kind: "drop", args };
  }
  if (kind === "files") {
    return { kind: "files", args };
  }
  if (kind === "grep") {
    return { kind: "grep", args };
  }
  if (kind === "tree") {
    return { kind: "tree", args };
  }
  return { kind: "history", args };
}

export type ReplInputClassification =
  | { kind: "empty" }
  | { kind: "command"; command: Exclude<ReplCommandMatch, { kind: "none" }> }
  | { kind: "message"; text: string; truncated: boolean };

export interface RenderedAssistantReply {
  text: string;
  truncated: boolean;
  usedFallback: boolean;
}

export interface TokenBudgetTrimResult {
  messages: ChatMessage[];
  estimatedTokens: number;
  truncated: boolean;
  droppedMessages: number;
  trimmedMessages: number;
}

export interface SessionsCommandOptions {
  limit?: number;
  offset: number;
  query?: string;
}

export interface HistoryCommandOptions {
  limit?: number;
  offset: number;
  audit: boolean;
  approvalStatus?: RunAuditApprovalStatus;
}

export interface FilesCommandOptions {
  limit?: number;
  query?: string;
}

export interface GrepCommandOptions {
  limit: number;
}

export interface TreeCommandOptions {
  depth: number;
  rootPath?: string;
}

export type ExportFormat = "json" | "md";

export interface ExportCommandOptions {
  format: ExportFormat;
  outPath?: string;
  force: boolean;
}

export interface SessionExportPayload {
  exportedAt: string;
  sessionId: number | null;
  sessionTitle: string;
  messageCount: number;
  messages: Array<{
    index: number;
    role: string;
    content: string;
    createdAt?: string;
  }>;
}

export interface GrepMatchRecord {
  filePath: string;
  lineNumber: number;
  lineText: string;
}

export interface GrepSearchResult {
  matches: GrepMatchRecord[];
  truncated: boolean;
}

export interface ProjectTreeNode {
  name: string;
  filePath: string;
  kind: "directory" | "file" | "symlink" | "error";
  error?: string;
  children: ProjectTreeNode[];
}

export interface ProjectTreeResult {
  root: ProjectTreeNode;
}

function normalizeContextPathForDedup(filePath: string): string {
  if (process.platform === "win32") {
    return filePath.toLowerCase();
  }
  return filePath;
}

export function normalizeContextFilePath(
  rawPath: string,
  cwd: string = process.cwd()
): string {
  const trimmed = rawPath.trim();
  if (trimmed.length === 0) {
    throw new Error("Path cannot be empty.");
  }
  return path.normalize(path.resolve(cwd, trimmed));
}

export function isBinaryContent(content: Buffer): boolean {
  const sampleLength = Math.min(content.length, 1024);
  let suspiciousCount = 0;
  for (let index = 0; index < sampleLength; index += 1) {
    const value = content[index];
    if (value === 0) {
      return true;
    }
    const isTab = value === 9;
    const isLineFeed = value === 10;
    const isCarriageReturn = value === 13;
    if (value < 32 && !isTab && !isLineFeed && !isCarriageReturn) {
      suspiciousCount += 1;
    }
  }
  if (sampleLength === 0) {
    return false;
  }
  return suspiciousCount / sampleLength > 0.3;
}

export function validateAddContextFile(
  filePath: string,
  fileSystem: Pick<typeof fs, "existsSync" | "statSync" | "readFileSync"> = fs
): { ok: true } | { ok: false; error: string } {
  if (!fileSystem.existsSync(filePath)) {
    return { ok: false, error: `File does not exist: ${filePath}` };
  }

  let stats: fs.Stats;
  try {
    stats = fileSystem.statSync(filePath);
  } catch (error) {
    const e = error as Error;
    return { ok: false, error: `Cannot access path: ${e.message}` };
  }

  if (stats.isDirectory()) {
    return { ok: false, error: `Path is a directory: ${filePath}` };
  }

  if (!stats.isFile()) {
    return { ok: false, error: `Path is not a regular file: ${filePath}` };
  }

  let content: Buffer;
  try {
    content = fileSystem.readFileSync(filePath);
  } catch (error) {
    const e = error as Error;
    return { ok: false, error: `Failed to read file: ${e.message}` };
  }

  if (isBinaryContent(content)) {
    return { ok: false, error: `Binary file is not supported: ${filePath}` };
  }

  try {
    new TextDecoder("utf-8", { fatal: true }).decode(content);
  } catch {
    return { ok: false, error: `File is not valid UTF-8: ${filePath}` };
  }

  return { ok: true };
}

export function addContextFile(
  existingFiles: string[],
  rawPath: string,
  cwd: string = process.cwd(),
  fileSystem: Pick<typeof fs, "existsSync" | "statSync" | "readFileSync"> = fs
): {
  ok: boolean;
  added: boolean;
  normalizedPath?: string;
  error?: string;
} {
  let normalizedPath = "";
  try {
    normalizedPath = normalizeContextFilePath(rawPath, cwd);
  } catch (error) {
    const e = error as Error;
    return {
      ok: false,
      added: false,
      error: e.message
    };
  }

  const validation = validateAddContextFile(normalizedPath, fileSystem);
  if (!validation.ok) {
    return {
      ok: false,
      added: false,
      error: validation.error
    };
  }

  const nextKey = normalizeContextPathForDedup(normalizedPath);
  const duplicated = existingFiles.some(
    (item) => normalizeContextPathForDedup(item) === nextKey
  );
  if (duplicated) {
    return {
      ok: true,
      added: false,
      normalizedPath
    };
  }

  existingFiles.push(normalizedPath);
  return {
    ok: true,
    added: true,
    normalizedPath
  };
}

function tryResolveDropTargetAsIndex(target: string): number | undefined {
  const trimmed = target.trim();
  if (trimmed.length === 0) {
    return undefined;
  }
  const normalized = trimmed.startsWith("#") ? trimmed.slice(1) : trimmed;
  if (!/^\d+$/.test(normalized)) {
    return undefined;
  }
  const numeric = Number(normalized);
  if (!Number.isInteger(numeric) || numeric <= 0) {
    return undefined;
  }
  return numeric;
}

export function dropContextFiles(
  existingFiles: string[],
  targets: string[],
  cwd: string = process.cwd()
): {
  ok: boolean;
  removed: string[];
  error?: string;
} {
  if (existingFiles.length === 0) {
    return {
      ok: false,
      removed: [],
      error: "Context file collection is empty."
    };
  }
  if (targets.length === 0) {
    return {
      ok: false,
      removed: [],
      error: "Missing drop target."
    };
  }

  const selectedIndexes = new Set<number>();
  for (const target of targets) {
    const index = tryResolveDropTargetAsIndex(target);
    if (index !== undefined) {
      if (index > existingFiles.length) {
        return {
          ok: false,
          removed: [],
          error: `Context index out of range: ${index}`
        };
      }
      selectedIndexes.add(index - 1);
      continue;
    }

    let normalizedPath = "";
    try {
      normalizedPath = normalizeContextFilePath(target, cwd);
    } catch (error) {
      const e = error as Error;
      return {
        ok: false,
        removed: [],
        error: e.message
      };
    }

    const normalizedKey = normalizeContextPathForDedup(normalizedPath);
    const foundIndex = existingFiles.findIndex(
      (item) => normalizeContextPathForDedup(item) === normalizedKey
    );
    if (foundIndex < 0) {
      return {
        ok: false,
        removed: [],
        error: `Context file not found: ${normalizedPath}`
      };
    }
    selectedIndexes.add(foundIndex);
  }

  const removed = [...selectedIndexes]
    .sort((left, right) => left - right)
    .map((index) => existingFiles[index]);
  const descending = [...selectedIndexes].sort((left, right) => right - left);
  for (const index of descending) {
    existingFiles.splice(index, 1);
  }

  return {
    ok: true,
    removed
  };
}

export function formatContextFileList(
  files: string[],
  cwd: string = process.cwd()
): string {
  if (files.length === 0) {
    return "No context files.\n";
  }
  const lines = files.map((filePath, index) => {
    const relative = path.relative(cwd, filePath);
    const displayPath =
      relative.length === 0 || relative.startsWith("..") ? filePath : relative;
    return `[${index + 1}] ${displayPath}`;
  });
  return `Context files:\n${lines.join("\n")}\n`;
}

function resolveContextDisplayPath(filePath: string, cwd: string): string {
  const relative = path.relative(cwd, filePath);
  return relative.length === 0 || relative.startsWith("..") ? filePath : relative;
}

export function abbreviateContextPath(
  displayPath: string,
  maxLength: number = 80
): string {
  if (displayPath.length <= maxLength) {
    return displayPath;
  }
  if (maxLength <= 3) {
    return displayPath.slice(-maxLength);
  }
  return `...${displayPath.slice(-(maxLength - 3))}`;
}

export function parseFilesCommandArgs(
  args: string[]
): { options?: FilesCommandOptions; error?: string } {
  const options: FilesCommandOptions = {};
  for (let index = 0; index < args.length; index += 1) {
    const token = args[index];
    if (token === "--limit") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --limit." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value <= 0) {
        return { error: "--limit must be a positive integer." };
      }
      options.limit = value;
      index += 1;
      continue;
    }

    if (token === "--q") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --q." };
      }
      const query = raw.trim();
      if (query.length === 0) {
        return { error: "--q cannot be empty." };
      }
      options.query = query;
      index += 1;
      continue;
    }

    return { error: `Unknown argument: ${token}` };
  }

  return { options };
}

export function parseExportCommandArgs(
  args: string[]
): { options?: ExportCommandOptions; error?: string } {
  const options: ExportCommandOptions = {
    format: "json",
    force: false
  };

  for (let index = 0; index < args.length; index += 1) {
    const token = args[index];
    if (token === "--force") {
      options.force = true;
      continue;
    }

    if (token === "--format") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --format." };
      }
      if (raw !== "json" && raw !== "md") {
        return { error: "--format must be json or md." };
      }
      options.format = raw;
      index += 1;
      continue;
    }

    if (token === "--out") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --out." };
      }
      const outPath = raw.trim();
      if (outPath.length === 0) {
        return { error: "--out cannot be empty." };
      }
      options.outPath = outPath;
      index += 1;
      continue;
    }

    return { error: `Unknown argument: ${token}` };
  }

  return { options };
}

export function formatFilesCommandOutput(
  files: string[],
  options: FilesCommandOptions = {},
  cwd: string = process.cwd(),
  maxPathLength: number = 80
): string {
  const sorted = [...files].sort((left, right) =>
    resolveContextDisplayPath(left, cwd).localeCompare(
      resolveContextDisplayPath(right, cwd),
      undefined,
      { sensitivity: "base" }
    )
  );
  const filtered =
    options.query === undefined
      ? sorted
      : sorted.filter((filePath) =>
          resolveContextDisplayPath(filePath, cwd)
            .toLowerCase()
            .includes(options.query?.toLowerCase() ?? "")
        );
  const limited =
    options.limit === undefined
      ? filtered
      : filtered.slice(0, Math.max(0, options.limit));
  if (limited.length === 0) {
    return "No context files.\n";
  }
  const lines = limited.map((filePath, index) => {
    const displayPath = resolveContextDisplayPath(filePath, cwd);
    return `[${index + 1}] ${abbreviateContextPath(displayPath, maxPathLength)}`;
  });
  return `Context files:\n${lines.join("\n")}\n`;
}

export function parseGrepCommandArgs(
  args: string[]
): { pattern?: string; options?: GrepCommandOptions; error?: string } {
  const options: GrepCommandOptions = {
    limit: DEFAULT_GREP_MATCH_LIMIT
  };
  let pattern: string | undefined;

  for (let index = 0; index < args.length; index += 1) {
    const token = args[index];
    if (token === "--limit") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --limit." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value <= 0) {
        return { error: "--limit must be a positive integer." };
      }
      options.limit = value;
      index += 1;
      continue;
    }

    if (token.startsWith("--")) {
      return { error: `Unknown argument: ${token}` };
    }

    if (pattern !== undefined) {
      return { error: "Pattern must be a single token." };
    }
    pattern = token;
  }

  if (pattern === undefined || pattern.trim().length === 0) {
    return { error: "Missing grep pattern." };
  }

  return { pattern, options };
}

export function parseTreeCommandArgs(
  args: string[]
): { options?: TreeCommandOptions; error?: string } {
  const options: TreeCommandOptions = {
    depth: DEFAULT_TREE_DEPTH_LIMIT
  };

  for (let index = 0; index < args.length; index += 1) {
    const token = args[index];
    if (token === "--depth") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --depth." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value < 0) {
        return { error: "--depth must be a non-negative integer." };
      }
      options.depth = value;
      index += 1;
      continue;
    }

    if (token.startsWith("--")) {
      return { error: `Unknown argument: ${token}` };
    }

    if (options.rootPath !== undefined) {
      return { error: "Path must be a single token." };
    }
    options.rootPath = token;
  }

  return { options };
}

export function parseGrepPattern(
  rawPattern: string
): { regex?: RegExp; error?: string } {
  const trimmed = rawPattern.trim();
  if (trimmed.length === 0) {
    return { error: "Pattern cannot be empty." };
  }

  const literalMatch = /^\/(.+)\/([a-z]*)$/.exec(trimmed);
  try {
    if (literalMatch) {
      const [, source, flags] = literalMatch;
      return {
        regex: new RegExp(source, flags.replace(/[gy]/g, ""))
      };
    }
    return {
      regex: new RegExp(trimmed)
    };
  } catch (error) {
    const e = error as Error;
    return { error: `Invalid regex pattern: ${e.message}` };
  }
}

function normalizeGrepIgnoreName(name: string): string {
  if (process.platform === "win32") {
    return name.toLowerCase();
  }
  return name;
}

function normalizeTreeIgnoreName(name: string): string {
  if (process.platform === "win32") {
    return name.toLowerCase();
  }
  return name;
}

function shouldIgnoreGrepEntry(
  name: string,
  ignoredDirectoryNames: ReadonlySet<string>
): boolean {
  return ignoredDirectoryNames.has(normalizeGrepIgnoreName(name));
}

function createSafeRegexForLineTest(regex: RegExp): RegExp {
  if (!regex.global && !regex.sticky) {
    return regex;
  }
  return new RegExp(regex.source, regex.flags.replace(/[gy]/g, ""));
}

export function grepProjectText(
  pattern: RegExp,
  options?: {
    cwd?: string;
    limit?: number;
    ignoredDirectoryNames?: readonly string[];
    fileSystem?: Pick<typeof fs, "readdirSync" | "readFileSync">;
  }
): GrepSearchResult {
  const cwd = options?.cwd ?? process.cwd();
  const limit = Math.max(1, Math.floor(options?.limit ?? DEFAULT_GREP_MATCH_LIMIT));
  const ignoredDirectoryNames = new Set(
    (options?.ignoredDirectoryNames ?? DEFAULT_GREP_IGNORED_DIRECTORY_NAMES).map(
      normalizeGrepIgnoreName
    )
  );
  const fileSystem = options?.fileSystem ?? fs;
  const regex = createSafeRegexForLineTest(pattern);
  const matches: GrepMatchRecord[] = [];
  let truncated = false;
  const stack: string[] = [cwd];

  while (stack.length > 0 && !truncated) {
    const currentDir = stack.pop() as string;
    let entries: fs.Dirent[];
    try {
      entries = fileSystem.readdirSync(currentDir, {
        withFileTypes: true
      });
    } catch {
      continue;
    }

    entries.sort((left, right) =>
      left.name.localeCompare(right.name, undefined, {
        sensitivity: "base"
      })
    );

    for (const entry of entries) {
      if (entry.isSymbolicLink()) {
        continue;
      }

      const fullPath = path.join(currentDir, entry.name);
      if (entry.isDirectory()) {
        if (!shouldIgnoreGrepEntry(entry.name, ignoredDirectoryNames)) {
          stack.push(fullPath);
        }
        continue;
      }
      if (!entry.isFile()) {
        continue;
      }

      let content: Buffer;
      try {
        content = fileSystem.readFileSync(fullPath);
      } catch {
        continue;
      }
      if (isBinaryContent(content)) {
        continue;
      }

      let decoded = "";
      try {
        decoded = new TextDecoder("utf-8", { fatal: true }).decode(content);
      } catch {
        continue;
      }

      const lines = decoded.split(/\r?\n/);
      for (let lineIndex = 0; lineIndex < lines.length; lineIndex += 1) {
        regex.lastIndex = 0;
        if (!regex.test(lines[lineIndex])) {
          continue;
        }
        matches.push({
          filePath: fullPath,
          lineNumber: lineIndex + 1,
          lineText: lines[lineIndex]
        });
        if (matches.length >= limit) {
          truncated = true;
          break;
        }
      }

      if (truncated) {
        break;
      }
    }
  }

  return {
    matches,
    truncated
  };
}

interface TreeDirentLike {
  name: string;
  isDirectory: () => boolean;
  isFile: () => boolean;
  isSymbolicLink: () => boolean;
}

interface TreeStatsLike {
  isDirectory: () => boolean;
  isFile: () => boolean;
  isSymbolicLink: () => boolean;
}

interface TreeFileSystemLike {
  lstatSync: (targetPath: string) => TreeStatsLike;
  readdirSync: (
    targetPath: string,
    options: {
      withFileTypes: true;
    }
  ) => TreeDirentLike[];
}

function resolveTreeNodeName(
  filePath: string,
  depth: number,
  rootDisplayName: string
): string {
  if (depth === 0) {
    return rootDisplayName;
  }
  return path.basename(filePath);
}

export function buildProjectTree(options?: {
  cwd?: string;
  rootPath?: string;
  depth?: number;
  ignoredDirectoryNames?: readonly string[];
  fileSystem?: TreeFileSystemLike;
}): ProjectTreeResult {
  const cwd = options?.cwd ?? process.cwd();
  const resolvedRootPath = normalizeContextFilePath(options?.rootPath ?? ".", cwd);
  const depth = Math.max(
    0,
    Math.floor(options?.depth ?? DEFAULT_TREE_DEPTH_LIMIT)
  );
  const rawRootPath = options?.rootPath?.trim();
  const rootDisplayName =
    rawRootPath === undefined || rawRootPath.length === 0 || rawRootPath === "."
      ? "."
      : resolveContextDisplayPath(resolvedRootPath, cwd);
  const ignoredDirectoryNames = new Set(
    (options?.ignoredDirectoryNames ?? DEFAULT_TREE_IGNORED_DIRECTORY_NAMES).map(
      normalizeTreeIgnoreName
    )
  );
  const fileSystem: TreeFileSystemLike = options?.fileSystem ?? {
    lstatSync: (targetPath: string) => fs.lstatSync(targetPath),
    readdirSync: (targetPath: string, readdirOptions: { withFileTypes: true }) =>
      fs.readdirSync(targetPath, readdirOptions)
  };

  const visit = (filePath: string, currentDepth: number): ProjectTreeNode => {
    const nodeName = resolveTreeNodeName(filePath, currentDepth, rootDisplayName);
    let stats: TreeStatsLike;
    try {
      stats = fileSystem.lstatSync(filePath);
    } catch (error) {
      const e = error as Error;
      return {
        name: nodeName,
        filePath,
        kind: "error",
        error: e.message,
        children: []
      };
    }

    if (stats.isSymbolicLink()) {
      return {
        name: nodeName,
        filePath,
        kind: "symlink",
        children: []
      };
    }

    if (!stats.isDirectory()) {
      return {
        name: nodeName,
        filePath,
        kind: "file",
        children: []
      };
    }

    if (currentDepth >= depth) {
      return {
        name: nodeName,
        filePath,
        kind: "directory",
        children: []
      };
    }

    let entries: TreeDirentLike[] = [];
    try {
      entries = fileSystem.readdirSync(filePath, {
        withFileTypes: true
      });
    } catch (error) {
      const e = error as Error;
      return {
        name: nodeName,
        filePath,
        kind: "error",
        error: e.message,
        children: []
      };
    }

    entries.sort((left, right) =>
      left.name.localeCompare(right.name, undefined, {
        sensitivity: "base"
      })
    );

    const children: ProjectTreeNode[] = [];
    for (const entry of entries) {
      if (
        entry.isDirectory() &&
        ignoredDirectoryNames.has(normalizeTreeIgnoreName(entry.name))
      ) {
        continue;
      }
      children.push(visit(path.join(filePath, entry.name), currentDepth + 1));
    }

    return {
      name: nodeName,
      filePath,
      kind: "directory",
      children
    };
  };

  return {
    root: visit(resolvedRootPath, 0)
  };
}

function formatProjectTreeNodeLabel(node: ProjectTreeNode): string {
  if (node.kind === "directory") {
    return `${node.name}/`;
  }
  if (node.kind === "symlink") {
    return `${node.name}@`;
  }
  if (node.kind === "error") {
    return `${node.name} [error: ${node.error ?? "unreadable"}]`;
  }
  return node.name;
}

export function formatProjectTree(tree: ProjectTreeResult): string {
  const lines: string[] = [];
  const renderNode = (
    node: ProjectTreeNode,
    prefix: string,
    isLast: boolean,
    isRoot: boolean
  ): void => {
    const label = formatProjectTreeNodeLabel(node);
    if (isRoot) {
      lines.push(label);
    } else {
      const branch = isLast ? "`-- " : "|-- ";
      lines.push(`${prefix}${branch}${label}`);
    }

    if (node.children.length === 0) {
      return;
    }
    const childPrefix = isRoot ? "" : `${prefix}${isLast ? "    " : "|   "}`;
    for (let index = 0; index < node.children.length; index += 1) {
      renderNode(
        node.children[index],
        childPrefix,
        index === node.children.length - 1,
        false
      );
    }
  };

  renderNode(tree.root, "", true, true);
  return `Tree:\n${lines.join("\n")}\n`;
}

export function formatGrepMatches(
  matches: GrepMatchRecord[],
  cwd: string = process.cwd(),
  maxLineLength: number = 160
): string {
  if (matches.length === 0) {
    return "No matches.\n";
  }
  const lines = matches.map((match, index) => {
    const displayPath = resolveContextDisplayPath(match.filePath, cwd);
    const lineText = truncateHistoryContent(match.lineText, maxLineLength);
    return `[${index + 1}] ${displayPath}:${match.lineNumber}: ${lineText}`;
  });
  return `Grep matches:\n${lines.join("\n")}\n`;
}

function normalizeContextContent(content: string): string {
  return content.replace(/\r\n?/g, "\n");
}

function countContextContentLines(content: string): number {
  if (content.length === 0) {
    return 0;
  }
  return content.split("\n").length;
}

export function buildContextSystemMessage(
  files: string[],
  cwd: string = process.cwd(),
  fileSystem: { readFileSync: (filePath: string) => Buffer | string } = fs
): ChatMessage | undefined {
  if (files.length === 0) {
    return undefined;
  }

  const uniqueFiles: string[] = [];
  const seen = new Set<string>();
  for (const filePath of files) {
    const key = normalizeContextPathForDedup(filePath);
    if (seen.has(key)) {
      continue;
    }
    seen.add(key);
    uniqueFiles.push(filePath);
  }

  if (uniqueFiles.length === 0) {
    return undefined;
  }

  const lines: string[] = ["Context snippets:"];
  for (let index = 0; index < uniqueFiles.length; index += 1) {
    const filePath = uniqueFiles[index];
    const displayPath = resolveContextDisplayPath(filePath, cwd);
    let content = "";
    try {
      const raw = fileSystem.readFileSync(filePath);
      const rawBuffer = Buffer.isBuffer(raw) ? raw : Buffer.from(raw);
      content = new TextDecoder("utf-8", { fatal: true }).decode(rawBuffer);
    } catch {
      content = "[unavailable]";
    }

    const normalizedContent = normalizeContextContent(content);
    const renderedContent = normalizedContent.endsWith("\n")
      ? normalizedContent.slice(0, -1)
      : normalizedContent;
    const lineCount = countContextContentLines(normalizedContent);
    const charCount = normalizedContent.length;

    lines.push(`[${index + 1}] path: ${displayPath}`);
    lines.push(
      `[${index + 1}] meta: lines=${lineCount}, chars=${charCount}, encoding=utf-8`
    );
    lines.push("```text");
    lines.push(renderedContent);
    lines.push("```");
  }

  return {
    role: "system",
    content: lines.join("\n")
  };
}

export type SwitchCommandTarget =
  | { mode: "id"; value: number }
  | { mode: "index"; value: number };

export function createRuntimeProvider(config: RuntimeConfig): LLMProvider {
  if (config.apiKey && config.apiKey.trim().length > 0) {
    return new GLMOpenAIProvider({
      apiKey: config.apiKey
    });
  }
  return new MockLLMProvider();
}

export function parseReplLine(
  line: string,
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH
): ParsedReplLine {
  const normalized = line.trim();
  if (normalized.length === 0) {
    return { kind: "empty", text: "", truncated: false };
  }

  if (normalized.length <= maxInputLength) {
    return { kind: "message", text: normalized, truncated: false };
  }

  return {
    kind: "message",
    text: normalized.slice(0, maxInputLength),
    truncated: true
  };
}

export function matchReplCommand(
  text: string,
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY
): ReplCommandMatch {
  if (!text.startsWith("/")) {
    return { kind: "none" };
  }

  const [token, ...args] = text.split(/\s+/);
  const resolved = registry.resolve(token);
  if (!resolved) {
    return { kind: "unknown", token };
  }

  const definition = resolved as ReplCommandRegistration;
  if (!definition.acceptsArgs && args.length > 0) {
    return { kind: "unknown", token };
  }

  return matchResolvedReplCommand(definition.kind, args);
}

export function formatSessionTimestamp(date: Date): string {
  return date.toISOString().replace("T", " ").slice(0, 19);
}

export function buildDefaultSessionTitle(date: Date): string {
  return `New Session ${formatSessionTimestamp(date)}`;
}

export function parseNewSessionTitle(args: string[]): string | undefined {
  const title = args.join(" ").trim();
  return title.length > 0 ? title : undefined;
}

export type ClearCommandScope = "session" | "all";

export function parseClearCommandArgs(
  args: string[]
): { scope?: ClearCommandScope; error?: string } {
  if (args.length === 0) {
    return { scope: "session" };
  }
  if (args.length > 1) {
    return { error: "Expected at most one scope argument: session|all" };
  }
  const normalized = args[0].trim().toLowerCase();
  if (normalized === "session" || normalized === "all") {
    return { scope: normalized };
  }
  return { error: "Invalid scope, expected session|all" };
}

export function resolveUniqueSessionTitle(
  baseTitle: string,
  existingTitles: string[]
): string {
  if (!existingTitles.includes(baseTitle)) {
    return baseTitle;
  }

  let index = 2;
  while (existingTitles.includes(`${baseTitle} (${index})`)) {
    index += 1;
  }
  return `${baseTitle} (${index})`;
}

export function parseSessionsCommandArgs(
  args: string[]
): { options?: SessionsCommandOptions; error?: string } {
  const options: SessionsCommandOptions = {
    offset: 0
  };

  for (let index = 0; index < args.length; index += 1) {
    const token = args[index];
    if (token === "--limit") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --limit." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value <= 0) {
        return { error: "--limit must be a positive integer." };
      }
      options.limit = value;
      index += 1;
      continue;
    }

    if (token === "--offset") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --offset." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value < 0) {
        return { error: "--offset must be a non-negative integer." };
      }
      options.offset = value;
      index += 1;
      continue;
    }

    if (token === "--q") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --q." };
      }
      const query = raw.trim();
      if (query.length === 0) {
        return { error: "--q cannot be empty." };
      }
      options.query = query;
      index += 1;
      continue;
    }

    return { error: `Unknown argument: ${token}` };
  }

  return { options };
}

export function parseSwitchCommandArgs(
  args: string[]
): { target?: SwitchCommandTarget; error?: string } {
  if (args.length === 0) {
    return { error: "Missing switch target." };
  }
  if (args.length > 1) {
    return { error: "Usage: /switch <#id|index>" };
  }

  const raw = args[0].trim();
  if (raw.length === 0) {
    return { error: "Missing switch target." };
  }

  if (raw.startsWith("#")) {
    const numeric = Number(raw.slice(1));
    if (!Number.isInteger(numeric) || numeric <= 0) {
      return { error: "Session id must be a positive integer like #3." };
    }
    return {
      target: {
        mode: "id",
        value: numeric
      }
    };
  }

  const index = Number(raw);
  if (!Number.isInteger(index) || index <= 0) {
    return { error: "Session index must be a positive integer like 1." };
  }
  return {
    target: {
      mode: "index",
      value: index
    }
  };
}

export function parseHistoryCommandArgs(
  args: string[]
): { options?: HistoryCommandOptions; error?: string } {
  const options: HistoryCommandOptions = {
    offset: 0,
    audit: false
  };
  for (let index = 0; index < args.length; index += 1) {
    const token = args[index];
    if (token === "--audit") {
      options.audit = true;
      continue;
    }

    if (token === "--limit") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --limit." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value <= 0) {
        return { error: "--limit must be a positive integer." };
      }
      options.limit = value;
      index += 1;
      continue;
    }

    if (token === "--offset") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --offset." };
      }
      const value = Number(raw);
      if (!Number.isInteger(value) || value < 0) {
        return { error: "--offset must be a non-negative integer." };
      }
      options.offset = value;
      index += 1;
      continue;
    }

    if (token === "--status") {
      const raw = args[index + 1];
      if (raw === undefined) {
        return { error: "Missing value for --status." };
      }
      if (
        raw !== "not_required" &&
        raw !== "approved" &&
        raw !== "rejected" &&
        raw !== "timeout"
      ) {
        return {
          error:
            "--status must be one of: not_required, approved, rejected, timeout."
        };
      }
      options.approvalStatus = raw;
      index += 1;
      continue;
    }

    return { error: `Unknown argument: ${token}` };
  }

  return { options };
}

export function formatRunAuditList(
  records: RunAuditRecord[],
  maxPreviewLength: number = DEFAULT_MAX_HISTORY_PREVIEW_LENGTH
): string {
  if (records.length === 0) {
    return "No audit records.\n";
  }

  const lines = records.map((record, index) => {
    const stdoutPreview =
      record.stdout.length === 0
        ? "-"
        : truncateHistoryContent(record.stdout, maxPreviewLength);
    const stderrPreview =
      record.stderr.length === 0
        ? "-"
        : truncateHistoryContent(record.stderr, maxPreviewLength);
    const exitCode = record.exitCode === null ? "-" : String(record.exitCode);
    const executed = record.executed ? "yes" : "no";
    return `[${index + 1}] ${record.createdAt} status=${record.approvalStatus} risk=${record.riskLevel} executed=${executed} exit=${exitCode} cmd=${record.command} out=${stdoutPreview} err=${stderrPreview}`;
  });
  return `${lines.join("\n")}\n`;
}

export function sortSessionsByRecent(sessions: SessionRecord[]): SessionRecord[] {
  return [...sessions].sort((left, right) => {
    if (left.updatedAt === right.updatedAt) {
      return right.id - left.id;
    }
    return right.updatedAt.localeCompare(left.updatedAt);
  });
}

export function formatSessionList(
  sessions: SessionRecord[],
  currentSessionId: number | null
): string {
  if (sessions.length === 0) {
    return "No sessions.\n";
  }

  const lines = sessions.map((session, index) => {
    const marker = session.id === currentSessionId ? "*" : " ";
    return `${marker} [${index + 1}] #${session.id} ${session.title} (updated ${session.updatedAt})`;
  });
  return `${lines.join("\n")}\n`;
}

export function truncateHistoryContent(
  content: string,
  maxLength: number = DEFAULT_MAX_HISTORY_PREVIEW_LENGTH
): string {
  if (content.length <= maxLength) {
    return content;
  }
  return `${content.slice(0, maxLength)}...[truncated]`;
}

export function formatHistoryList(
  messages: Array<{
    role: string;
    content: string;
    createdAt?: string;
  }>,
  maxContentLength: number = DEFAULT_MAX_HISTORY_PREVIEW_LENGTH
): string {
  if (messages.length === 0) {
    return "No history.\n";
  }

  const lines = messages.map((message, index) => {
    const content = truncateHistoryContent(message.content, maxContentLength);
    return `[${index + 1}] ${message.role}: ${content}`;
  });
  return `${lines.join("\n")}\n`;
}

export interface ExportFileSystem {
  existsSync: (filePath: string) => boolean;
  mkdirSync: (targetPath: string, options: { recursive?: boolean }) => void;
  writeFileSync: (
    filePath: string,
    data: string,
    options: {
      encoding: BufferEncoding;
      flag: "w" | "wx";
    }
  ) => void;
}

const DEFAULT_EXPORT_FILE_SYSTEM: ExportFileSystem = {
  existsSync: fs.existsSync,
  mkdirSync: fs.mkdirSync,
  writeFileSync: fs.writeFileSync
};

function formatExportTimestamp(now: Date): string {
  const year = now.getFullYear();
  const month = String(now.getMonth() + 1).padStart(2, "0");
  const day = String(now.getDate()).padStart(2, "0");
  const hour = String(now.getHours()).padStart(2, "0");
  const minute = String(now.getMinutes()).padStart(2, "0");
  const second = String(now.getSeconds()).padStart(2, "0");
  return `${year}${month}${day}-${hour}${minute}${second}`;
}

function slugifyExportFileSegment(value: string): string {
  const normalized = value
    .trim()
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "");
  return normalized.length === 0 ? "session" : normalized;
}

export function buildExportFileName(
  format: ExportFormat,
  now: Date,
  sessionId?: number | null,
  sessionTitle?: string
): string {
  const timestamp = formatExportTimestamp(now);
  const idSegment =
    typeof sessionId === "number" && sessionId > 0 ? `s${sessionId}` : "session";
  const titleSegment =
    sessionTitle && sessionTitle.trim().length > 0
      ? slugifyExportFileSegment(sessionTitle)
      : "session";
  return `${idSegment}-${titleSegment}-${timestamp}.${format}`;
}

export function resolveExportOutputPath(
  options: ExportCommandOptions,
  cwd: string,
  now: Date,
  sessionId?: number | null,
  sessionTitle?: string
): string {
  const basePath =
    options.outPath === undefined
      ? path.join(
          cwd,
          buildExportFileName(options.format, now, sessionId, sessionTitle)
        )
      : path.isAbsolute(options.outPath)
        ? options.outPath
        : path.resolve(cwd, options.outPath);
  if (path.extname(basePath).length > 0) {
    return basePath;
  }
  return `${basePath}.${options.format}`;
}

export function buildSessionExportPayload(
  messages: Array<{
    role: string;
    content: string;
    createdAt?: string;
  }>,
  options: {
    exportedAt: string;
    sessionId: number | null;
    sessionTitle: string;
  }
): SessionExportPayload {
  return {
    exportedAt: options.exportedAt,
    sessionId: options.sessionId,
    sessionTitle: options.sessionTitle,
    messageCount: messages.length,
    messages: messages.map((message, index) => ({
      index: index + 1,
      role: message.role,
      content: message.content,
      createdAt: message.createdAt
    }))
  };
}

export function formatSessionExport(
  payload: SessionExportPayload,
  format: ExportFormat
): string {
  if (format === "json") {
    return `${JSON.stringify(payload, null, 2)}\n`;
  }

  const lines: string[] = [
    "# MiniCLI Session Export",
    "",
    `- exportedAt: ${payload.exportedAt}`,
    `- sessionId: ${payload.sessionId === null ? "none" : payload.sessionId}`,
    `- sessionTitle: ${payload.sessionTitle}`,
    `- messageCount: ${payload.messageCount}`,
    ""
  ];

  if (payload.messages.length === 0) {
    lines.push("_No messages._", "");
    return lines.join("\n");
  }

  for (const message of payload.messages) {
    lines.push(`## [${message.index}] ${message.role}`);
    if (message.createdAt) {
      lines.push(`- createdAt: ${message.createdAt}`);
    }
    lines.push("", "```text");
    lines.push(message.content);
    lines.push("```", "");
  }
  return lines.join("\n");
}

export function writeSessionExportFile(
  payload: SessionExportPayload,
  options: ExportCommandOptions,
  writeOptions?: {
    cwd?: string;
    now?: Date;
    fileSystem?: ExportFileSystem;
  }
): { ok: true; outputPath: string } | { ok: false; error: string } {
  const cwd = writeOptions?.cwd ?? process.cwd();
  const now = writeOptions?.now ?? new Date();
  const fileSystem = writeOptions?.fileSystem ?? DEFAULT_EXPORT_FILE_SYSTEM;
  const outputPath = resolveExportOutputPath(
    options,
    cwd,
    now,
    payload.sessionId,
    payload.sessionTitle
  );

  if (!options.force && fileSystem.existsSync(outputPath)) {
    return {
      ok: false,
      error: "output file already exists. Use --force to overwrite."
    };
  }

  const content = formatSessionExport(payload, options.format);
  try {
    fileSystem.mkdirSync(path.dirname(outputPath), { recursive: true });
    fileSystem.writeFileSync(outputPath, content, {
      encoding: "utf8",
      flag: options.force ? "w" : "wx"
    });
  } catch (error) {
    const e = error as NodeJS.ErrnoException;
    if (e.code === "EEXIST") {
      return {
        ok: false,
        error: "output file already exists. Use --force to overwrite."
      };
    }
    if (e.code === "EACCES" || e.code === "EPERM") {
      return {
        ok: false,
        error: `permission denied for output path: ${outputPath}`
      };
    }
    return {
      ok: false,
      error: e.message
    };
  }

  return { ok: true, outputPath };
}

export function classifyReplInput(
  line: string,
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH,
  registry: CommandRegistry<KnownReplCommandKind> = DEFAULT_COMMAND_REGISTRY
): ReplInputClassification {
  const parsed = parseReplLine(line, maxInputLength);
  if (parsed.kind === "empty") {
    return { kind: "empty" };
  }

  const command = matchReplCommand(parsed.text, registry);
  if (command.kind !== "none") {
    return { kind: "command", command };
  }

  return {
    kind: "message",
    text: parsed.text,
    truncated: parsed.truncated
  };
}

export function renderAssistantReply(
  content: string,
  maxReplyLength: number = DEFAULT_MAX_REPLY_LENGTH
): RenderedAssistantReply {
  const trimmed = content.trim();
  const baseText = trimmed.length === 0 ? EMPTY_REPLY_PLACEHOLDER : content;
  const usedFallback = trimmed.length === 0;

  if (baseText.length <= maxReplyLength) {
    return {
      text: baseText,
      truncated: false,
      usedFallback
    };
  }

  return {
    text: `${baseText.slice(0, maxReplyLength)}...[truncated]`,
    truncated: true,
    usedFallback
  };
}

export function estimateMessageTokens(message: ChatMessage): number {
  const normalizedLength = message.content.length;
  const contentTokens = Math.ceil(normalizedLength / 4);
  return 1 + contentTokens;
}

export function estimateChatRequestTokens(messages: ChatMessage[]): number {
  return messages.reduce((sum, message) => sum + estimateMessageTokens(message), 0);
}

function clipContentToTokenBudget(content: string, tokenBudget: number): string {
  const maxChars = Math.max(0, tokenBudget * 4);
  if (content.length <= maxChars) {
    return content;
  }
  if (maxChars === 0) {
    return "";
  }
  const suffix = "\n[...truncated by token budget]";
  if (maxChars <= suffix.length) {
    return content.slice(0, maxChars);
  }
  return `${content.slice(0, maxChars - suffix.length)}${suffix}`;
}

function trimMessageToBudget(
  message: ChatMessage,
  budget: number
): ChatMessage | undefined {
  if (budget <= 0) {
    return undefined;
  }
  const availableContentTokens = Math.max(0, budget - 1);
  const clipped = clipContentToTokenBudget(message.content, availableContentTokens);
  const candidate: ChatMessage = {
    role: message.role,
    content: clipped
  };
  if (estimateMessageTokens(candidate) <= budget) {
    return candidate;
  }
  return {
    role: message.role,
    content: ""
  };
}

function normalizeTokenBudget(tokenBudget: number): number {
  if (!Number.isFinite(tokenBudget) || tokenBudget <= 0) {
    return DEFAULT_REQUEST_TOKEN_BUDGET;
  }
  return Math.floor(tokenBudget);
}

function normalizeRunConfirmationTimeout(timeoutMs: number): number {
  if (!Number.isFinite(timeoutMs) || timeoutMs <= 0) {
    return DEFAULT_RUN_CONFIRMATION_TIMEOUT_MS;
  }
  return Math.floor(timeoutMs);
}

function isManagedConfigKey(value: string): value is ManagedConfigKey {
  return MANAGED_CONFIG_KEYS.includes(value as ManagedConfigKey);
}

function isReadOnlyConfigKey(key: ManagedConfigKey): boolean {
  return READ_ONLY_CONFIG_KEYS.has(key);
}

export function trimMessagesToTokenBudget(
  messages: ChatMessage[],
  tokenBudget: number = DEFAULT_REQUEST_TOKEN_BUDGET
): TokenBudgetTrimResult {
  const normalizedBudget = normalizeTokenBudget(tokenBudget);
  const initialTokens = estimateChatRequestTokens(messages);
  if (initialTokens <= normalizedBudget) {
    return {
      messages: [...messages],
      estimatedTokens: initialTokens,
      truncated: false,
      droppedMessages: 0,
      trimmedMessages: 0
    };
  }

  const latestUserIndex = (() => {
    for (let index = messages.length - 1; index >= 0; index -= 1) {
      if (messages[index].role === "user") {
        return index;
      }
    }
    return -1;
  })();

  const selected = new Map<number, ChatMessage>();
  let remaining = normalizedBudget;
  let trimmedMessages = 0;

  const includeCandidate = (
    index: number,
    allowTrim: boolean
  ): void => {
    if (selected.has(index) || remaining <= 0) {
      return;
    }
    const message = messages[index];
    const tokens = estimateMessageTokens(message);
    if (tokens <= remaining) {
      selected.set(index, { ...message });
      remaining -= tokens;
      return;
    }
    if (!allowTrim) {
      return;
    }
    const trimmed = trimMessageToBudget(message, remaining);
    if (!trimmed) {
      return;
    }
    selected.set(index, trimmed);
    remaining -= estimateMessageTokens(trimmed);
    trimmedMessages += 1;
  };

  if (latestUserIndex >= 0) {
    includeCandidate(latestUserIndex, true);
  }

  for (let index = 0; index < messages.length; index += 1) {
    if (messages[index].role === "system") {
      includeCandidate(index, true);
    }
  }

  for (let index = messages.length - 1; index >= 0; index -= 1) {
    if (index === latestUserIndex) {
      continue;
    }
    if (messages[index].role === "system") {
      continue;
    }
    includeCandidate(index, false);
  }

  const keptIndexes = [...selected.keys()].sort((left, right) => left - right);
  const keptMessages = keptIndexes.map((index) => selected.get(index) as ChatMessage);
  const estimatedTokens = estimateChatRequestTokens(keptMessages);
  const droppedMessages = Math.max(0, messages.length - keptMessages.length);
  const truncated = droppedMessages > 0 || trimmedMessages > 0;

  return {
    messages: keptMessages,
    estimatedTokens,
    truncated,
    droppedMessages,
    trimmedMessages
  };
}

export function formatTokenBudgetTrimNotice(
  result: TokenBudgetTrimResult,
  tokenBudget: number
): string {
  return `[context:warn] request exceeded token budget ${tokenBudget}; dropped ${result.droppedMessages} message(s), trimmed ${result.trimmedMessages} message(s), estimated tokens ${result.estimatedTokens}.\n`;
}

export function createReplSession(
  writers: ReplWriters,
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH,
  options?: ReplSessionOptions
): ReplSession {
  const buffer = new OutputBuffer(writers.stdout);
  const hasFixedProvider = options?.provider !== undefined;
  const config: RuntimeConfig = options?.config ?? {
    model: "mock-mini",
    timeoutMs: 30000
  };
  let provider = options?.provider ?? createRuntimeProvider(config);
  const maxReplyLength = options?.maxReplyLength ?? DEFAULT_MAX_REPLY_LENGTH;
  let requestTokenBudget = normalizeTokenBudget(
    options?.requestTokenBudget ??
      config.requestTokenBudget ??
      DEFAULT_REQUEST_TOKEN_BUDGET
  );
  const sessionRepository = options?.sessionRepository;
  const messageRepository = options?.messageRepository;
  const commandHistoryRepository = options?.commandHistoryRepository;
  const runAuditRepository = options?.runAuditRepository;
  const commandRegistry = options?.commandRegistry ?? DEFAULT_COMMAND_REGISTRY;
  const completionUsageFrequency: Record<string, number> =
    options?.completionUsageFrequency ??
    commandHistoryRepository?.listUsageFrequency() ??
    {};
  const helpText = formatHelpText(commandRegistry);
  const now = options?.now ?? (() => new Date());
  let runConfirmationTimeoutMs = normalizeRunConfirmationTimeout(
    options?.runConfirmationTimeoutMs ??
      config.runConfirmationTimeoutMs ??
      DEFAULT_RUN_CONFIRMATION_TIMEOUT_MS
  );
  const initialManagedConfigValues: Record<MutableManagedConfigKey, string | number> = {
    model: config.model,
    timeoutMs: config.timeoutMs,
    runConfirmationTimeoutMs,
    requestTokenBudget
  };
  const executeRun =
    options?.executeRunCommand ??
    ((command: string, cwd: string) =>
      executeReadOnlyCommand(command, {
        cwd
      }));
  const conversation: ChatMessage[] = [];
  const contextFiles: string[] = [];
  let currentSessionId: number | null = null;
  let pendingRunConfirmation: PendingRunConfirmation | undefined;

  const recordRunAudit = (input: {
    command: string;
    riskLevel: RunAuditRiskLevel;
    approvalStatus: RunAuditApprovalStatus;
    executed: boolean;
    exitCode?: number | null;
    stdout?: string;
    stderr?: string;
  }): void => {
    if (!runAuditRepository) {
      return;
    }
    try {
      runAuditRepository.recordAudit(input);
    } catch (error) {
      const e = error as Error;
      writers.stderr(`[audit:error] ${e.message}\n`);
    }
  };

  const executeAndRenderRunCommand = (
    command: string
  ): { result: RunCommandResult } => {
    const result = executeRun(command, process.cwd());
    if (!result.ok) {
      writers.stderr(`[run:error] ${result.error}\n`);
      return { result };
    }

    if (result.stdout.length > 0) {
      writers.stdout(
        result.stdout.endsWith("\n") ? result.stdout : `${result.stdout}\n`
      );
    }
    if (result.stderr.length > 0) {
      writers.stderr(result.stderr.endsWith("\n") ? result.stderr : `${result.stderr}\n`);
    }
    if (result.stdoutTruncated) {
      writers.stderr(
        `[run:warn] stdout exceeded ${DEFAULT_RUN_OUTPUT_LIMIT} chars; truncated.\n`
      );
    }
    if (result.stderrTruncated) {
      writers.stderr(
        `[run:warn] stderr exceeded ${DEFAULT_RUN_OUTPUT_LIMIT} chars; truncated.\n`
      );
    }
    if (result.exitCode !== 0) {
      writers.stderr(`[run:error] command exited with code ${result.exitCode}.\n`);
    }
    return { result };
  };

  const createAndSwitchSession = (requestedTitle?: string): {
    id?: number;
    title: string;
  } => {
    const baseTitle = requestedTitle ?? buildDefaultSessionTitle(now());
    if (!sessionRepository) {
      currentSessionId = null;
      return { title: baseTitle };
    }

    const existingTitles = sessionRepository
      .listSessions()
      .map((session) => session.title);
    const uniqueTitle = resolveUniqueSessionTitle(baseTitle, existingTitles);
    const created = sessionRepository.createSession(uniqueTitle);
    currentSessionId = created.id;
    return { id: created.id, title: created.title };
  };

  const formatStatusOutput = (): string => {
    const providerId = provider.id;
    const sessionLabel =
      currentSessionId === null ? "none" : `#${currentSessionId}`;
    return [
      "Runtime status:",
      `model: ${config.model}`,
      `timeoutMs: ${config.timeoutMs}`,
      `provider: ${providerId}`,
      `session: ${sessionLabel}`,
      `runConfirmationTimeoutMs: ${runConfirmationTimeoutMs}`,
      `requestTokenBudget: ${requestTokenBudget}`,
      `contextFiles: ${contextFiles.length}`,
      `pendingRunConfirmation: ${pendingRunConfirmation ? "yes" : "no"}`,
      ""
    ].join("\n");
  };

  const commandHandlers: Record<
    KnownReplCommandKind,
    (args: string[]) => Promise<boolean>
  > = {
    help: async () => {
      writers.stdout(helpText);
      return false;
    },
    exit: async () => {
      writers.stdout("Bye.\n");
      return true;
    },
    login: async (args) => {
      if (args.length !== 1) {
        writers.stderr("Usage: /login <apiKey>\n");
        return false;
      }

      const apiKey = args[0].trim();
      if (apiKey.length === 0) {
        writers.stderr("API key cannot be empty.\n");
        return false;
      }

      try {
        options?.saveApiKey?.(apiKey);
      } catch (error) {
        const e = error as Error;
        writers.stderr(`[config:error] ${e.message}\n`);
        return false;
      }

      config.apiKey = apiKey;
      if (!hasFixedProvider) {
        provider = createRuntimeProvider(config);
      }
      writers.stdout(`API key saved: ${maskSecret(apiKey)}\n`);
      return false;
    },
    logout: async () => {
      config.apiKey = undefined;
      if (!hasFixedProvider) {
        provider = createRuntimeProvider(config);
      }
      writers.stdout("API key cleared from runtime.\n");
      return false;
    },
    model: async (args) => {
      if (args.length === 0) {
        writers.stdout(`Current model: ${config.model}\n`);
        return false;
      }

      if (args.length > 1) {
        writers.stderr("Usage: /model [name]\n");
        return false;
      }

      const nextModel = args[0].trim();
      if (!isSupportedModelName(nextModel)) {
        writers.stderr("Model name cannot be empty.\n");
        return false;
      }

      try {
        options?.saveModel?.(nextModel);
      } catch (error) {
        const e = error as Error;
        writers.stderr(`[config:error] ${e.message}\n`);
        return false;
      }

      config.model = nextModel;
      writers.stdout(`Model updated: ${config.model}\n`);
      return false;
    },
    config: async (args) => {
      if (args.length === 0) {
        writers.stderr(
          "Usage: /config <get|set|list|reset> [key] [value]\n"
        );
        return false;
      }
      const subcommand = args[0];
      if (
        subcommand !== "get" &&
        subcommand !== "set" &&
        subcommand !== "list" &&
        subcommand !== "reset"
      ) {
        writers.stderr(
          "[config:error] unknown subcommand. Use get|set|list|reset.\n"
        );
        return false;
      }

      if (subcommand === "list") {
        writers.stdout(
          [
            "Config keys:",
            "- model",
            "- timeoutMs",
            "- apiKey (read-only: use /login)",
            "- runConfirmationTimeoutMs",
            "- requestTokenBudget",
            ""
          ].join("\n")
        );
        return false;
      }

      if (args.length < 2) {
        writers.stderr(`[config:error] ${subcommand} requires a key.\n`);
        return false;
      }

      const keyToken = args[1];
      if (!isManagedConfigKey(keyToken)) {
        writers.stderr(`[config:error] unknown key "${keyToken}".\n`);
        return false;
      }

      const readValue = (key: ManagedConfigKey): string | number => {
        if (key === "model") {
          return config.model;
        }
        if (key === "timeoutMs") {
          return config.timeoutMs;
        }
        if (key === "runConfirmationTimeoutMs") {
          return runConfirmationTimeoutMs;
        }
        if (key === "requestTokenBudget") {
          return requestTokenBudget;
        }
        return config.apiKey ? maskSecret(config.apiKey) : "(empty)";
      };

      if (subcommand === "get") {
        if (args.length !== 2) {
          writers.stderr("Usage: /config get <key>\n");
          return false;
        }
        writers.stdout(`${keyToken}=${readValue(keyToken)}\n`);
        return false;
      }

      if (isReadOnlyConfigKey(keyToken)) {
        writers.stderr(
          `[config:error] key "${keyToken}" is read-only. Use /login or /logout.\n`
        );
        return false;
      }

      if (subcommand === "set") {
        if (args.length < 3) {
          writers.stderr("Usage: /config set <key> <value>\n");
          return false;
        }

        const valueToken = args.slice(2).join(" ").trim();
        let parsedValue: string | number;
        if (keyToken === "model") {
          if (!isSupportedModelName(valueToken)) {
            writers.stderr("[config:error] model cannot be empty.\n");
            return false;
          }
          parsedValue = valueToken;
        } else {
          const numeric = Number(valueToken);
          if (!Number.isFinite(numeric) || numeric <= 0) {
            writers.stderr(
              `[config:error] ${keyToken} must be a positive number.\n`
            );
            return false;
          }
          parsedValue = Math.floor(numeric);
        }

        try {
          options?.saveConfigValue?.(keyToken as MutableManagedConfigKey, parsedValue);
        } catch (error) {
          const e = error as Error;
          writers.stderr(`[config:error] ${e.message}\n`);
          return false;
        }

        if (keyToken === "model") {
          config.model = parsedValue as string;
          if (!hasFixedProvider) {
            provider = createRuntimeProvider(config);
          }
        } else if (keyToken === "timeoutMs") {
          config.timeoutMs = parsedValue as number;
          if (!hasFixedProvider) {
            provider = createRuntimeProvider(config);
          }
        } else if (keyToken === "runConfirmationTimeoutMs") {
          runConfirmationTimeoutMs = normalizeRunConfirmationTimeout(
            parsedValue as number
          );
          config.runConfirmationTimeoutMs = runConfirmationTimeoutMs;
        } else if (keyToken === "requestTokenBudget") {
          requestTokenBudget = normalizeTokenBudget(parsedValue as number);
          config.requestTokenBudget = requestTokenBudget;
        }

        writers.stdout(`[config] updated ${keyToken}=${readValue(keyToken)}\n`);
        return false;
      }

      if (args.length !== 2) {
        writers.stderr("Usage: /config reset <key>\n");
        return false;
      }

      try {
        options?.resetConfigValue?.(keyToken as MutableManagedConfigKey);
      } catch (error) {
        const e = error as Error;
        writers.stderr(`[config:error] ${e.message}\n`);
        return false;
      }

      if (keyToken === "model") {
        config.model = String(initialManagedConfigValues.model);
        if (!hasFixedProvider) {
          provider = createRuntimeProvider(config);
        }
      } else if (keyToken === "timeoutMs") {
        config.timeoutMs = Number(initialManagedConfigValues.timeoutMs);
        if (!hasFixedProvider) {
          provider = createRuntimeProvider(config);
        }
      } else if (keyToken === "runConfirmationTimeoutMs") {
        runConfirmationTimeoutMs = normalizeRunConfirmationTimeout(
          Number(initialManagedConfigValues.runConfirmationTimeoutMs)
        );
        config.runConfirmationTimeoutMs = runConfirmationTimeoutMs;
      } else if (keyToken === "requestTokenBudget") {
        requestTokenBudget = normalizeTokenBudget(
          Number(initialManagedConfigValues.requestTokenBudget)
        );
        config.requestTokenBudget = requestTokenBudget;
      }

      writers.stdout(`[config] reset ${keyToken}=${readValue(keyToken)}\n`);
      return false;
    },
    clear: async (args) => {
      const parsed = parseClearCommandArgs(args);
      if (!parsed.scope) {
        writers.stderr(`${parsed.error}\nUsage: /clear [session|all]\n`);
        return false;
      }
      conversation.length = 0;
      if (parsed.scope === "all") {
        contextFiles.length = 0;
        currentSessionId = null;
      }
      writers.stdout(
        `[clear] cleared ${parsed.scope === "all" ? "session/context/session-pointer" : "session context"}.\n`
      );
      return false;
    },
    status: async () => {
      writers.stdout(formatStatusOutput());
      return false;
    },
    approve: async () => {
      if (!pendingRunConfirmation) {
        writers.stderr("[run:confirm] no pending command to approve.\n");
        return false;
      }
      const approved = pendingRunConfirmation;
      pendingRunConfirmation = undefined;
      const { result } = executeAndRenderRunCommand(approved.command);
      recordRunAudit({
        command: approved.command,
        riskLevel: approved.risk.level,
        approvalStatus: "approved",
        executed: result.ok,
        exitCode: result.ok ? result.exitCode : null,
        stdout: result.stdout,
        stderr: result.ok ? result.stderr : result.error
      });
      return false;
    },
    version: async () => {
      writers.stdout(`${formatVersionOutput()}\n`);
      return false;
    },
    new: async (args) => {
      const requestedTitle = parseNewSessionTitle(args);
      try {
        const next = createAndSwitchSession(requestedTitle);
        conversation.length = 0;
        if (next.id === undefined) {
          writers.stdout(`Started new session: ${next.title}\n`);
        } else {
          writers.stdout(`Switched to session #${next.id}: ${next.title}\n`);
        }
      } catch (error) {
        const e = error as Error;
        writers.stderr(`[session:error] ${e.message}\n`);
      }
      return false;
    },
    rename: async (args) => {
      if (args.length === 0) {
        writers.stderr("Usage: /rename <title>\n");
        return false;
      }
      const title = args.join(" ").trim();
      if (title.length === 0) {
        writers.stderr("Usage: /rename <title>\n");
        return false;
      }
      if (currentSessionId === null) {
        writers.stderr("[session:error] no active session to rename.\n");
        return false;
      }
      writers.stdout(
        `[session] rename requested for #${currentSessionId} -> ${title}\n`
      );
      return false;
    },
    sessions: async (args) => {
      if (!sessionRepository) {
        writers.stderr("Session storage is unavailable.\n");
        return false;
      }

      const parsed = parseSessionsCommandArgs(args);
      if (!parsed.options) {
        writers.stderr(
          `${parsed.error}\nUsage: /sessions [--limit N] [--offset N] [--q keyword]\n`
        );
        return false;
      }
      const options = parsed.options;

      const ordered = sortSessionsByRecent(sessionRepository.listSessions());
      const query = options.query;
      const filtered =
        query === undefined
          ? ordered
          : ordered.filter((session) =>
              session.title.toLowerCase().includes(query.toLowerCase())
            );
      const sliced =
        options.limit === undefined
          ? filtered.slice(options.offset)
          : filtered.slice(options.offset, options.offset + options.limit);
      writers.stdout(formatSessionList(sliced, currentSessionId));
      return false;
    },
    switch: async (args) => {
      if (!sessionRepository) {
        writers.stderr("Session storage is unavailable.\n");
        return false;
      }

      const parsed = parseSwitchCommandArgs(args);
      if (!parsed.target) {
        writers.stderr(`${parsed.error}\nUsage: /switch <#id|index>\n`);
        return false;
      }

      const ordered = sortSessionsByRecent(sessionRepository.listSessions());
      const target =
        parsed.target.mode === "id"
          ? ordered.find((session) => session.id === parsed.target?.value)
          : ordered[parsed.target.value - 1];
      if (!target) {
        writers.stderr("[session:error] target session does not exist.\n");
        return false;
      }

      if (target.id === currentSessionId) {
        writers.stdout(`Already in session #${target.id}: ${target.title}\n`);
        return false;
      }

      currentSessionId = target.id;
      conversation.length = 0;
      writers.stdout(`Switched to session #${target.id}: ${target.title}\n`);
      return false;
    },
    history: async (args) => {
      const parsed = parseHistoryCommandArgs(args);
      if (!parsed.options) {
        writers.stderr(
          `${parsed.error}\nUsage: /history [--limit N] [--offset N] [--audit] [--status <not_required|approved|rejected|timeout>]\n`
        );
        return false;
      }
      const options = parsed.options;

      if (options.audit) {
        if (!runAuditRepository) {
          writers.stderr("Audit storage is unavailable.\n");
          return false;
        }
        const audits = runAuditRepository.listAudits({
          limit: options.limit,
          offset: options.offset,
          approvalStatus: options.approvalStatus
        });
        writers.stdout(formatRunAuditList(audits));
        return false;
      }

      if (!messageRepository) {
        writers.stderr("Session storage is unavailable.\n");
        return false;
      }

      if (currentSessionId === null) {
        writers.stdout("No active session.\n");
        return false;
      }

      const allMessages = messageRepository.listMessagesBySession(currentSessionId);
      const selected =
        options.limit === undefined
          ? allMessages.slice(options.offset)
          : options.offset === 0
            ? allMessages.slice(-options.limit)
            : allMessages.slice(options.offset, options.offset + options.limit);
      writers.stdout(formatHistoryList(selected));
      return false;
    },
    export: async (args) => {
      const parsed = parseExportCommandArgs(args);
      if (!parsed.options) {
        writers.stderr(
          `${parsed.error}\nUsage: /export [--format json|md] [--out path] [--force]\n`
        );
        return false;
      }
      if (!messageRepository) {
        writers.stderr("Session storage is unavailable.\n");
        return false;
      }
      if (currentSessionId === null) {
        writers.stderr("[export:error] no active session.\n");
        return false;
      }

      const currentSessionTitle =
        sessionRepository
          ?.listSessions()
          .find((session) => session.id === currentSessionId)?.title ??
        `Session ${currentSessionId}`;
      const messages = messageRepository.listMessagesBySession(currentSessionId);
      const payload = buildSessionExportPayload(messages, {
        exportedAt: now().toISOString(),
        sessionId: currentSessionId,
        sessionTitle: currentSessionTitle
      });
      const output = writeSessionExportFile(payload, parsed.options, {
        cwd: process.cwd(),
        now: now()
      });
      if (!output.ok) {
        writers.stderr(`[export:error] ${output.error}\n`);
        return false;
      }
      writers.stdout(
        `[export] wrote ${payload.messageCount} message(s) to ${output.outputPath}\n`
      );
      return false;
    },
    run: async (args) => {
      const raw = args.join(" ").trim();
      if (raw.length === 0) {
        writers.stderr("[run:error] Usage: /run <command>\n");
        return false;
      }
      const risk = classifyRunCommandRisk(raw);
      if (risk.level === "medium" || risk.level === "high") {
        pendingRunConfirmation = {
          command: raw,
          risk,
          requestedAtMs: now().getTime()
        };
        writers.stderr(
          formatRunConfirmationPrompt(
            pendingRunConfirmation,
            runConfirmationTimeoutMs
          )
        );
        return false;
      }

      const { result } = executeAndRenderRunCommand(raw);
      recordRunAudit({
        command: raw,
        riskLevel: risk.level,
        approvalStatus: "not_required",
        executed: result.ok,
        exitCode: result.ok ? result.exitCode : null,
        stdout: result.stdout,
        stderr: result.ok ? result.stderr : result.error
      });
      return false;
    },
    init: async (args) => {
      const target = args[0]?.trim() || process.cwd();
      writers.stdout(
        `[init] bootstrap requested at ${target}. Detailed initialization is a future task.\n`
      );
      return false;
    },
    doctor: async () => {
      writers.stdout("[doctor] diagnostics placeholder: runtime is reachable.\n");
      return false;
    },
    pwd: async () => {
      writers.stdout(`${process.cwd()}\n`);
      return false;
    },
    alias: async (args) => {
      if (args.length < 2) {
        writers.stderr("Usage: /alias <name> <command>\n");
        return false;
      }
      writers.stdout(
        `[alias] placeholder saved: ${args[0]} => ${args.slice(1).join(" ")}\n`
      );
      return false;
    },
    unalias: async (args) => {
      if (args.length !== 1) {
        writers.stderr("Usage: /unalias <name>\n");
        return false;
      }
      writers.stdout(`[alias] placeholder removed: ${args[0]}\n`);
      return false;
    },
    add: async (args) => {
      const raw = args.join(" ").trim();
      if (raw.length === 0) {
        writers.stderr("Usage: /add <path>\n");
        return false;
      }

      const result = addContextFile(contextFiles, raw, process.cwd());
      if (!result.ok) {
        writers.stderr(`[add:error] ${result.error}\n`);
        return false;
      }

      const status = result.added ? "added" : "already added";
      writers.stdout(`[add] ${status}: ${result.normalizedPath}\n`);
      writers.stdout(formatContextFileList(contextFiles, process.cwd()));
      return false;
    },
    drop: async (args) => {
      if (args.length === 0) {
        writers.stderr("Usage: /drop <path|index> [more paths or indexes]\n");
        return false;
      }
      const result = dropContextFiles(contextFiles, args, process.cwd());
      if (!result.ok) {
        writers.stderr(`[drop:error] ${result.error}\n`);
        return false;
      }

      writers.stdout(`[drop] removed ${result.removed.length} file(s).\n`);
      writers.stdout(formatContextFileList(contextFiles, process.cwd()));
      return false;
    },
    files: async (args) => {
      const parsed = parseFilesCommandArgs(args);
      if (!parsed.options) {
        writers.stderr(`${parsed.error}\nUsage: /files [--limit N] [--q keyword]\n`);
        return false;
      }
      writers.stdout(
        formatFilesCommandOutput(contextFiles, parsed.options, process.cwd())
      );
      return false;
    },
    grep: async (args) => {
      const parsed = parseGrepCommandArgs(args);
      if (!parsed.options || !parsed.pattern) {
        writers.stderr(`${parsed.error}\nUsage: /grep <pattern> [--limit N]\n`);
        return false;
      }

      const resolvedPattern = parseGrepPattern(parsed.pattern);
      if (!resolvedPattern.regex) {
        writers.stderr(`[grep:error] ${resolvedPattern.error}\n`);
        return false;
      }

      const result = grepProjectText(resolvedPattern.regex, {
        cwd: process.cwd(),
        limit: parsed.options.limit
      });
      writers.stdout(formatGrepMatches(result.matches, process.cwd()));
      if (result.truncated) {
        writers.stderr(
          `[grep:warn] reached match limit (${parsed.options.limit}); refine pattern or increase --limit.\n`
        );
      }
      return false;
    },
    tree: async (args) => {
      const parsed = parseTreeCommandArgs(args);
      if (!parsed.options) {
        writers.stderr(`${parsed.error}\nUsage: /tree [path] [--depth N]\n`);
        return false;
      }
      const tree = buildProjectTree({
        cwd: process.cwd(),
        rootPath: parsed.options.rootPath,
        depth: parsed.options.depth
      });
      writers.stdout(formatProjectTree(tree));
      return false;
    }
  };

  return {
    onLine: async (line: string) => {
      if (pendingRunConfirmation) {
        const normalizedLine = line.trim().toLowerCase();
        if (normalizedLine === "/approve") {
          return commandHandlers.approve([]);
        }
        const elapsedMs = now().getTime() - pendingRunConfirmation.requestedAtMs;
        if (elapsedMs > runConfirmationTimeoutMs) {
          const timedOut = pendingRunConfirmation;
          pendingRunConfirmation = undefined;
          writers.stderr("[run:confirm] confirmation timed out; command cancelled.\n");
          recordRunAudit({
            command: timedOut.command,
            riskLevel: timedOut.risk.level,
            approvalStatus: "timeout",
            executed: false,
            exitCode: null
          });
          return false;
        }

        const decision = parseRunConfirmationDecision(line);
        if (decision === "invalid") {
          writers.stderr("[run:confirm] invalid input. Reply yes or no.\n");
          return false;
        }

        if (decision === "reject") {
          const rejected = pendingRunConfirmation;
          pendingRunConfirmation = undefined;
          writers.stderr("[run:confirm] command cancelled.\n");
          recordRunAudit({
            command: rejected.command,
            riskLevel: rejected.risk.level,
            approvalStatus: "rejected",
            executed: false,
            exitCode: null
          });
          return false;
        }

        const approved = pendingRunConfirmation;
        pendingRunConfirmation = undefined;
        const { result } = executeAndRenderRunCommand(approved.command);
        recordRunAudit({
          command: approved.command,
          riskLevel: approved.risk.level,
          approvalStatus: "approved",
          executed: result.ok,
          exitCode: result.ok ? result.exitCode : null,
          stdout: result.stdout,
          stderr: result.ok ? result.stderr : result.error
        });
        return false;
      }

      const resolvedLine = resolveInlineTabCompletions(
        line,
        commandRegistry,
        completionUsageFrequency
      );
      const input = classifyReplInput(
        resolvedLine,
        maxInputLength,
        commandRegistry
      );
      if (input.kind === "empty") {
        return false;
      }

      if (input.kind === "message" && input.truncated) {
        writers.stderr(
          `[warn] input exceeded ${maxInputLength} chars; truncated.\n`
        );
      }

      if (input.kind === "command" && input.command.kind === "unknown") {
        const completions = completeReplCommandPrefix(
          input.command.token,
          commandRegistry,
          completionUsageFrequency
        );
        if (completions.length > 0) {
          writers.stdout(formatCompletionCandidates(completions));
          return false;
        }
        writers.stderr(
          `Unknown command: ${input.command.token}. Type /help for commands.\n`
        );
        return false;
      }

      if (input.kind === "command") {
        const normalizedCommandToken = resolvedLine.trim().split(/\s+/, 1)[0];
        const resolvedCommand = commandRegistry.resolve(normalizedCommandToken);
        if (resolvedCommand) {
          incrementCompletionUsageFrequency(
            completionUsageFrequency,
            resolvedCommand.metadata.name
          );
          commandHistoryRepository?.recordCommand({
            command: resolvedCommand.metadata.name,
            cwd: process.cwd()
          });
        }

        const command = input.command;
        const args = "args" in command ? command.args : [];
        switch (command.kind) {
          case "help":
          case "exit":
          case "login":
          case "logout":
          case "model":
          case "config":
          case "clear":
          case "status":
          case "approve":
          case "version":
          case "new":
          case "rename":
          case "sessions":
          case "switch":
          case "history":
          case "export":
          case "run":
          case "init":
          case "doctor":
          case "pwd":
          case "alias":
          case "unalias":
          case "add":
          case "drop":
          case "files":
          case "grep":
          case "tree":
            return commandHandlers[command.kind](args);
          default:
            return false;
        }
      }

      if (input.kind !== "message") {
        return false;
      }

      if (sessionRepository && messageRepository) {
        try {
          if (currentSessionId === null) {
            const next = createAndSwitchSession();
            if (next.id !== undefined) {
              writers.stdout(`Switched to session #${next.id}: ${next.title}\n`);
            }
          }
          if (currentSessionId !== null) {
            messageRepository.createMessage({
              sessionId: currentSessionId,
              role: "user",
              content: input.text
            });
          }
        } catch (error) {
          const e = error as Error;
          writers.stderr(`[session:error] ${e.message}\n`);
          return false;
        }
      }

      try {
        const request = buildChatRequest(conversation, input.text, config);
        const contextMessage = buildContextSystemMessage(
          contextFiles,
          process.cwd()
        );
        if (contextMessage) {
          request.messages = [contextMessage, ...request.messages];
        }
        const trimmed = trimMessagesToTokenBudget(
          request.messages,
          requestTokenBudget
        );
        request.messages = trimmed.messages;
        if (trimmed.truncated) {
          writers.stderr(formatTokenBudgetTrimNotice(trimmed, requestTokenBudget));
        }
        const response = await provider.complete(request);
        const renderedReply = renderAssistantReply(
          response.message.content,
          maxReplyLength
        );
        if (renderedReply.truncated) {
          writers.stderr(
            `[warn] reply exceeded ${maxReplyLength} chars; truncated.\n`
          );
        }
        conversation.push(
          {
            role: "user",
            content: input.text
          },
          {
            role: response.message.role,
            content: renderedReply.text
          }
        );
        if (currentSessionId !== null && messageRepository) {
          messageRepository.createMessage({
            sessionId: currentSessionId,
            role: "assistant",
            content: renderedReply.text
          });
        }
        buffer.append(`${renderedReply.text}\n`);
        buffer.flush();
      } catch (error) {
        const e = error as Error;
        writers.stderr(`[provider:error] ${e.message}\n`);
      }

      return false;
    },
    onClose: () => {
      buffer.flush();
      writers.stdout("EOF received. Bye.\n");
    },
    onSigint: () => {
      buffer.flush();
      writers.stdout("^C\nInterrupted. Bye.\n");
    }
  };
}

export function startRepl(
  runtime: ReplRuntime,
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH,
  options?: ReplSessionOptions
): readline.Interface {
  const sharedCompletionUsageFrequency =
    options?.completionUsageFrequency ??
    options?.commandHistoryRepository?.listUsageFrequency() ??
    {};
  const session = createReplSession(runtime, maxInputLength, {
    ...(options ?? {}),
    completionUsageFrequency: sharedCompletionUsageFrequency
  });
  let closedByExit = false;
  let closedBySigint = false;
  const rl = readline.createInterface({
    input: runtime.input,
    output: runtime.output,
    terminal: shouldUseTerminalMode(runtime),
    completer: createReplReadlineCompleter(
      options?.commandRegistry,
      sharedCompletionUsageFrequency
    )
  });

  runtime.stdout("MiniCLI REPL ready. Type your message.\n");
  rl.setPrompt("> ");
  rl.prompt();

  rl.on("line", async (line: string) => {
    const shouldExit = await session.onLine(line);
    if (shouldExit) {
      closedByExit = true;
      rl.close();
      return;
    }
    rl.prompt();
  });

  rl.on("close", () => {
    if (closedByExit || closedBySigint) {
      return;
    }
    session.onClose();
  });

  rl.on("SIGINT", () => {
    closedBySigint = true;
    session.onSigint();
    rl.close();
  });

  return rl;
}
