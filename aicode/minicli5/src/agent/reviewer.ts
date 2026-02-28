import type { Provider, Message } from "../types.js";
import { log } from "../utils/logger.js";

const REVIEWER_SYSTEM = `You are a quality reviewer for an AI coding agent. Review the actions taken and results produced. Check for:
1. Correctness - did the actions achieve the goal?
2. Completeness - is anything missing?
3. Safety - any risky operations performed?

Respond with either "APPROVED" if everything looks good, or specific issues that need addressing.
Keep your review brief (1-3 sentences).`;

export async function runReviewer(
  provider: Provider,
  messages: Message[],
  model?: string,
): Promise<{ approved: boolean; notes: string }> {
  const reviewMessages: Message[] = [
    { role: "system", content: REVIEWER_SYSTEM },
    ...messages,
  ];

  try {
    const resp = await provider.chat({
      messages: reviewMessages,
      stream: false,
      model,
      max_tokens: 512,
    });

    const content = resp.choices[0]?.message?.content ?? "APPROVED";
    const approved = content.toUpperCase().includes("APPROVED");
    log("info", "Reviewer output", { approved, notes: content });
    return { approved, notes: content };
  } catch (err) {
    log("error", "Reviewer failed", { error: String(err) });
    return { approved: true, notes: "" };
  }
}
