import type { ChatMessage } from './Provider.js';
import { logger } from '../utils/logger.js';

export interface CompactOptions {
  maxTokens: number;
  threshold: number;
  preserveRecent: number;
  preserveSystem: boolean;
}

export interface CompactResult {
  messages: ChatMessage[];
  summary: string;
  compressedTokens: number;
  originalTokens: number;
}

interface TokenEstimator {
  estimate(text: string): number;
}

class SimpleTokenEstimator implements TokenEstimator {
  estimate(text: string): number {
    const chineseChars = (text.match(/[\u4e00-\u9fa5]/g) || []).length;
    const otherChars = text.length - chineseChars;
    return Math.ceil(chineseChars / 2 + otherChars / 4);
  }
}

export class ContextCompactor {
  private options: CompactOptions;
  private tokenEstimator: TokenEstimator;

  constructor(options: Partial<CompactOptions> = {}) {
    this.options = {
      maxTokens: options.maxTokens ?? 128000,
      threshold: options.threshold ?? 100000,
      preserveRecent: options.preserveRecent ?? 10,
      preserveSystem: options.preserveSystem ?? true,
    };
    this.tokenEstimator = new SimpleTokenEstimator();
  }

  needsCompact(messages: ChatMessage[]): boolean {
    return this.estimateTokens(messages) >= this.options.threshold;
  }

  async compact(
    messages: ChatMessage[],
    provider: { chatSync: (msgs: ChatMessage[], opts?: { signal?: AbortSignal }) => Promise<string> }
  ): Promise<CompactResult> {
    const originalTokens = this.estimateTokens(messages);

    if (messages.length <= this.options.preserveRecent) {
      return {
        messages,
        summary: '',
        compressedTokens: originalTokens,
        originalTokens,
      };
    }

    const systemMessages: ChatMessage[] = [];
    const recentMessages: ChatMessage[] = [];
    const toCompact: ChatMessage[] = [];

    let recentStartIndex = messages.length - this.options.preserveRecent;

    if (this.options.preserveSystem) {
      for (let i = 0; i < messages.length; i++) {
        if (messages[i].role === 'system') {
          systemMessages.push(messages[i]);
          if (i < recentStartIndex) {
            recentStartIndex--;
          }
        }
      }
    }

    for (let i = 0; i < messages.length; i++) {
      const msg = messages[i];
      if (this.options.preserveSystem && msg.role === 'system') {
        continue;
      }
      if (i >= recentStartIndex) {
        recentMessages.push(msg);
      } else {
        toCompact.push(msg);
      }
    }

    const summary = await this.generateSummary(toCompact, provider);
    const summaryMessage: ChatMessage = {
      role: 'assistant',
      content: `[Context Summary]\n${summary}`,
    };

    const compactedMessages: ChatMessage[] = [
      ...systemMessages,
      summaryMessage,
      ...recentMessages,
    ];

    const compressedTokens = this.estimateTokens(compactedMessages);

    logger.info(
      `Context compacted: ${originalTokens} -> ${compressedTokens} tokens (${Math.round((1 - compressedTokens / originalTokens) * 100)}% reduction)`
    );

    return {
      messages: compactedMessages,
      summary,
      compressedTokens,
      originalTokens,
    };
  }

  estimateTokens(messages: ChatMessage[]): number {
    let total = 0;

    for (const msg of messages) {
      total += 4;

      if (msg.content) {
        total += this.tokenEstimator.estimate(msg.content);
      }

      if (msg.tool_calls) {
        for (const tc of msg.tool_calls) {
          total += this.tokenEstimator.estimate(tc.function.name);
          total += this.tokenEstimator.estimate(tc.function.arguments);
        }
      }
    }

    return total;
  }

  private async generateSummary(
    messages: ChatMessage[],
    provider: { chatSync: (msgs: ChatMessage[], opts?: { signal?: AbortSignal }) => Promise<string> }
  ): Promise<string> {
    if (messages.length === 0) {
      return '';
    }

    const contextText = this.formatMessagesForSummary(messages);

    const summaryPrompt: ChatMessage[] = [
      {
        role: 'system',
        content: `You are a context summarizer. Your task is to create a concise summary of the conversation history.
Focus on:
1. Key decisions made
2. Important information discovered
3. Tasks completed
4. Current state and pending work

Keep the summary brief and informative. Use bullet points if helpful.`,
      },
      {
        role: 'user',
        content: `Please summarize the following conversation history:\n\n${contextText}`,
      },
    ];

    try {
      const summary = await provider.chatSync(summaryPrompt);
      return summary || this.fallbackSummary(messages);
    } catch (error) {
      logger.warn('Failed to generate AI summary, using fallback:', error as Error);
      return this.fallbackSummary(messages);
    }
  }

  private formatMessagesForSummary(messages: ChatMessage[]): string {
    const parts: string[] = [];

    for (const msg of messages) {
      switch (msg.role) {
        case 'system':
          parts.push(`[System]: ${this.truncate(msg.content, 300)}`);
          break;
        case 'user':
          parts.push(`[User]: ${this.truncate(msg.content, 500)}`);
          break;
        case 'assistant':
          if (msg.tool_calls && msg.tool_calls.length > 0) {
            const toolNames = msg.tool_calls.map((tc) => tc.function.name).join(', ');
            parts.push(`[Assistant]: Used tools: ${toolNames}`);
            if (msg.content) {
              parts.push(`  Response: ${this.truncate(msg.content, 200)}`);
            }
          } else {
            parts.push(`[Assistant]: ${this.truncate(msg.content, 500)}`);
          }
          break;
        case 'tool':
          parts.push(`[Tool Result]: ${this.truncate(msg.content, 200)}`);
          break;
      }
    }

    return parts.join('\n');
  }

  private fallbackSummary(messages: ChatMessage[]): string {
    const userCount = messages.filter((m) => m.role === 'user').length;
    const assistantCount = messages.filter((m) => m.role === 'assistant').length;
    const toolCount = messages.filter((m) => m.role === 'tool').length;

    const toolNames = new Set<string>();
    for (const msg of messages) {
      if (msg.tool_calls) {
        for (const tc of msg.tool_calls) {
          toolNames.add(tc.function.name);
        }
      }
    }

    let summary = `Previous conversation: ${userCount} user messages, ${assistantCount} assistant responses, ${toolCount} tool calls.`;

    if (toolNames.size > 0) {
      summary += `\nTools used: ${Array.from(toolNames).join(', ')}.`;
    }

    return summary;
  }

  private truncate(text: string, maxLength: number): string {
    if (text.length <= maxLength) {
      return text;
    }
    return text.slice(0, maxLength - 3) + '...';
  }
}
