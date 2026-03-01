export class SubAgentRunner {
  constructor({ modelClient, tools, baseMessages }) {
    this.modelClient = modelClient;
    this.tools = tools;
    this.baseMessages = baseMessages;
  }

  async run(goal, maxSteps = 4) {
    const messages = [
      ...this.baseMessages,
      {
        role: "system",
        content:
          "You are a focused sub-agent. Solve the scoped goal quickly, use tools only when necessary, and return a concise result.",
      },
      {
        role: "user",
        content: `Sub-agent goal: ${goal}`,
      },
    ];

    let finalText = "";
    for (let step = 0; step < maxSteps; step += 1) {
      const result = await this.modelClient.streamChat({
        messages,
        tools: this.tools,
      });

      if (result.content) {
        finalText = result.content;
        messages.push({ role: "assistant", content: result.content });
      }

      if (!result.toolCalls.length) {
        break;
      }

      for (const tc of result.toolCalls) {
        messages.push({
          role: "assistant",
          content: "",
          tool_calls: [tc],
        });
        messages.push({
          role: "tool",
          tool_call_id: tc.id,
          content: "Sub-agent tool use is disabled in this lightweight mode.",
        });
      }
    }

    return finalText || "Sub-agent finished with no content.";
  }
}
