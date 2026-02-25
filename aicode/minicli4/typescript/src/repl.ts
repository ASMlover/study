import readline from "node:readline";
import { loadConfig, RuntimeConfig } from "./config";
import { runMultiAgentRound } from "./agent";
import { complete } from "./completion";
import { runSlash, COMMANDS } from "./commands";
import { SessionRecord, SessionStore } from "./session";
import { GLMProvider } from "./provider";
import { ToolRegistry } from "./tools";
import { PaneState, TwoPaneTui } from "./tui";

export interface ReplRuntime {
  input: NodeJS.ReadableStream;
  output: NodeJS.WritableStream;
  stdout: (text: string) => void;
  stderr: (text: string) => void;
}

export function startRepl(runtime: ReplRuntime, projectRoot = process.cwd()): readline.Interface {
  const loaded = loadConfig(projectRoot);
  let config: RuntimeConfig = loaded.config;
  const sessions = new SessionStore(loaded.stateDir);
  let session: SessionRecord = sessions.current();
  let pendingApproval = "";

  const provider = new GLMProvider(config);
  let tools = new ToolRegistry(projectRoot, config);
  const tui = new TwoPaneTui(runtime.output);

  const pane: PaneState = {
    sessionId: session.session_id,
    model: config.model,
    stage: "idle",
    tools: tools.listToolNames().join(","),
    mode: config.safe_mode,
    pendingApproval: "none"
  };

  const display: string[] = [];
  const pushLine = (line: string): void => {
    display.push(line);
    runtime.stdout(`${line}\n`);
    tui.render(display, pane);
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
  rl.setPrompt("> ");
  rl.prompt();

  const inputStream = runtime.input as NodeJS.ReadStream;
  readline.emitKeypressEvents(inputStream, rl);
  const onKeypress = (_str: string, key: readline.Key): void => {
    if (key.ctrl && key.name === "l") {
      tui.render(display, pane);
      rl.prompt();
      return;
    }
    if (key.name === "f1") {
      runSlash(
        {
          projectRoot,
          config,
          session,
          sessions,
          tools,
          setConfig: () => {},
          setSession: () => {},
          out: pushLine,
          setPendingApproval: () => {},
          clearPendingApproval: () => {},
          pendingApproval: () => pendingApproval
        },
        "/help"
      );
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
    tools = new ToolRegistry(projectRoot, config);
    pane.model = config.model;
    pane.mode = config.safe_mode;
    pane.tools = tools.listToolNames().join(",");
    tui.applyConfig(next);
  };

  const setSession = (next: SessionRecord): void => {
    session = next;
    pane.sessionId = next.session_id;
  };

  rl.on("line", async (line: string) => {
    const input = line.trim();
    if (!input) {
      rl.prompt();
      return;
    }

    try {
      if (input.startsWith("/")) {
        const keep = runSlash(
          {
            projectRoot,
            config,
            session,
            sessions,
            tools,
            setConfig,
            setSession,
            out: pushLine,
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

        if (!keep) {
          rl.close();
          return;
        }
        rl.prompt();
        return;
      }

      session.messages.push({ role: "user", content: input, created_at: new Date().toISOString() });
      sessions.save(session);
      pushLine(`user: ${input}`);
      pane.stage = "planner";

      const result = await runMultiAgentRound(input, session.messages, provider, tools, config, {
        onStage: (stage, detail) => {
          pane.stage = stage;
          pushLine(`[stage:${stage}] ${detail}`);
        },
        onTool: (name, output) => {
          pushLine(`[tool:${name}] ${output.slice(0, 160)}`);
        },
        onDelta: (chunk) => {
          if (display.length === 0 || !display[display.length - 1].startsWith("assistant: ")) {
            display.push("assistant: ");
          }
          display[display.length - 1] += chunk;
          tui.render(display, pane);
        }
      });

      session.messages.push({ role: "assistant", content: result.final, created_at: new Date().toISOString() });
      sessions.save(session);
      pane.stage = "idle";
    } catch (error) {
      const e = error as Error;
      runtime.stderr(`[error] ${e.message}\n`);
      pane.stage = "error";
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
