import { Agent, AgentEvent } from './Agent.js';
import { Tool } from '../tools/Tool.js';
import type { MiniCLIConfig } from '../core/Config.js';
import { logger } from '../utils/logger.js';

export interface SubAgentOptions {
  task: string;
  tools?: Tool[];
  maxIterations?: number;
  timeout?: number;
  onProgress?: (event: AgentEvent) => void;
  subagentType?: 'general' | 'explore';
}

export interface SubAgentResult {
  success: boolean;
  output: string;
  iterations: number;
  duration: number;
}

export class SubAgent {
  private agent: Agent;
  private task: string;
  private options: SubAgentOptions;
  private result: SubAgentResult | null = null;
  private startTime: number = 0;

  constructor(parentConfig: MiniCLIConfig, options: SubAgentOptions) {
    this.task = options.task;
    this.options = options;

    const subConfig: MiniCLIConfig = {
      ...parentConfig,
      agent: {
        ...parentConfig.agent,
        maxIterations: options.maxIterations ?? 10,
        timeout: options.timeout ?? 60000,
      },
    };

    this.agent = new Agent({
      config: subConfig,
      onToolCall: async () => ({ allowed: true, scope: 'session' }),
    });

    const systemPrompt = this.buildSystemPrompt(options.subagentType ?? 'general');
    this.agent.setSystemPrompt(systemPrompt);
  }

  private buildSystemPrompt(type: 'general' | 'explore'): string {
    const basePrompt = `You are a sub-agent tasked with completing a specific task.
Focus on completing the task efficiently and return a clear summary of your findings or actions.
Be concise in your responses.`;

    if (type === 'explore') {
      return `${basePrompt}

You are in "explore" mode - use only read-only tools to investigate and understand the codebase.
Do NOT make any modifications. Focus on finding relevant information and summarizing your discoveries.`;
    }

    return basePrompt;
  }

  async execute(): Promise<SubAgentResult> {
    this.startTime = Date.now();
    let iterations = 0;
    let output = '';
    let success = false;
    let error: Error | null = null;

    const timeoutMs = this.options.timeout ?? 60000;
    const timeoutPromise = new Promise<never>((_, reject) => {
      setTimeout(() => reject(new Error('Sub-agent timeout')), timeoutMs);
    });

    try {
      const executionPromise = this.runAgent();
      await Promise.race([executionPromise, timeoutPromise]);
    } catch (err) {
      error = err as Error;
      logger.error(`Sub-agent error: ${error.message}`);
    }

    this.result = {
      success,
      output,
      iterations,
      duration: Date.now() - this.startTime,
    };

    if (error) {
      this.result.output = `Error: ${error.message}`;
      this.result.success = false;
    }

    return this.result;
  }

  private async runAgent(): Promise<void> {
    const generator = this.agent.sendMessage(this.task);

    let iterations = 0;

    try {
      for await (const event of generator) {
        if (this.options.onProgress) {
          this.options.onProgress(event);
        }

        switch (event.type) {
          case 'tool_call':
            iterations++;
            break;
          case 'text':
            if (this.result) {
              this.result.output += event.content;
            }
            break;
          case 'done':
            if (this.result) {
              this.result.success = true;
            }
            return;
          case 'error':
            throw event.error;
        }
      }
    } finally {
      if (this.result) {
        this.result.iterations = iterations;
      }
    }
  }

  async *sendMessage(content: string): AsyncGenerator<AgentEvent> {
    yield* this.agent.sendMessage(content);
  }

  getResult(): SubAgentResult | null {
    return this.result;
  }
}
