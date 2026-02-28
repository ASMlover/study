import type { Provider, Message, AgentTurn, AppConfig, ToolContext } from "../types.js";
import { ToolRegistry } from "../tools/registry.js";
import { runPlanner } from "./planner.js";
import { runCoder } from "./coder.js";
import { runReviewer } from "./reviewer.js";
import { runOrchestrator } from "./orchestrator.js";
import { spinner } from "../tui/spinner.js";
import { getTheme } from "../tui/theme.js";
import { log } from "../utils/logger.js";

const SYSTEM_PROMPT = `You are MiniCLI5, an AI coding assistant running in a terminal. You help users with software engineering tasks by reading, writing, and analyzing code. You have access to tools for file operations, search, and shell commands.

Guidelines:
- Read files before modifying them
- Validate paths stay within the project root
- Be concise and focused on the task
- Use tools when needed, don't just describe what you would do
- For file writes and shell commands, explain what you're doing`;

export interface AgentLoopOptions {
  provider: Provider;
  toolRegistry: ToolRegistry;
  config: AppConfig;
  projectRoot: string;
  messages: Message[];
  approve?: (action: string, detail: string) => Promise<boolean>;
  onTurn?: (turn: AgentTurn) => void;
}

export async function runAgentLoop(opts: AgentLoopOptions): Promise<AgentTurn> {
  const { provider, toolRegistry, config, projectRoot, messages, approve, onTurn } = opts;
  const t = getTheme();

  const toolCtx: ToolContext = {
    projectRoot,
    approve,
  };

  // Ensure system prompt is first
  const allMessages: Message[] = [
    { role: "system", content: SYSTEM_PROMPT },
    ...messages,
  ];

  let totalPrompt = 0;
  let totalCompletion = 0;

  // ── Stage 1: Planner ──
  spinner.start("planning");
  const planNotes = await runPlanner(provider, allMessages, config.model);

  // ── Stage 2: Coder (tool loop) ──
  spinner.setStage("coding");

  // Inject plan as assistant context
  const coderMessages: Message[] = [...allMessages];
  if (planNotes) {
    coderMessages.push({
      role: "assistant",
      content: `[Internal plan: ${planNotes}]\n\nLet me work on this.`,
    });
  }

  const coderResult = await runCoder(
    provider,
    coderMessages,
    toolRegistry.getDefinitions(),
    toolRegistry,
    toolCtx,
    config.max_tool_rounds,
    config.model,
  );
  totalPrompt += coderResult.tokenUsage.prompt;
  totalCompletion += coderResult.tokenUsage.completion;

  // ── Stage 3: Reviewer ──
  spinner.setStage("reviewing");
  const review = await runReviewer(provider, coderResult.messages, config.model);

  // ── Stage 4: Orchestrator (streaming response) ──
  spinner.setStage("orchestrating");
  spinner.stop();

  process.stderr.write("\n");

  const orchResult = await runOrchestrator(
    provider,
    coderResult.messages,
    planNotes,
    review.notes,
    config.model,
  );
  totalPrompt += orchResult.tokenUsage.prompt;
  totalCompletion += orchResult.tokenUsage.completion;

  // Token usage display
  const totalTokens = totalPrompt + totalCompletion;
  process.stderr.write(`\n${t.dim}tokens: ${totalPrompt}→${totalCompletion} (${totalTokens} total)${t.reset}\n`);

  const turn: AgentTurn = {
    role: "assistant",
    content: orchResult.content,
    toolCalls: coderResult.toolCalls,
    tokenUsage: { prompt: totalPrompt, completion: totalCompletion },
    timestamp: Date.now(),
  };

  onTurn?.(turn);
  return turn;
}
