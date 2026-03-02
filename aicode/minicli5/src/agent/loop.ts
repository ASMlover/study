import type { Provider, Message, AgentTurn, AppConfig, ToolContext, StreamChunk } from "../types.js";
import { ToolRegistry } from "../tools/registry.js";
import { runCoder } from "./coder.js";
import { runReviewer } from "./reviewer.js";
import { runOrchestrator } from "./orchestrator.js";
import { spinner } from "../tui/spinner.js";
import { renderer } from "../tui/renderer.js";
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
  signal?: AbortSignal;
}

/**
 * Fast path: single streaming call with tools available.
 * If the LLM responds with text only (no tool calls), stream directly to the user.
 * Returns null if tool calls are present (needs full pipeline).
 */
async function tryDirectStream(
  provider: Provider,
  messages: Message[],
  tools: import("../types.js").ToolDefinition[],
  model?: string,
  signal?: AbortSignal,
): Promise<{ content: string; tokenUsage: { prompt: number; completion: number } } | null> {
  renderer.reset();
  let fullContent = "";
  let hasToolCalls = false;
  let totalPrompt = 0;
  let totalCompletion = 0;
  let firstContent = true;

  spinner.start("planning");

  for await (const chunk of provider.chatStream({
    messages,
    tools: tools.length > 0 ? tools : undefined,
    stream: true,
    model,
    signal,
  })) {
    // Check abort signal each chunk
    if (signal?.aborted) break;

    const delta = chunk.choices[0]?.delta;

    // If the LLM wants to call tools, abort the fast path
    if (delta?.tool_calls && delta.tool_calls.length > 0) {
      hasToolCalls = true;
      break;
    }

    if (delta?.content) {
      // Stop spinner on first content chunk
      if (firstContent) {
        spinner.stop();
        process.stderr.write("\n");
        firstContent = false;
      }
      fullContent += delta.content;
      const rendered = renderer.renderChunk(delta.content);
      if (rendered) process.stdout.write(rendered);
    }

    if (chunk.usage) {
      totalPrompt = chunk.usage.prompt_tokens;
      totalCompletion = chunk.usage.completion_tokens;
    }
  }

  if (signal?.aborted) {
    spinner.stop();
    const flushed = renderer.flush();
    if (flushed) process.stdout.write(flushed);
    throw new DOMException("The operation was aborted.", "AbortError");
  }

  if (hasToolCalls) {
    spinner.stop();
    // Erase any partial content already written (if any)
    if (fullContent) {
      process.stdout.write("\r\x1b[K");
    }
    return null;
  }

  const flushed = renderer.flush();
  if (flushed) process.stdout.write(flushed);
  process.stdout.write("\n");

  return { content: fullContent, tokenUsage: { prompt: totalPrompt, completion: totalCompletion } };
}

export async function runAgentLoop(opts: AgentLoopOptions): Promise<AgentTurn> {
  const { provider, toolRegistry, config, projectRoot, messages, approve, onTurn, signal } = opts;
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

  // ── Fast path: try a single streaming call ──
  // If the LLM responds without tool calls, we skip planner/reviewer/orchestrator entirely.
  const tools = toolRegistry.getDefinitions();
  const directResult = await tryDirectStream(provider, allMessages, tools, config.model, signal);

  if (directResult) {
    // Fast path succeeded — no tool calls needed
    totalPrompt = directResult.tokenUsage.prompt;
    totalCompletion = directResult.tokenUsage.completion;

    const totalTokens = totalPrompt + totalCompletion;
    process.stderr.write(`\n  ${t.muted}◇${t.reset} ${t.dim}${totalPrompt}→${totalCompletion} (${totalTokens} tokens)${t.reset}\n`);

    const turn: AgentTurn = {
      role: "assistant",
      content: directResult.content,
      tokenUsage: { prompt: totalPrompt, completion: totalCompletion },
      timestamp: Date.now(),
    };

    onTurn?.(turn);
    return turn;
  }

  // ── Full pipeline: tool calls detected ──
  // The LLM wants to use tools, so we run the full coder loop + orchestrator.

  spinner.start("coding");

  const coderResult = await runCoder(
    provider,
    allMessages,
    tools,
    toolRegistry,
    toolCtx,
    config.max_tool_rounds,
    config.model,
  );
  totalPrompt += coderResult.tokenUsage.prompt;
  totalCompletion += coderResult.tokenUsage.completion;

  // Only review if tools were actually called
  let reviewNotes = "";
  if (coderResult.toolCalls.length > 0) {
    spinner.setStage("reviewing");
    const review = await runReviewer(provider, coderResult.messages, config.model);
    reviewNotes = review.notes;
  }

  // ── Orchestrator (streaming final response) ──
  spinner.stop();
  process.stderr.write("\n");

  const orchResult = await runOrchestrator(
    provider,
    coderResult.messages,
    "",  // no separate plan notes
    reviewNotes,
    config.model,
  );
  totalPrompt += orchResult.tokenUsage.prompt;
  totalCompletion += orchResult.tokenUsage.completion;

  // Token usage display
  const totalTokens = totalPrompt + totalCompletion;
  process.stderr.write(`\n  ${t.muted}◇${t.reset} ${t.dim}${totalPrompt}→${totalCompletion} (${totalTokens} tokens)${t.reset}\n`);

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
