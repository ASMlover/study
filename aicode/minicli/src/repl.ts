import fs from "node:fs";
import path from "node:path";
import readline from "node:readline";
import {
  CommandRegistration,
  CommandRegistry
} from "./command-registry";
import {
  isSupportedModelName,
  maskSecret,
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

export const DEFAULT_MAX_INPUT_LENGTH = 1024;
export const DEFAULT_OUTPUT_BUFFER_LIMIT = 4096;
export const DEFAULT_MAX_REPLY_LENGTH = 2048;
export const DEFAULT_MAX_HISTORY_PREVIEW_LENGTH = 120;
export const EMPTY_REPLY_PLACEHOLDER = "[empty reply]";
export const MAX_COMPLETION_USAGE_FREQUENCY = 2_147_483_647;
export const DEFAULT_RUN_CONFIRMATION_TIMEOUT_MS = 15_000;

export type RunConfirmationDecision = "confirm" | "reject" | "invalid";

export type KnownReplCommandKind =
  | "help"
  | "exit"
  | "login"
  | "model"
  | "new"
  | "sessions"
  | "switch"
  | "history"
  | "run"
  | "add";

interface ReplCommandRegistration
  extends CommandRegistration<KnownReplCommandKind> {
  readonly acceptsArgs: boolean;
}

function createReplCommand(
  kind: KnownReplCommandKind,
  name: string,
  usage: string,
  description: string,
  acceptsArgs: boolean,
  aliases: readonly string[] = []
): ReplCommandRegistration {
  return {
    kind,
    metadata: {
      name,
      usage,
      description
    },
    acceptsArgs,
    aliases
  };
}

export function createDefaultReplCommandRegistry(): CommandRegistry<KnownReplCommandKind> {
  const registry = new CommandRegistry<KnownReplCommandKind>();
  registry.registerMany([
    createReplCommand("help", "/help", "/help", "Show this help message", false),
    createReplCommand(
      "exit",
      "/exit",
      "/exit",
      "Exit MiniCLI",
      false,
      ["/quit"]
    ),
    createReplCommand(
      "login",
      "/login",
      "/login <apiKey>",
      "Save API key to global config",
      true
    ),
    createReplCommand(
      "model",
      "/model",
      "/model [name]",
      "Show or set current model",
      true
    ),
    createReplCommand(
      "new",
      "/new",
      "/new [title]",
      "Create and switch to a new session",
      true
    ),
    createReplCommand(
      "sessions",
      "/sessions",
      "/sessions [--limit N] [--offset N] [--q keyword]",
      "List sessions",
      true
    ),
    createReplCommand(
      "switch",
      "/switch",
      "/switch <#id|index>",
      "Switch to a specific session",
      true
    ),
    createReplCommand(
      "history",
      "/history",
      "/history [--limit N] [--offset N] [--audit] [--status <not_required|approved|rejected|timeout>]",
      "Show messages in current session",
      true
    ),
    createReplCommand(
      "run",
      "/run",
      "/run <command>",
      "Execute a read-only shell command",
      true
    ),
    createReplCommand(
      "add",
      "/add",
      "/add <path>",
      "Add a text file into context collection",
      true
    )
  ]);
  return registry;
}

function formatHelpText(
  registry: CommandRegistry<KnownReplCommandKind>
): string {
  const lines = registry.list().map((command) => {
    return `${command.metadata.usage} ${command.metadata.description}`;
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
  prefix: string;
  candidates: string[];
  selectedIndex: number;
  navigated: boolean;
}

interface ReplCompletionContext {
  prefix: string;
  firstTokenStart: number;
  firstTokenEnd: number;
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
  const firstTokenStart = leadingWhitespaceLength;

  if (firstTokenStart >= line.length || line[firstTokenStart] !== "/") {
    return undefined;
  }

  let firstTokenEnd = line.length;
  for (let index = firstTokenStart; index < line.length; index += 1) {
    if (/\s/.test(line[index])) {
      firstTokenEnd = index;
      break;
    }
  }

  if (boundedCursor < firstTokenStart || boundedCursor > firstTokenEnd) {
    return undefined;
  }

  return {
    prefix: line.slice(firstTokenStart, boundedCursor),
    firstTokenStart,
    firstTokenEnd
  };
}

function applyResolvedCompletionCandidate(
  line: string,
  context: ReplCompletionContext,
  candidate: string
): { line: string; cursor: number } {
  const beforeToken = line.slice(0, context.firstTokenStart);
  const afterToken = line.slice(context.firstTokenEnd);
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
  const context = resolveReplCompletionContext(line, boundedCursor);
  if (!context) {
    return {
      line,
      cursor: boundedCursor,
      accepted: false,
      candidates: []
    };
  }

  const candidates = completeReplCommandPrefix(
    context.prefix,
    registry,
    usageFrequency
  );
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
      const context = resolveReplCompletionContext(resolved, resolved.length);
      if (!context) {
        completionState = undefined;
        index += 1;
        continue;
      }

      const candidates = completeReplCommandPrefix(
        context.prefix,
        registry,
        usageFrequency
      );
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

      const canAcceptSelection =
        completionState?.prefix === context.prefix &&
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
          prefix: context.prefix,
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
    const context = resolveReplCompletionContext(line, line.length);
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
      const completedToken = accepted.line
        .slice(context.firstTokenStart)
        .split(/\s/, 1)[0];
      return [[completedToken], context.prefix];
    }

    return [accepted.candidates, context.prefix];
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
  maxReplyLength?: number;
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
  | { kind: "model"; args: string[] }
  | { kind: "new"; args: string[] }
  | { kind: "sessions"; args: string[] }
  | { kind: "switch"; args: string[] }
  | { kind: "history"; args: string[] }
  | { kind: "run"; args: string[] }
  | { kind: "add"; args: string[] }
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
  if (kind === "model") {
    return { kind: "model", args };
  }
  if (kind === "new") {
    return { kind: "new", args };
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
  if (kind === "add") {
    return { kind: "add", args };
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
  const runConfirmationTimeoutMs =
    options?.runConfirmationTimeoutMs ?? DEFAULT_RUN_CONFIRMATION_TIMEOUT_MS;
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
    }
  };

  return {
    onLine: async (line: string) => {
      if (pendingRunConfirmation) {
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
          case "model":
          case "new":
          case "sessions":
          case "switch":
          case "history":
          case "run":
          case "add":
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
