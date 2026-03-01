import { logger } from '../utils/logger.js';

export interface ToolCall {
  id: string;
  type: 'function';
  function: {
    name: string;
    arguments: string;
  };
}

export interface ChatMessage {
  role: 'system' | 'user' | 'assistant' | 'tool';
  content: string;
  tool_calls?: ToolCall[];
  tool_call_id?: string;
}

export interface ToolDefinition {
  type: 'function';
  function: {
    name: string;
    description: string;
    parameters: object;
  };
}

export interface StreamDelta {
  content?: string;
  tool_calls?: Partial<ToolCall>[];
  finish_reason?: string;
}

export type StreamCallback = (delta: StreamDelta) => void;

export interface GLM5ProviderConfig {
  apiKey: string;
  baseUrl: string;
  model: string;
  maxRetries?: number;
  retryDelay?: number;
}

interface SSEChoice {
  delta: {
    content?: string;
    tool_calls?: Array<{
      id?: string;
      type?: 'function';
      function?: {
        name?: string;
        arguments?: string;
      };
    }>;
  };
  finish_reason?: string;
}

interface SSEResponse {
  choices: SSEChoice[];
}

export class GLM5Provider {
  private apiKey: string;
  private baseUrl: string;
  private model: string;
  private maxRetries: number;
  private retryDelay: number;

  constructor(config: GLM5ProviderConfig) {
    this.apiKey = config.apiKey;
    this.baseUrl = config.baseUrl.replace(/\/$/, '');
    this.model = config.model;
    this.maxRetries = config.maxRetries ?? 3;
    this.retryDelay = config.retryDelay ?? 1000;
  }

  async chat(
    messages: ChatMessage[],
    tools: ToolDefinition[],
    onStream: StreamCallback,
    options?: { signal?: AbortSignal }
  ): Promise<ChatMessage> {
    const requestBody: Record<string, unknown> = {
      model: this.model,
      messages,
      stream: true,
    };

    if (tools.length > 0) {
      requestBody.tools = tools;
    }

    return this.executeWithRetry(async () => {
      const response = await this.fetchWithTimeout(
        `${this.baseUrl}/chat/completions`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            Authorization: `Bearer ${this.apiKey}`,
          },
          body: JSON.stringify(requestBody),
          signal: options?.signal,
        }
      );

      if (!response.ok) {
        const errorText = await response.text();
        throw new GLM5APIError(
          `API request failed: ${response.status} ${response.statusText}`,
          response.status,
          errorText
        );
      }

      if (!response.body) {
        throw new Error('Response body is null');
      }

      return this.processSSEStream(response.body, onStream, options?.signal);
    });
  }

  async chatSync(
    messages: ChatMessage[],
    options?: { signal?: AbortSignal }
  ): Promise<string> {
    const requestBody = {
      model: this.model,
      messages,
      stream: false,
    };

    return this.executeWithRetry(async () => {
      const response = await this.fetchWithTimeout(
        `${this.baseUrl}/chat/completions`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            Authorization: `Bearer ${this.apiKey}`,
          },
          body: JSON.stringify(requestBody),
          signal: options?.signal,
        }
      );

      if (!response.ok) {
        const errorText = await response.text();
        throw new GLM5APIError(
          `API request failed: ${response.status} ${response.statusText}`,
          response.status,
          errorText
        );
      }

      const data = await response.json() as { choices?: { message?: { content?: string } }[] };
      return data.choices?.[0]?.message?.content ?? '';
    });
  }

  private async executeWithRetry<T>(fn: () => Promise<T>): Promise<T> {
    let lastError: Error | null = null;

    for (let attempt = 0; attempt < this.maxRetries; attempt++) {
      try {
        return await fn();
      } catch (error) {
        lastError = error as Error;

        if (error instanceof GLM5APIError && error.status === 401) {
          throw error;
        }

        if (error instanceof Error && error.name === 'AbortError') {
          throw error;
        }

        if (attempt < this.maxRetries - 1) {
          const delay = this.retryDelay * Math.pow(2, attempt);
          logger.warn(
            `Request failed, retrying in ${delay}ms (attempt ${attempt + 1}/${this.maxRetries})`
          );
          await this.sleep(delay);
        }
      }
    }

    throw lastError;
  }

  private async processSSEStream(
    body: ReadableStream<Uint8Array>,
    onStream: StreamCallback,
    signal?: AbortSignal
  ): Promise<ChatMessage> {
    const reader = body.getReader();
    const decoder = new TextDecoder();
    let buffer = '';
    let fullContent = '';
    const toolCalls: Map<number, ToolCall> = new Map();

    try {
      while (true) {
        if (signal?.aborted) {
          reader.cancel();
          break;
        }

        const { done, value } = await reader.read();

        if (done) {
          break;
        }

        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split('\n');
        buffer = lines.pop() ?? '';

        for (const line of lines) {
          const trimmed = line.trim();

          if (!trimmed || !trimmed.startsWith('data: ')) {
            continue;
          }

          const data = trimmed.slice(6);

          if (data === '[DONE]') {
            continue;
          }

          try {
            const parsed: SSEResponse = JSON.parse(data);
            const choice = parsed.choices?.[0];

            if (!choice) {
              continue;
            }

            const delta: StreamDelta = {};

            if (choice.delta?.content) {
              delta.content = choice.delta.content;
              fullContent += choice.delta.content;
            }

            if (choice.delta?.tool_calls) {
              delta.tool_calls = [];

              for (const tc of choice.delta.tool_calls) {
                if (tc.id) {
                  const toolCall: ToolCall = {
                    id: tc.id,
                    type: 'function',
                    function: {
                      name: tc.function?.name ?? '',
                      arguments: tc.function?.arguments ?? '',
                    },
                  };
                  toolCalls.set(parseInt(tc.id) || toolCalls.size, toolCall);
                  delta.tool_calls.push(toolCall);
                } else if (tc.function?.arguments) {
                  const index = toolCalls.size - 1;
                  const existing = toolCalls.get(index);
                  if (existing) {
                    existing.function.arguments += tc.function.arguments;
                    delta.tool_calls.push(existing);
                  }
                }
              }
            }

            if (choice.finish_reason) {
              delta.finish_reason = choice.finish_reason;
            }

            onStream(delta);
          } catch {
            logger.debug('Failed to parse SSE data:', data as string);
          }
        }
      }
    } finally {
      reader.releaseLock();
    }

    const result: ChatMessage = {
      role: 'assistant',
      content: fullContent,
    };

    if (toolCalls.size > 0) {
      result.tool_calls = Array.from(toolCalls.values());
    }

    return result;
  }

  private async fetchWithTimeout(
    url: string,
    options: RequestInit & { signal?: AbortSignal },
    timeout = 120000
  ): Promise<Response> {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), timeout);

    const signal = options.signal
      ? this.combineSignals(options.signal, controller.signal)
      : controller.signal;

    try {
      return await fetch(url, { ...options, signal });
    } finally {
      clearTimeout(timeoutId);
    }
  }

  private combineSignals(
    signal1: AbortSignal,
    signal2: AbortSignal
  ): AbortSignal {
    const controller = new AbortController();

    const abort = () => controller.abort();

    signal1.addEventListener('abort', abort);
    signal2.addEventListener('abort', abort);

    return controller.signal;
  }

  private sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }
}

export class GLM5APIError extends Error {
  constructor(
    message: string,
    public status: number,
    public body: string
  ) {
    super(message);
    this.name = 'GLM5APIError';
  }
}
