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
  readonly code: "http_401" | "http_429" | "http_5xx" | "http_other" | "timeout" | "network";
  readonly statusCode?: number;
  readonly retryable: boolean;

  constructor(
    message: string,
    options?: {
      code?: "http_401" | "http_429" | "http_5xx" | "http_other" | "timeout" | "network";
      statusCode?: number;
      retryable?: boolean;
    }
  ) {
    super(message);
    this.name = "ProviderNetworkError";
    this.code = options?.code ?? "network";
    this.statusCode = options?.statusCode;
    this.retryable = options?.retryable ?? false;
  }
}

export interface GLMProviderOptions {
  apiKey: string;
  baseUrl?: string;
  defaultModel?: string;
  maxRetries?: number;
  retryDelayMs?: number;
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
  private readonly maxRetries: number;
  private readonly retryDelayMs: number;
  private readonly fetchImpl: typeof fetch;

  constructor(options: GLMProviderOptions) {
    this.apiKey = options.apiKey;
    this.baseUrl =
      options.baseUrl ?? "https://open.bigmodel.cn/api/paas/v4";
    this.defaultModel = options.defaultModel ?? "glm-4";
    this.maxRetries = Math.max(0, options.maxRetries ?? 2);
    this.retryDelayMs = Math.max(0, options.retryDelayMs ?? 200);
    this.fetchImpl = options.fetchImpl ?? fetch;
  }

  async complete(request: ChatRequest): Promise<ChatResponse> {
    const targetModel = request.model.trim().length
      ? request.model
      : this.defaultModel;
    const url = buildOpenAICompatibleUrl(this.baseUrl);
    for (let attempt = 0; attempt <= this.maxRetries; attempt += 1) {
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
          const mappedError = mapHttpStatusError(response.status);
          if (mappedError.retryable && attempt < this.maxRetries) {
            await this.sleepBeforeRetry(attempt);
            continue;
          }
          throw mappedError;
        }

        const payload = (await response.json()) as OpenAICompatibleResponse;
        return mapOpenAICompatibleResponse(payload);
      } catch (error) {
        if (error instanceof ProviderNetworkError) {
          if (error.retryable && attempt < this.maxRetries) {
            await this.sleepBeforeRetry(attempt);
            continue;
          }
          throw error;
        }

        if (isAbortError(error)) {
          throw new ProviderNetworkError(
            `GLM request timed out after ${request.timeoutMs}ms.`,
            {
              code: "timeout",
              retryable: false
            }
          );
        }

        const message = error instanceof Error ? error.message : String(error);
        const networkError = new ProviderNetworkError(
          `GLM network error: ${message}`,
          {
            code: "network",
            retryable: true
          }
        );

        if (attempt < this.maxRetries) {
          await this.sleepBeforeRetry(attempt);
          continue;
        }

        throw networkError;
      } finally {
        clearTimeout(timeout);
      }
    }

    throw new ProviderNetworkError("GLM request failed after retries.", {
      code: "network",
      retryable: false
    });
  }

  private async sleepBeforeRetry(attempt: number): Promise<void> {
    const delay = this.retryDelayMs * (attempt + 1);
    if (delay <= 0) {
      return;
    }
    await new Promise<void>((resolve) => {
      setTimeout(resolve, delay);
    });
  }
}

function isAbortError(error: unknown): boolean {
  if (!(error instanceof Error)) {
    return false;
  }
  return error.name === "AbortError";
}

function mapHttpStatusError(status: number): ProviderNetworkError {
  if (status === 401) {
    return new ProviderNetworkError(
      "Authentication failed (401). Check your API key and login again.",
      {
        code: "http_401",
        statusCode: status,
        retryable: false
      }
    );
  }

  if (status === 429) {
    return new ProviderNetworkError(
      "Rate limited by provider (429). Retrying may help shortly.",
      {
        code: "http_429",
        statusCode: status,
        retryable: true
      }
    );
  }

  if (status >= 500 && status <= 599) {
    return new ProviderNetworkError(
      `Provider service error (${status}).`,
      {
        code: "http_5xx",
        statusCode: status,
        retryable: true
      }
    );
  }

  return new ProviderNetworkError(
    `GLM request failed with status ${status}.`,
    {
      code: "http_other",
      statusCode: status,
      retryable: false
    }
  );
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
