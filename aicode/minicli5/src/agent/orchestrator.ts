import type { Provider, Message, StreamChunk } from "../types.js";
import { renderer } from "../tui/renderer.js";
import { log } from "../utils/logger.js";

const ORCHESTRATOR_SYSTEM = `You are the final response synthesizer for an AI coding agent. Given the conversation context, planning notes, tool results, and review feedback, compose a clear, helpful final response to the user.

Format your response in markdown. Be concise but thorough. If tools were used, briefly summarize what was done and the results.`;

export async function runOrchestrator(
  provider: Provider,
  messages: Message[],
  planNotes: string,
  reviewNotes: string,
  model?: string,
): Promise<{ content: string; tokenUsage: { prompt: number; completion: number } }> {
  const contextMsg: Message = {
    role: "system",
    content: [
      ORCHESTRATOR_SYSTEM,
      planNotes ? `\n[Planning notes: ${planNotes}]` : "",
      reviewNotes ? `\n[Review notes: ${reviewNotes}]` : "",
    ].join(""),
  };

  const orchMessages: Message[] = [contextMsg, ...messages];

  renderer.reset();
  let fullContent = "";
  let totalPrompt = 0;
  let totalCompletion = 0;

  try {
    for await (const chunk of provider.chatStream({
      messages: orchMessages,
      stream: true,
      model,
    })) {
      const delta = chunk.choices[0]?.delta;
      if (delta?.content) {
        fullContent += delta.content;
        const rendered = renderer.renderChunk(delta.content);
        if (rendered) process.stdout.write(rendered);
      }
      if (chunk.usage) {
        totalPrompt = chunk.usage.prompt_tokens;
        totalCompletion = chunk.usage.completion_tokens;
      }
    }

    // Flush any remaining buffered content
    const flushed = renderer.flush();
    if (flushed) process.stdout.write(flushed);
    process.stdout.write("\n");
  } catch (err) {
    log("error", "Orchestrator streaming failed", { error: String(err) });
    process.stdout.write("\n(streaming error)\n");
  }

  return {
    content: fullContent,
    tokenUsage: { prompt: totalPrompt, completion: totalCompletion },
  };
}
