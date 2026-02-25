import { RuntimeConfig } from "./config";

export type ChatRole = "system" | "user" | "assistant" | "tool";

export interface ChatMessage {
  role: ChatRole;
  content: string;
  tool_call_id?: string;
  name?: string;
}

export interface ToolCall {
  id: string;
  type: "function";
  function: {
    name: string;
    arguments: string | Record<string, unknown>;
  };
}

export interface ChatChoice {
  message?: {
    role?: ChatRole;
    content?: string;
    tool_calls?: ToolCall[];
  };
  delta?: {
    content?: string;
    reasoning_content?: string;
  };
}

export interface ChatResponse {
  choices: ChatChoice[];
}

export interface ProviderRequest {
  messages: ChatMessage[];
  tools?: unknown[];
  stream: boolean;
  temperature: number;
  max_tokens: number;
}

export class ProviderError extends Error {
  constructor(message: string, public readonly retryable = false) {
    super(message);
    this.name = "ProviderError";
  }
}

export class GLMProvider {
  constructor(private readonly config: RuntimeConfig, private readonly fetchImpl: typeof fetch = fetch) {
    if (config.model !== "glm-5") {
      throw new ProviderError("model must be glm-5");
    }
  }

  async chat(req: ProviderRequest): Promise<ChatResponse | AsyncGenerator<ChatResponse>> {
    const payload = {
      model: "glm-5",
      messages: req.messages,
      tools: req.tools,
      stream: req.stream,
      temperature: req.temperature,
      max_tokens: req.max_tokens
    };

    let lastError: Error | undefined;
    for (let attempt = 0; attempt <= this.config.max_retries; attempt += 1) {
      const controller = new AbortController();
      const timer = setTimeout(() => controller.abort(), this.config.timeout_ms);
      try {
        const res = await this.fetchImpl(`${this.config.base_url.replace(/\/+$/, "")}/chat/completions`, {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${this.config.api_key}`
          },
          body: JSON.stringify(payload),
          signal: controller.signal
        });

        if (!res.ok) {
          if (res.status === 401) {
            throw new ProviderError("authentication failed (401)", false);
          }
          if (res.status === 429) {
            throw new ProviderError("rate limited (429)", true);
          }
          if (res.status >= 500) {
            throw new ProviderError(`provider error (${res.status})`, true);
          }
          throw new ProviderError(`request failed (${res.status})`, false);
        }

        if (req.stream) {
          return this.parseStream(res);
        }

        const json = (await res.json()) as ChatResponse;
        return json;
      } catch (error) {
        const e = error as Error;
        lastError = e;
        const retryable = error instanceof ProviderError ? error.retryable : true;
        if (!retryable || attempt === this.config.max_retries) {
          break;
        }
        await new Promise((resolve) => setTimeout(resolve, 2 ** attempt * 200));
      } finally {
        clearTimeout(timer);
      }
    }

    throw lastError ?? new ProviderError("provider error");
  }

  private async *parseStream(res: Response): AsyncGenerator<ChatResponse> {
    const body = res.body;
    if (!body) {
      return;
    }
    const reader = body.getReader();
    const decoder = new TextDecoder();
    let buffer = "";
    while (true) {
      const { value, done } = await reader.read();
      if (done) {
        break;
      }
      buffer += decoder.decode(value, { stream: true });
      let idx = buffer.indexOf("\n");
      while (idx >= 0) {
        const line = buffer.slice(0, idx).trim();
        buffer = buffer.slice(idx + 1);
        if (line.startsWith("data: ")) {
          const payload = line.slice(6).trim();
          if (payload === "[DONE]") {
            return;
          }
          try {
            yield JSON.parse(payload) as ChatResponse;
          } catch {
            // ignore malformed chunk
          }
        }
        idx = buffer.indexOf("\n");
      }
    }
  }
}
