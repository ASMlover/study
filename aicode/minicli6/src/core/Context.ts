import type { ChatMessage } from './Provider.js';
import { ContextCompactor, CompactOptions, CompactResult } from './Compact.js';

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

export interface ContextOptions {
  maxTokens?: number;
  systemPrompt?: string;
  compactOptions?: Partial<CompactOptions>;
}

export class ContextManager {
  private messages: ChatMessage[] = [];
  private systemPrompt: string = '';
  private tokenEstimator: TokenEstimator = new SimpleTokenEstimator();
  private maxTokens: number;
  private compactor: ContextCompactor;

  constructor(options: ContextOptions = {}) {
    this.maxTokens = options.maxTokens ?? 128000;
    if (options.systemPrompt) {
      this.systemPrompt = options.systemPrompt;
    }
    this.compactor = new ContextCompactor({
      maxTokens: this.maxTokens,
      ...options.compactOptions,
    });
  }

  addMessage(msg: ChatMessage): void {
    this.messages.push(msg);
  }

  addSystemMessage(content: string): void {
    this.messages.unshift({
      role: 'system',
      content,
    });
  }

  addUserMessage(content: string): void {
    this.addMessage({
      role: 'user',
      content,
    });
  }

  addAssistantMessage(
    content: string,
    toolCalls?: ChatMessage['tool_calls']
  ): void {
    this.addMessage({
      role: 'assistant',
      content,
      tool_calls: toolCalls,
    });
  }

  addToolMessage(toolCallId: string, content: string): void {
    this.addMessage({
      role: 'tool',
      tool_call_id: toolCallId,
      content,
    });
  }

  getMessages(): ChatMessage[] {
    const result: ChatMessage[] = [];

    if (this.systemPrompt) {
      result.push({
        role: 'system',
        content: this.systemPrompt,
      });
    }

    result.push(...this.messages);
    return result;
  }

  setSystemPrompt(prompt: string): void {
    this.systemPrompt = prompt;
  }

  getSystemPrompt(): string {
    return this.systemPrompt;
  }

  clear(): void {
    this.messages = [];
  }

  getMessageCount(): number {
    return this.messages.length;
  }

  estimateTokens(): number {
    let total = 0;

    if (this.systemPrompt) {
      total += this.tokenEstimator.estimate(this.systemPrompt);
      total += 4;
    }

    for (const msg of this.messages) {
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

  estimateMessageTokens(msg: ChatMessage): number {
    let total = 4;

    if (msg.content) {
      total += this.tokenEstimator.estimate(msg.content);
    }

    if (msg.tool_calls) {
      for (const tc of msg.tool_calls) {
        total += this.tokenEstimator.estimate(tc.function.name);
        total += this.tokenEstimator.estimate(tc.function.arguments);
      }
    }

    return total;
  }

  needsCompaction(threshold: number): boolean {
    return this.estimateTokens() >= threshold;
  }

  compact(preserveRecent: number, summary?: string): void {
    if (this.messages.length <= preserveRecent) {
      return;
    }

    const recentMessages = this.messages.slice(-preserveRecent);
    const oldMessages = this.messages.slice(0, -preserveRecent);

    if (summary) {
      this.messages = [
        {
          role: 'system',
          content: `[Context Summary]\n${summary}`,
        },
        ...recentMessages,
      ];
    } else {
      const oldContext = this.summarizeMessages(oldMessages);
      this.messages = [
        {
          role: 'system',
          content: `[Previous Context]\n${oldContext}`,
        },
        ...recentMessages,
      ];
    }
  }

  private summarizeMessages(messages: ChatMessage[]): string {
    const parts: string[] = [];

    for (const msg of messages) {
      switch (msg.role) {
        case 'user':
          parts.push(`User: ${this.truncate(msg.content, 200)}`);
          break;
        case 'assistant':
          if (msg.tool_calls) {
            const toolNames = msg.tool_calls
              .map((tc) => tc.function.name)
              .join(', ');
            parts.push(`Assistant: Used tools [${toolNames}]`);
          } else {
            parts.push(`Assistant: ${this.truncate(msg.content, 200)}`);
          }
          break;
        case 'tool':
          parts.push(`Tool result: ${this.truncate(msg.content, 100)}`);
          break;
      }
    }

    return parts.join('\n');
  }

  private truncate(text: string, maxLength: number): string {
    if (text.length <= maxLength) {
      return text;
    }
    return text.slice(0, maxLength - 3) + '...';
  }

  setMaxTokens(max: number): void {
    this.maxTokens = max;
  }

  getMaxTokens(): number {
    return this.maxTokens;
  }

  async compactIfNeeded(
    provider: { chatSync: (msgs: ChatMessage[], opts?: { signal?: AbortSignal }) => Promise<string> }
  ): Promise<CompactResult | null> {
    const allMessages = this.getMessages();
    
    if (!this.compactor.needsCompact(allMessages)) {
      return null;
    }

    const result = await this.compactor.compact(allMessages, provider);
    
    if (this.systemPrompt) {
      const systemMsgIndex = result.messages.findIndex(m => m.role === 'system' && m.content === this.systemPrompt);
      if (systemMsgIndex >= 0) {
        result.messages.splice(systemMsgIndex, 1);
      }
      this.messages = result.messages;
    } else {
      this.messages = result.messages;
    }

    return result;
  }

  getCompactor(): ContextCompactor {
    return this.compactor;
  }
}
