import readline from "node:readline";
import { loadConfig, RuntimeConfig } from "./config";
import { runMultiAgentRound } from "./agent";
import { complete } from "./completion";
import { runSlash, COMMANDS } from "./commands";
import { SessionRecord, SessionStore } from "./session";
import { GLMProvider } from "./provider";
import { ToolRegistry } from "./tools";
import { PaneState, TwoPaneTui } from "./tui";
import { MarkdownAnsiStreamRenderer } from "./markdown";
import { setAnsiTheme } from "./ansi";

export interface ReplRuntime {
  input: NodeJS.ReadableStream;
  output: NodeJS.WritableStream;
  stdout: (text: string) => void;
  stderr: (text: string) => void;
}

export function startRepl(runtime: ReplRuntime, projectRoot = process.cwd()): readline.Interface {
  const loaded = loadConfig(projectRoot);
  let config: RuntimeConfig = loaded.config;
  setAnsiTheme(config.theme);
  const sessions = new SessionStore(loaded.stateDir);
  let session: SessionRecord = sessions.current();
  let pendingApproval = "";

  const provider = new GLMProvider(config);
  let tools = new ToolRegistry(projectRoot, config);
  const tui = new TwoPaneTui(runtime.output as NodeJS.WritableStream & { isTTY?: boolean; columns?: number });
  const isTTY = Boolean((runtime.output as NodeJS.WritableStream & { isTTY?: boolean }).isTTY);

  const pane: PaneState = {
    sessionId: session.session_id,
    model: config.model,
    stage: "idle",
    tools: tools.listToolNames().join(","),
    mode: config.safe_mode,
    pendingApproval: "none"
  };

  const pushLine = (line: string): void => {
    if (isTTY) {
      tui.printEvent(line);
      return;
    }
    runtime.stdout(`${line}\n`);
  };

  const pushLines = (lines: string[]): void => {
    if (lines.length === 0) {
      return;
    }
    if (isTTY) {
      for (const line of lines) {
        tui.printEvent(line);
      }
      return;
    }
    runtime.stdout(`${lines.join("\n")}\n`);
  };

  const rl = readline.createInterface({
    input: runtime.input,
    output: runtime.output,
    terminal: true,
    completer: (line: string): [string[], string] => {
      const items = complete(line, {
        commandNames: COMMANDS,
        sessionIds: sessions.list().map((s) => s.session_id),
        configKeys: Object.keys(config),
        toolNames: tools.listToolNames(),
        projectRoot
      });
      return [items, line];
    }
  });

  runtime.stdout("MiniCLI4 TypeScript REPL\n");
  runtime.stdout("Type /help for commands.\n");
  if (isTTY) {
    tui.applyConfig(config);
    tui.start(pane);
  }
  rl.setPrompt("> ");
  rl.prompt();

  const inputStream = runtime.input as NodeJS.ReadStream;
  readline.emitKeypressEvents(inputStream, rl);
  const onKeypress = (_str: string, key: readline.Key): void => {
    if (key.ctrl && key.name === "l") {
      if (isTTY) {
        tui.printEvent("[ui] append-only mode; showing latest status");
        tui.printStatus(pane);
      }
      rl.prompt();
      return;
    }
    if (key.name === "f1") {
      const lines: string[] = [];
      runSlash(
        {
          projectRoot,
          config,
          session,
          sessions,
          tools,
          setConfig: () => {},
          setSession: () => {},
          out: (line) => {
            lines.push(line);
          },
          setPendingApproval: () => {},
          clearPendingApproval: () => {},
          pendingApproval: () => pendingApproval
        },
        "/help"
      );
      if (isTTY) {
        tui.announceInput("/help");
      }
      pushLines(lines);
      rl.prompt();
      return;
    }
    if (key.name === "escape") {
      pushLine("[completion] dismissed");
      rl.prompt();
    }
  };
  inputStream.on("keypress", onKeypress);

  const setConfig = (next: RuntimeConfig): void => {
    config = next;
    setAnsiTheme(next.theme);
    tools = new ToolRegistry(projectRoot, config);
    pane.model = config.model;
    pane.mode = config.safe_mode;
    pane.tools = tools.listToolNames().join(",");
    tui.applyConfig(next);
    tui.updateStatus(pane);
  };

  const setSession = (next: SessionRecord): void => {
    session = next;
    pane.sessionId = next.session_id;
    tui.updateStatus(pane);
  };

  rl.on("line", async (line: string) => {
    const input = line.trim();
    if (!input) {
      rl.prompt();
      return;
    }
    if (isTTY) {
      tui.announceInput(input);
    }

    try {
      if (input.startsWith("/")) {
        const commandLines: string[] = [];
        const keep = runSlash(
          {
            projectRoot,
            config,
            session,
            sessions,
            tools,
            setConfig,
            setSession,
            out: (line) => {
              commandLines.push(line);
            },
            setPendingApproval: (value) => {
              pendingApproval = value;
              pane.pendingApproval = value;
            },
            clearPendingApproval: () => {
              pendingApproval = "";
              pane.pendingApproval = "none";
            },
            pendingApproval: () => pendingApproval
          },
          input
        );

        pushLines(commandLines);
        tui.updateStatus(pane);

        if (!keep) {
          rl.close();
          return;
        }
        rl.prompt();
        return;
      }

      session.messages.push({ role: "user", content: input, created_at: new Date().toISOString() });
      sessions.save(session);

      pane.stage = "planner";
      tui.updateStatus(pane);
      if (isTTY) {
        tui.startThinking();
      }

      let streamStarted = false;
      const markdownRenderer = new MarkdownAnsiStreamRenderer({ colorize: isTTY });
      const result = await runMultiAgentRound(input, session.messages, provider, tools, config, {
        onStage: (stage, _detail) => {
          pane.stage = stage;
          tui.updateStatus(pane);
        },
        onTool: (_name, _output) => {},
        onDelta: (chunk) => {
          const rendered = markdownRenderer.write(chunk);
          if (!rendered) {
            return;
          }
          if (!streamStarted) {
            streamStarted = true;
            if (isTTY) {
              tui.stopThinking();
              tui.startAssistantStream();
            }
          }
          if (isTTY) {
            tui.appendAssistantChunk(rendered);
          } else {
            runtime.stdout(rendered);
          }
        }
      });

      const tail = markdownRenderer.flush();
      if (tail.length > 0) {
        if (!streamStarted) {
          streamStarted = true;
          if (isTTY) {
            tui.stopThinking();
            tui.startAssistantStream();
          }
        }
        if (isTTY) {
          tui.appendAssistantChunk(tail);
        } else {
          runtime.stdout(tail);
        }
      }

      if (streamStarted) {
        if (isTTY) {
          tui.stopThinking();
          tui.endAssistantStream();
        } else {
          runtime.stdout("\n");
        }
      } else {
        const nonStreamRenderer = new MarkdownAnsiStreamRenderer({ colorize: isTTY });
        const fallbackRendered = `${nonStreamRenderer.write(`${result.final}\n`)}${nonStreamRenderer.flush()}`.replace(/\n$/, "");
        if (isTTY) {
          tui.stopThinking();
          tui.startAssistantStream();
          tui.appendAssistantChunk(fallbackRendered);
          tui.endAssistantStream();
        } else {
          runtime.stdout(`${fallbackRendered}\n`);
        }
      }

      session.messages.push({ role: "assistant", content: result.final, created_at: new Date().toISOString() });
      sessions.save(session);
      pane.stage = "idle";
      tui.updateStatus(pane);
    } catch (error) {
      const e = error as Error;
      if (isTTY) {
        tui.stopThinking();
      }
      runtime.stderr(`[error] ${e.message}\n`);
      pane.stage = "error";
      tui.updateStatus(pane);
    }

    rl.prompt();
  });

  rl.on("SIGINT", () => {
    runtime.stdout("\nInterrupted. Type /exit to quit.\n");
    rl.prompt();
  });

  rl.on("close", () => {
    inputStream.off("keypress", onKeypress);
    runtime.stdout("Bye.\n");
  });

  return rl;
}
