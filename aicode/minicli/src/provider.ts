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

export class ProviderNetworkError extends Error {
  constructor(message: string) {
    super(message);
    this.name = "ProviderNetworkError";
  }
}

export interface GLMProviderOptions {
  apiKey: string;
  baseUrl?: string;
  defaultModel?: string;
  fetchImpl?: typeof fetch;
}

interface OpenAICompatibleChoice {
  message?: {
    role?: ChatRole;
    content?: string;
  };
}

interface OpenAICompatibleResponse {
  choices?: OpenAICompatibleChoice[];
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

export function buildOpenAICompatibleUrl(baseUrl: string): string {
  return `${baseUrl.replace(/\/+$/, "")}/chat/completions`;
}

export function buildGLMHeaders(apiKey: string): Record<string, string> {
  return {
    "Content-Type": "application/json",
    Authorization: `Bearer ${apiKey}`
  };
}

export function mapOpenAICompatibleResponse(
  payload: OpenAICompatibleResponse
): ChatResponse {
  const choice = payload.choices?.[0];
  if (!choice?.message || typeof choice.message.content !== "string") {
    throw new Error("No choices returned by provider.");
  }

  return {
    message: {
      role: choice.message.role ?? "assistant",
      content: choice.message.content
    }
  };
}

export class GLMOpenAIProvider implements LLMProvider {
  readonly id = "glm-openai-compatible";
  private readonly apiKey: string;
  private readonly baseUrl: string;
  private readonly defaultModel: string;
  private readonly fetchImpl: typeof fetch;

  constructor(options: GLMProviderOptions) {
    this.apiKey = options.apiKey;
    this.baseUrl =
      options.baseUrl ?? "https://open.bigmodel.cn/api/paas/v4";
    this.defaultModel = options.defaultModel ?? "glm-4";
    this.fetchImpl = options.fetchImpl ?? fetch;
  }

  async complete(request: ChatRequest): Promise<ChatResponse> {
    const targetModel = request.model.trim().length
      ? request.model
      : this.defaultModel;
    const url = buildOpenAICompatibleUrl(this.baseUrl);
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), request.timeoutMs);

    try {
      const response = await this.fetchImpl(url, {
        method: "POST",
        headers: buildGLMHeaders(this.apiKey),
        body: JSON.stringify({
          model: targetModel,
          messages: request.messages
        }),
        signal: controller.signal
      });

      if (!response.ok) {
        throw new ProviderNetworkError(
          `GLM request failed with status ${response.status}.`
        );
      }

      const payload = (await response.json()) as OpenAICompatibleResponse;
      return mapOpenAICompatibleResponse(payload);
    } catch (error) {
      if (error instanceof ProviderNetworkError) {
        throw error;
      }
      const message = error instanceof Error ? error.message : String(error);
      throw new ProviderNetworkError(`GLM network error: ${message}`);
    } finally {
      clearTimeout(timeout);
    }
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
