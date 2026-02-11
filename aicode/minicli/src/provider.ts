import { RuntimeConfig } from "./config";

export type ChatRole = "system" | "user" | "assistant";

export interface ChatMessage {
  role: ChatRole;
  content: string;
}

export interface ChatRequest {
  model: string;
  timeoutMs: number;
  messages: ChatMessage[];
}

export interface ChatResponse {
  message: ChatMessage;
}

export interface LLMProvider {
  readonly id: string;
  complete(request: ChatRequest): Promise<ChatResponse>;
}

export class MockLLMProvider implements LLMProvider {
  readonly id = "mock";

  async complete(request: ChatRequest): Promise<ChatResponse> {
    const lastUserMessage = [...request.messages]
      .reverse()
      .find((message) => message.role === "user");

    if (!lastUserMessage || lastUserMessage.content.trim().length === 0) {
      throw new Error("No user message provided.");
    }

    return {
      message: {
        role: "assistant",
        content: `mock(${request.model}): ${lastUserMessage.content}`
      }
    };
  }
}

export function buildChatRequest(
  history: ChatMessage[],
  userInput: string,
  config: RuntimeConfig
): ChatRequest {
  return {
    model: config.model,
    timeoutMs: config.timeoutMs,
    messages: [
      ...history,
      {
        role: "user",
        content: userInput
      }
    ]
  };
}
