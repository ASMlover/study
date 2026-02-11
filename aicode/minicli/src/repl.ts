import readline from "node:readline";

export const DEFAULT_MAX_INPUT_LENGTH = 1024;
export const DEFAULT_OUTPUT_BUFFER_LIMIT = 4096;

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
  onLine: (line: string) => void;
  onClose: () => void;
  onSigint: () => void;
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

export function createReplSession(
  writers: ReplWriters,
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH
): ReplSession {
  const buffer = new OutputBuffer(writers.stdout);

  return {
    onLine: (line: string) => {
      const parsed = parseReplLine(line, maxInputLength);
      if (parsed.kind === "empty") {
        return;
      }

      if (parsed.truncated) {
        writers.stderr(
          `[warn] input exceeded ${maxInputLength} chars; truncated.\n`
        );
      }

      buffer.append(`echo: ${parsed.text}\n`);
      buffer.flush();
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
  maxInputLength: number = DEFAULT_MAX_INPUT_LENGTH
): readline.Interface {
  const session = createReplSession(runtime, maxInputLength);
  const rl = readline.createInterface({
    input: runtime.input,
    output: runtime.output,
    terminal: (runtime.output as Partial<NodeJS.WriteStream>).isTTY === true
  });

  runtime.stdout("MiniCLI REPL ready. Type your message.\n");
  rl.setPrompt("> ");
  rl.prompt();

  rl.on("line", (line: string) => {
    session.onLine(line);
    rl.prompt();
  });

  rl.on("close", () => {
    session.onClose();
  });

  rl.on("SIGINT", () => {
    session.onSigint();
    rl.close();
  });

  return rl;
}
