import { resolve } from "node:path";
import { loadConfig, ensureConfigDir } from "./config.js";
import { GlmProvider } from "./provider/glm.js";
import { toolRegistry } from "./tools/registry.js";
import { runAgentLoop } from "./agent/loop.js";
import { spawnSubAgent } from "./agent/sub-agent.js";
import { prompt } from "./tui/prompt.js";
import { spinner } from "./tui/spinner.js";
import { getTheme, colorize, hrule, BOX } from "./tui/theme.js";
import { confirmPermission, clearAlwaysAllowed } from "./tui/confirm.js";
import { initLogger, log } from "./utils/logger.js";
import { initSkills, getSkill, skillNames, listSkills } from "./skills/registry.js";
import { createSession, saveSession, loadSession, listSessions, deleteSession, addTurn } from "./context/session.js";
import { shouldCompact, compactContext } from "./context/compact.js";
import { loadMemory } from "./context/memory.js";
import { createTask, getTask, updateTask, listTasks, loadTasks, exportTasks } from "./tasks/manager.js";
import { formatTaskList } from "./tasks/scheduler.js";
import { estimateMessagesTokens } from "./utils/tokens.js";
import type { Message, AgentTurn, AppConfig, Session, SubAgentType } from "./types.js";

// ─── CLI Args ────────────────────────────────────────────────────────────────

function parseArgs(): { projectRoot: string; sessionId?: string; model?: string } {
  const args = process.argv.slice(2);
  let projectRoot = process.cwd();
  let sessionId: string | undefined;
  let model: string | undefined;

  for (let i = 0; i < args.length; i++) {
    if (args[i] === "--config" && args[i + 1]) {
      projectRoot = resolve(args[++i]);
    } else if (args[i] === "--session" && args[i + 1]) {
      sessionId = args[++i];
    } else if (args[i] === "--model" && args[i + 1]) {
      model = args[++i];
    }
  }

  return { projectRoot, sessionId, model };
}

// ─── Welcome Banner ──────────────────────────────────────────────────────────

function showBanner(config: AppConfig): void {
  const t = getTheme();
  const w = Math.min(process.stderr.columns || 60, 60);

  // ASCII art logo — geometric, compact
  const logo = [
    `${t.primary}    ╔╦╗╦╔╗╔╦  ╔═╗╦  ╦${t.reset}`,
    `${t.primary}    ║║║║║║║║  ║  ║  ║${t.reset}`,
    `${t.accent}    ╩ ╩╩╝╚╝╩  ╚═╝╩═╝╩${t.reset}  ${t.dim}v0.1.0${t.reset}`,
  ];

  process.stderr.write("\n");
  for (const line of logo) {
    process.stderr.write(line + "\n");
  }

  process.stderr.write(`\n  ${hrule(w - 4)}\n`);
  process.stderr.write(`  ${t.muted}Model${t.reset}  ${t.dim}${config.model}${t.reset}\n`);
  process.stderr.write(`  ${t.muted}API${t.reset}    ${t.dim}${config.provider}${t.reset}\n`);
  process.stderr.write(`  ${hrule(w - 4)}\n`);
  process.stderr.write(`  ${t.dim}/help commands ${BOX.v} Ctrl+D exit${t.reset}\n\n`);
}

// ─── Slash Commands ──────────────────────────────────────────────────────────

function showHelp(): void {
  const t = getTheme();
  const w = Math.min(process.stderr.columns || 60, 60);

  const cmds = [
    ["/help",    "Show this help"],
    ["/clear",   "Clear conversation"],
    ["/status",  "Session status"],
    ["/tasks",   "Task list"],
    ["/compact", "Compact context"],
    ["/session", "Session management"],
    ["/skills",  "Available skills"],
  ];

  const skills = listSkills();

  const lines: string[] = [];

  // Commands section
  lines.push(`\n  ${t.heading}Commands${t.reset}`);
  lines.push(`  ${hrule(w - 4)}`);
  for (const [cmd, desc] of cmds) {
    lines.push(`  ${t.primary}${cmd.padEnd(12)}${t.reset}${t.dim}${desc}${t.reset}`);
  }

  // Skills section
  if (skills.length > 0) {
    lines.push("");
    lines.push(`  ${t.heading}Skills${t.reset}`);
    lines.push(`  ${hrule(w - 4)}`);
    for (const s of skills) {
      lines.push(`  ${t.accent}/${s.name.padEnd(11)}${t.reset}${t.dim}${s.description}${t.reset}`);
    }
  }

  // Keys section
  lines.push("");
  lines.push(`  ${t.heading}Keys${t.reset}`);
  lines.push(`  ${hrule(w - 4)}`);
  lines.push(`  ${t.muted}Ctrl+C${t.reset}      ${t.dim}Abort generation${t.reset}`);
  lines.push(`  ${t.muted}Ctrl+D${t.reset}      ${t.dim}Exit${t.reset}`);

  process.stderr.write(lines.join("\n") + "\n\n");
}

