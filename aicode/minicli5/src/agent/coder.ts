import type { Provider, Message, ToolDefinition, ToolResult, ToolCall, StreamChunk } from "../types.js";
import { ToolRegistry } from "../tools/registry.js";
import type { ToolContext } from "../types.js";
import { renderToolCall, renderToolResult } from "../tui/renderer.js";
import { spinner } from "../tui/spinner.js";
import { log } from "../utils/logger.js";

export interface CoderResult {
  messages: Message[];
  toolCalls: Array<{ name: string; args: Record<string, unknown>; result: ToolResult }>;
  tokenUsage: { prompt: number; completion: number };
}

export async function runCoder(
  provider: Provider,
  messages: Message[],
  tools: ToolDefinition[],
  toolRegistry: ToolRegistry,
  toolCtx: ToolContext,
  maxRounds: number,
  model?: string,
): Promise<CoderResult> {
  const allMessages = [...messages];
  const allToolCalls: CoderResult["toolCalls"] = [];
  let totalPrompt = 0;
  let totalCompletion = 0;

  for (let round = 0; round < maxRounds; round++) {
    spinner.setStage("coding", `tool round ${round + 1}/${maxRounds}`);

    const resp = await provider.chat({
      messages: allMessages,
      tools,
      stream: false,
      model,
    });

    const usage = resp.usage;
    if (usage) {
      totalPrompt += usage.prompt_tokens;
      totalCompletion += usage.completion_tokens;
    }

    const choice = resp.choices[0];
    if (!choice) break;

    const msg = choice.message;
    allMessages.push(msg);

    // If no tool calls, we're done
    if (!msg.tool_calls || msg.tool_calls.length === 0) {
      break;
    }

    // Execute tool calls
    for (const tc of msg.tool_calls) {
      let args: Record<string, unknown>;
      try {
        args = JSON.parse(tc.function.arguments);
      } catch {
        args = {};
      }

      // Pause spinner for tool execution (approval prompts need clean terminal)
      spinner.stop();

      // Display tool call
      process.stderr.write(renderToolCall(tc.function.name, args) + "\n");

      const result = await toolRegistry.execute(tc.function.name, args, toolCtx);

      // Display result
      process.stderr.write(renderToolResult(result) + "\n");

      allToolCalls.push({ name: tc.function.name, args, result });

      // Add tool result message
      allMessages.push({
        role: "tool",
        content: result.success ? result.output : `Error: ${result.error}\n${result.output}`,
        tool_call_id: tc.id,
      });

      // Resume spinner for next round
      spinner.start("coding");
    }
  }

  return {
    messages: allMessages,
    toolCalls: allToolCalls,
    tokenUsage: { prompt: totalPrompt, completion: totalCompletion },
  };
}
