import type { Provider, Message } from "../types.js";
import { log } from "../utils/logger.js";

const PLANNER_SYSTEM = `You are a strategic planner for an AI coding agent. Given the user's request and conversation context, produce a brief plan of action. Focus on:
1. What needs to be done (decompose into steps)
2. Which tools will be needed
3. Key risks or considerations

Keep your plan concise (3-8 bullet points). Do NOT execute any actions - just plan.`;

export async function runPlanner(
  provider: Provider,
  messages: Message[],
  model?: string,
): Promise<string> {
  const plannerMessages: Message[] = [
    { role: "system", content: PLANNER_SYSTEM },
    ...messages,
  ];

  try {
    const resp = await provider.chat({
      messages: plannerMessages,
      stream: false,
      model,
      max_tokens: 1024,
    });

    const plan = resp.choices[0]?.message?.content ?? "";
    log("info", "Planner output", { plan });
    return plan;
  } catch (err) {
    log("error", "Planner failed", { error: String(err) });
    return "";
  }
}
