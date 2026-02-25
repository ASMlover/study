import { ChatMessage, ChatResponse, GLMProvider, ToolCall } from "./provider";
import { RuntimeConfig } from "./config";
import { SessionMessage } from "./session";
import { ToolRegistry } from "./tools";

export interface AgentCallbacks {
  onStage: (stage: string, detail: string) => void;
  onTool: (name: string, output: string) => void;
  onDelta: (chunk: string) => void;
}

export interface AgentResult {
  final: string;
  stages: string[];
}

function extractAssistant(response: ChatResponse): { content: string; toolCalls: ToolCall[] } {
  const msg = response.choices?.[0]?.message;
  return {
    content: msg?.content ?? "",
    toolCalls: msg?.tool_calls ?? []
  };
}

function asChatMessages(messages: SessionMessage[]): ChatMessage[] {
  return messages.map((m) => ({
    role: m.role === "tool" ? "tool" : m.role,
    content: m.content,
    tool_call_id: m.tool_call_id,
    name: m.name
  }));
}

export async function runMultiAgentRound(
  input: string,
  sessionMessages: SessionMessage[],
  provider: GLMProvider,
  tools: ToolRegistry,
  config: RuntimeConfig,
  callbacks: AgentCallbacks
): Promise<AgentResult> {
  const stages: string[] = [];
  const working = [...asChatMessages(sessionMessages), { role: "user" as const, content: input }];

  const plannerPrompt: ChatMessage = {
    role: "system",
    content: "You are planner agent. Return concise execution plan for coder and reviewer."
  };
  callbacks.onStage("planner", "planning");
  stages.push("planner");
  const planner = await provider.chat({
    messages: [plannerPrompt, ...working],
    stream: false,
    temperature: config.temperature,
    max_tokens: Math.min(config.max_tokens, 512)
  });
  if (Symbol.asyncIterator in Object(planner)) {
    throw new Error("planner should not stream");
  }
  const planText = extractAssistant(planner as ChatResponse).content;

  callbacks.onStage("coder", "tool planning and execution");
  stages.push("coder");
  const coderContext: ChatMessage[] = [
    {
      role: "system",
      content: "You are coder agent. Use tools when needed and produce actionable answer."
    },
    { role: "system", content: `planner_notes:\n${planText}` },
    ...working
  ];

  let rounds = 0;
  while (rounds < config.agent_max_rounds) {
    rounds += 1;
    const coding = await provider.chat({
      messages: coderContext,
      tools: tools.definitions(),
      stream: false,
      temperature: config.temperature,
      max_tokens: config.max_tokens
    });
    if (Symbol.asyncIterator in Object(coding)) {
      throw new Error("coder planning should not stream");
    }

    const parsed = extractAssistant(coding as ChatResponse);
    coderContext.push({ role: "assistant", content: parsed.content });
    if (parsed.toolCalls.length === 0) {
      break;
    }

    for (const call of parsed.toolCalls) {
      const toolName = call.function.name;
      const result = tools.execute(toolName, call.function.arguments, JSON.stringify(sessionMessages, null, 2));
      callbacks.onTool(toolName, result.output);
      coderContext.push({
        role: "tool",
        tool_call_id: call.id,
        name: toolName,
        content: result.output
      });
      if (result.requiresApproval) {
        coderContext.push({ role: "assistant", content: "Tool execution denied pending approval in strict mode." });
      }
    }
  }

  callbacks.onStage("reviewer", "quality review");
  stages.push("reviewer");
  const reviewer = await provider.chat({
    messages: [
      {
        role: "system",
        content: "You are reviewer agent. Improve clarity and safety before final user response."
      },
      ...coderContext
    ],
    stream: false,
    temperature: config.temperature,
    max_tokens: Math.min(config.max_tokens, 1024)
  });

  let reviewerText = "";
  if (!(Symbol.asyncIterator in Object(reviewer))) {
    reviewerText = extractAssistant(reviewer as ChatResponse).content;
  }

  callbacks.onStage("orchestrator", "final streaming reply");
  stages.push("orchestrator");
  const finalReq = await provider.chat({
    messages: [
      {
        role: "system",
        content: "You are the final assistant. Provide concise, user-facing answer in markdown."
      },
      ...coderContext,
      { role: "system", content: `reviewer_notes:\n${reviewerText}` }
    ],
    stream: config.stream,
    temperature: config.temperature,
    max_tokens: config.max_tokens
  });

  let final = "";
  if (Symbol.asyncIterator in Object(finalReq)) {
    for await (const chunk of finalReq as AsyncGenerator<ChatResponse>) {
      const delta = chunk.choices?.[0]?.delta?.content ?? chunk.choices?.[0]?.delta?.reasoning_content ?? "";
      if (delta.length > 0) {
        final += delta;
        callbacks.onDelta(delta);
      }
    }
  } else {
    final = extractAssistant(finalReq as ChatResponse).content;
    callbacks.onDelta(final);
  }

  return { final, stages };
}