// ─── Approval Gate ───────────────────────────────────────────────────────────

async function approveAction(toolName: string, detail: string, context?: string): Promise<boolean> {
  return confirmPermission(toolName, detail, context);
}

// ─── Main REPL ───────────────────────────────────────────────────────────────

async function main(): Promise<void> {
  const { projectRoot, sessionId, model: cliModel } = parseArgs();

  // Init
  ensureConfigDir(projectRoot);
  initLogger(projectRoot);
  initSkills();

  const config = loadConfig(projectRoot);
  if (cliModel) config.model = cliModel;

  if (!config.api_key) {
    const t = getTheme();
    process.stderr.write(`${t.error}Error: No API key configured.${t.reset}\n`);
    process.stderr.write(`Edit ${projectRoot}/.minicli5/config.json and set "api_key".\n`);
    process.exit(1);
  }

  const provider = new GlmProvider(config);
  showBanner(config);

  // Session
  let session: Session;
  if (sessionId) {
    const loaded = loadSession(projectRoot, sessionId);
    if (loaded) {
      session = loaded;
      if (loaded.tasks) loadTasks(loaded.tasks);
      process.stderr.write(`  ${colorize("success", "✓")} ${colorize("dim", `Resumed session: ${session.name}`)}\n\n`);
    } else {
      session = createSession(projectRoot);
    }
  } else {
    session = createSession(projectRoot);
  }

  // Load memory
  const memory = loadMemory(projectRoot);

  // Conversation messages
  let messages: Message[] = [];
  if (memory) {
    messages.push({ role: "system", content: `[Cross-session memory]\n${memory}` });
  }

  // Reconstruct from session turns
  for (const turn of session.turns) {
    messages.push({ role: turn.role, content: turn.content });
  }

  // Abort controller
  let abortController = new AbortController();

  // Prompt setup
  prompt.init({
    skills: [...skillNames(), "help", "clear", "status", "tasks", "compact", "session", "skills"],
    onAbort: () => {
      abortController.abort();
      spinner.stop();
      process.stderr.write(`\n  ${colorize("warning", "◆ Aborted.")}\n`);
    },
  });

  // ── REPL Loop ──
  while (true) {
    const input = await prompt.readInput();

    if (input === null) {
      // Ctrl+D or close
      process.stderr.write(`\n  ${colorize("dim", "Goodbye.")} ${colorize("muted", "◆")}\n`);
      break;
    }

    if (!input) continue;

    // ── Slash Commands ──
    if (input.startsWith("/")) {
      const [cmd, ...rest] = input.slice(1).split(/\s+/);
      const arg = rest.join(" ");

      switch (cmd) {
        case "help":
          showHelp();
          continue;

        case "clear":
          messages = [];
          if (memory) messages.push({ role: "system", content: `[Cross-session memory]\n${memory}` });
          clearAlwaysAllowed();
          process.stderr.write(`  ${colorize("success", "✓")} ${colorize("dim", "Conversation cleared.")}\n\n`);
          continue;

        case "status": {
          const t = getTheme();
          const tokenCount = estimateMessagesTokens(messages);
          const pct = ((tokenCount / config.context_max_tokens) * 100).toFixed(1);
          const pctNum = parseFloat(pct);

          // Token usage bar
          const barWidth = 20;
          const filled = Math.round(barWidth * pctNum / 100);
          const barColor = pctNum > 80 ? t.error : pctNum > 60 ? t.warning : t.success;
          const bar = `${barColor}${"█".repeat(filled)}${t.dim}${"░".repeat(barWidth - filled)}${t.reset}`;

          const w = Math.min(process.stderr.columns || 60, 60);
          process.stderr.write(`\n  ${t.heading}Status${t.reset}\n`);
          process.stderr.write(`  ${hrule(w - 4)}\n`);
          process.stderr.write(`  ${t.muted}Session${t.reset}   ${session.name} ${t.dim}(${session.id.slice(0, 8)})${t.reset}\n`);
          process.stderr.write(`  ${t.muted}Model${t.reset}     ${config.model}\n`);
          process.stderr.write(`  ${t.muted}Messages${t.reset}  ${messages.length}  ${t.dim}${BOX.v}${t.reset}  ${t.muted}Turns${t.reset}  ${session.turns.length}  ${t.dim}${BOX.v}${t.reset}  ${t.muted}Tasks${t.reset}  ${listTasks().length}\n`);
          process.stderr.write(`  ${t.muted}Tokens${t.reset}    ${bar} ${t.dim}~${tokenCount}/${config.context_max_tokens} (${pct}%)${t.reset}\n\n`);
          continue;
        }

        case "tasks":
          process.stderr.write(formatTaskList() + "\n\n");
          continue;

        case "compact": {
          process.stderr.write(`  ${colorize("dim", "◇ Compacting context...")}\n`);
          const result = await compactContext(provider, messages, config);
          messages = result.messages;
          session.compactedSummary = result.summary;
          saveSession(projectRoot, session);
          process.stderr.write(`  ${colorize("success", "✓")} ${colorize("dim", "Context compacted.")}\n\n`);
          continue;
        }

        case "session": {
          if (arg === "list") {
            const sessions = listSessions(projectRoot);
            if (sessions.length === 0) {
              process.stderr.write("No saved sessions.\n\n");
            } else {
              for (const s of sessions) {
                const date = new Date(s.updatedAt).toLocaleString();
                const current = s.id === session.id ? " ← current" : "";
                process.stderr.write(`  ${s.id} ${s.name} (${date})${current}\n`);
              }
              process.stderr.write("\n");
            }
          } else if (arg.startsWith("switch ")) {
            const sid = arg.slice(7).trim();
            const loaded = loadSession(projectRoot, sid);
            if (loaded) {
              session = loaded;
              messages = [];
              if (memory) messages.push({ role: "system", content: `[Cross-session memory]\n${memory}` });
              for (const turn of session.turns) {
                messages.push({ role: turn.role, content: turn.content });
              }
              if (loaded.tasks) loadTasks(loaded.tasks);
              process.stderr.write(`  ${colorize("success", "✓")} ${colorize("dim", `Switched to: ${session.name}`)}\n\n`);
            } else {
              process.stderr.write(colorize("error", `Session not found: ${sid}`) + "\n\n");
            }
          } else if (arg.startsWith("delete ")) {
            const sid = arg.slice(7).trim();
            if (deleteSession(projectRoot, sid)) {
              process.stderr.write(`  ${colorize("success", "✓")} ${colorize("dim", `Deleted: ${sid}`)}\n\n`);
            } else {
              process.stderr.write(colorize("error", `Session not found: ${sid}`) + "\n\n");
            }
          } else {
            process.stderr.write("Usage: /session list | /session switch <id> | /session delete <id>\n\n");
          }
          continue;
        }

        case "skills":
          for (const s of listSkills()) {
            process.stderr.write(`  ${colorize("primary", `/${s.name}`)}  ${colorize("dim", s.description)}\n`);
          }
          process.stderr.write("\n");
          continue;

        default: {
          // Check if it's a skill
          const skill = getSkill(cmd);
          if (skill) {
            const skillPrompt = arg ? `${skill.promptTemplate}\n\nUser context: ${arg}` : skill.promptTemplate;
            messages.push({ role: "user", content: skillPrompt });

            const userTurn: AgentTurn = { role: "user", content: skillPrompt, timestamp: Date.now() };
            addTurn(session, userTurn);
          } else {
            process.stderr.write(colorize("error", `Unknown command: /${cmd}`) + "\n");
            process.stderr.write(colorize("dim", "Type /help for available commands.") + "\n\n");
            continue;
          }
        }
      }

      // If we fell through from a skill invocation, continue to agent loop below
      if (!getSkill(cmd)) continue;
    } else {
      // Regular user message
      messages.push({ role: "user", content: input });

      const userTurn: AgentTurn = { role: "user", content: input, timestamp: Date.now() };
      addTurn(session, userTurn);
    }

    // ── Auto-compact check ──
    if (shouldCompact(messages, config)) {
      process.stderr.write(`  ${colorize("dim", "◇ Auto-compacting context...")}\n`);
      const result = await compactContext(provider, messages, config);
      messages = result.messages;
      session.compactedSummary = result.summary;
    }

    // ── Agent Loop ──
    abortController = new AbortController();

    try {
      const turn = await runAgentLoop({
        provider,
        toolRegistry,
        config,
        projectRoot,
        messages,
        approve: approveAction,
        onTurn: (t) => {
          addTurn(session, t);
          session.tasks = exportTasks();
          saveSession(projectRoot, session);
        },
      });

      // Add assistant response to conversation
      messages.push({ role: "assistant", content: turn.content });
    } catch (err: unknown) {
      spinner.stop();
      if (err instanceof Error && err.name === "AbortError") {
        process.stderr.write(`  ${colorize("warning", "◆ Generation aborted.")}\n\n`);
      } else {
        const msg = err instanceof Error ? err.message : String(err);
        process.stderr.write(`  ${colorize("error", `✗ ${msg}`)}\n\n`);
        log("error", "Agent loop error", { error: msg });
      }
    }

    process.stderr.write("\n");
  }

  // Save final state
  session.tasks = exportTasks();
  saveSession(projectRoot, session);
  prompt.close();
}

main().catch(err => {
  console.error("Fatal:", err);
  process.exit(1);
});
