import { BaseProvider } from "./base.js";
import { withRetry } from "./retry.js";
import { log } from "../utils/logger.js";
import type { ChatRequest, ChatResponse, StreamChunk, AppConfig } from "../types.js";

export class GlmProvider extends BaseProvider {
  private apiBase: string;
  private apiKey: string;
  private model: string;
  private maxRetries: number;
  private defaultMaxTokens: number;
  private defaultTemperature: number;

  constructor(config: AppConfig) {
    super();
    this.apiBase = config.api_base;
    this.apiKey = config.api_key;
    this.model = config.model;
    this.maxRetries = config.max_retries;
    this.defaultMaxTokens = config.max_tokens;
    this.defaultTemperature = config.temperature;
  }

  private buildBody(req: ChatRequest): Record<string, unknown> {
    const body: Record<string, unknown> = {
      model: req.model ?? this.model,
      messages: req.messages,
      max_tokens: req.max_tokens ?? this.defaultMaxTokens,
      temperature: req.temperature ?? this.defaultTemperature,
      stream: req.stream ?? false,
    };
    if (req.tools && req.tools.length > 0) {
      body.tools = req.tools;
      body.tool_choice = "auto";
    }
    return body;
  }

  private get headers(): Record<string, string> {
    return {
      "Content-Type": "application/json",
      Authorization: `Bearer ${this.apiKey}`,
    };
  }

  async chat(req: ChatRequest): Promise<ChatResponse> {
    return withRetry(async () => {
      const body = this.buildBody({ ...req, stream: false });
      const res = await fetch(`${this.apiBase}/chat/completions`, {
        method: "POST",
        headers: this.headers,
        body: JSON.stringify(body),
      });

      if (!res.ok) {
        const text = await res.text();
        log("error", "GLM API error", { status: res.status, body: text });
        const err = new Error(`GLM API error ${res.status}: ${text}`);
        (err as any).status = res.status;
        throw err;
      }

      return (await res.json()) as ChatResponse;
    }, { maxRetries: this.maxRetries });
  }

  async *chatStream(req: ChatRequest): AsyncIterable<StreamChunk> {
    const body = this.buildBody({ ...req, stream: true });

    const res = await withRetry(async () => {
      const r = await fetch(`${this.apiBase}/chat/completions`, {
        method: "POST",
        headers: this.headers,
        body: JSON.stringify(body),
      });
      if (!r.ok) {
        const text = await r.text();
        log("error", "GLM stream API error", { status: r.status, body: text });
        const err = new Error(`GLM API error ${r.status}: ${text}`);
        (err as any).status = r.status;
        throw err;
      }
      return r;
    }, { maxRetries: this.maxRetries });

    const reader = res.body?.getReader();
    if (!reader) throw new Error("No response body");

    const decoder = new TextDecoder();
    let buffer = "";

    try {
      while (true) {
        const { done, value } = await reader.read();
        if (done) break;

        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split("\n");
        buffer = lines.pop() ?? "";

        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed || !trimmed.startsWith("data:")) continue;

          const data = trimmed.slice(5).trim();
          if (data === "[DONE]") return;

          try {
            yield JSON.parse(data) as StreamChunk;
          } catch {
            log("warn", "Failed to parse SSE chunk", { data });
          }
        }
      }
    } finally {
      reader.releaseLock();
    }
  }
}
