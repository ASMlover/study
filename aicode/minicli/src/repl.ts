import readline from "node:readline";
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
import { MessageRepository, SessionRepository } from "./repository";

export const DEFAULT_MAX_INPUT_LENGTH = 1024;
export const DEFAULT_OUTPUT_BUFFER_LIMIT = 4096;
export const DEFAULT_MAX_REPLY_LENGTH = 2048;
export const EMPTY_REPLY_PLACEHOLDER = "[empty reply]";
export const HELP_TEXT =
  "Available commands:\n/help Show this help message\n/exit Exit MiniCLI\n/login <apiKey> Save API key to global config\n/model [name] Show or set current model\n/new [title] Create and switch to a new session\n";

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
  messageRepository?: Pick<MessageRepository, "createMessage">;
  now?: () => Date;
}

export type ReplCommandMatch =
  | { kind: "help" }
  | { kind: "exit" }
  | { kind: "login"; args: string[] }
  | { kind: "model"; args: string[] }
  | { kind: "new"; args: string[] }
  | { kind: "unknown"; token: string }
  | { kind: "none" };

export type ReplInputClassification =
  | { kind: "empty" }
  | { kind: "command"; command: Exclude<ReplCommandMatch, { kind: "none" }> }
  | { kind: "message"; text: string; truncated: boolean };

export interface RenderedAssistantReply {
  text: string;
  truncated: boolean;
  usedFallback: boolean;
}

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

export function matchReplCommand(text: string): ReplCommandMatch {
  if (!text.startsWith("/")) {
    return { kind: "none" };
  }

  const [token, ...args] = text.split(/\s+/);

  if (token === "/help") {
    if (args.length !== 0) {
      return { kind: "unknown", token };
    }
    return { kind: "help" };
  }

  if (token === "/exit") {
    if (args.length !== 0) {
      return { kind: "unknown", token };
    }
    return { kind: "exit" };
  }

  if (token === "/login") {
    return { kind: "login", args };
  }

  if (token === "/model") {
    return { kind: "model", args };
  }

  if (token === "/new") {
    return { kind: "new", args };
  }

  return { kind: "unknown", token };
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

export function classifyReplInput(
  line: string,
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH
): ReplInputClassification {
  const parsed = parseReplLine(line, maxInputLength);
  if (parsed.kind === "empty") {
    return { kind: "empty" };
  }

  const command = matchReplCommand(parsed.text);
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
  const now = options?.now ?? (() => new Date());
  const conversation: ChatMessage[] = [];
  let currentSessionId: number | null = null;

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

  return {
    onLine: async (line: string) => {
      const input = classifyReplInput(line, maxInputLength);
      if (input.kind === "empty") {
        return false;
      }

      if (input.kind === "message" && input.truncated) {
        writers.stderr(
          `[warn] input exceeded ${maxInputLength} chars; truncated.\n`
        );
      }

      if (input.kind === "command" && input.command.kind === "help") {
        writers.stdout(HELP_TEXT);
        return false;
      }

      if (input.kind === "command" && input.command.kind === "exit") {
        writers.stdout("Bye.\n");
        return true;
      }

      if (input.kind === "command" && input.command.kind === "unknown") {
        writers.stderr(
          `Unknown command: ${input.command.token}. Type /help for commands.\n`
        );
        return false;
      }

      if (input.kind === "command" && input.command.kind === "login") {
        if (input.command.args.length !== 1) {
          writers.stderr("Usage: /login <apiKey>\n");
          return false;
        }

        const apiKey = input.command.args[0].trim();
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
      }

      if (input.kind === "command" && input.command.kind === "model") {
        if (input.command.args.length === 0) {
          writers.stdout(`Current model: ${config.model}\n`);
          return false;
        }

        if (input.command.args.length > 1) {
          writers.stderr("Usage: /model [name]\n");
          return false;
        }

        const nextModel = input.command.args[0].trim();
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
      }

      if (input.kind === "command" && input.command.kind === "new") {
        const requestedTitle = parseNewSessionTitle(input.command.args);
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
  const session = createReplSession(runtime, maxInputLength, options);
  let closedByExit = false;
  let closedBySigint = false;
  const rl = readline.createInterface({
    input: runtime.input,
    output: runtime.output,
    terminal: (runtime.output as Partial<NodeJS.WriteStream>).isTTY === true
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
