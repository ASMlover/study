import type { Provider, Message, Session, AppConfig } from "../types.js";
import { estimateMessagesTokens } from "../utils/tokens.js";
import { log } from "../utils/logger.js";

export function shouldCompact(messages: Message[], config: AppConfig): boolean {
  const used = estimateMessagesTokens(messages);
  const threshold = config.context_max_tokens * config.compact_threshold;
  return used > threshold;
}

export async function compactContext(
  provider: Provider,
  messages: Message[],
  config: AppConfig,
): Promise<{ messages: Message[]; summary: string }> {
  // Keep system messages and last 3 user/assistant pairs
  const systemMsgs = messages.filter(m => m.role === "system");
  const nonSystem = messages.filter(m => m.role !== "system");

  // Find the last 6 non-system messages (roughly 3 pairs)
  const keepCount = Math.min(6, nonSystem.length);
  const toSummarize = nonSystem.slice(0, nonSystem.length - keepCount);
  const toKeep = nonSystem.slice(nonSystem.length - keepCount);

  if (toSummarize.length === 0) {
    return { messages, summary: "" };
  }

  // Summarize older messages
  const summaryPrompt: Message[] = [
    {
      role: "system",
      content: "Summarize the following conversation concisely. Focus on key decisions, actions taken, and important context. Keep under 500 words.",
    },
    {
      role: "user",
      content: toSummarize.map(m => `[${m.role}]: ${m.content ?? "(tool call)"}`).join("\n\n"),
    },
  ];

  let summary = "";
  try {
    const resp = await provider.chat({
      messages: summaryPrompt,
      stream: false,
      model: config.model,
      max_tokens: 1024,
    });
    summary = resp.choices[0]?.message?.content ?? "";
  } catch (err) {
    log("error", "Compaction summary failed", { error: String(err) });
    // Fallback: just truncate
    summary = `[Previous conversation: ${toSummarize.length} messages compacted]`;
  }

  const compactedMessages: Message[] = [
    ...systemMsgs,
    { role: "assistant", content: `[Conversation summary: ${summary}]` },
    ...toKeep,
  ];

  log("info", "Context compacted", {
    before: messages.length,
    after: compactedMessages.length,
    summarized: toSummarize.length,
  });

  return { messages: compactedMessages, summary };
}
