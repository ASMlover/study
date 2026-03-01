import { GLM5Provider, ChatMessage, ToolCall as ProviderToolCall, StreamDelta, ToolDefinition } from '../core/Provider.js';
import { ContextManager } from '../core/Context.js';
import type { MiniCLIConfig } from '../core/Config.js';
import { ToolExecutor, ToolResult, PermissionDecision } from './Executor.js';
import { ToolCall } from '../tools/Tool.js';
import { logger } from '../utils/logger.js';

export interface AgentOptions {
  config: MiniCLIConfig;
  onMessage?: (msg: ChatMessage) => void;
  onStatusChange?: (status: AgentStatus) => void;
  onToolCall?: (tool: ToolCall) => Promise<PermissionDecision>;
}

export type AgentStatus = 'idle' | 'thinking' | 'tool_use' | 'waiting_input';

export type AgentEvent =
  | { type: 'text'; content: string }
  | { type: 'tool_call'; tool: ProviderToolCall }
  | { type: 'tool_result'; result: ToolResult }
  | { type: 'done' }
  | { type: 'error'; error: Error };

export class Agent {
  private provider: GLM5Provider;
  private context: ContextManager;
  private config: MiniCLIConfig;
  private status: AgentStatus = 'idle';
  private executor: ToolExecutor;
  private onMessage?: (msg: ChatMessage) => void;
  private onStatusChange?: (status: AgentStatus) => void;
  private onToolCall?: (tool: ToolCall) => Promise<PermissionDecision>;
  private abortController: AbortController | null = null;
  private tools: ToolDefinition[] = [];
  private initialized: boolean = false;

  constructor(options: AgentOptions) {
    this.config = options.config;
    this.onMessage = options.onMessage;
    this.onStatusChange = options.onStatusChange;
    this.onToolCall = options.onToolCall;

    this.provider = new GLM5Provider({
      apiKey: this.config.provider.apiKey,
      baseUrl: this.config.provider.baseUrl,
      model: this.config.provider.model,
    });

    this.context = new ContextManager({
      maxTokens: this.config.context.maxTokens,
    });

    this.executor = new ToolExecutor(
      this.config,
      process.cwd(),
      crypto.randomUUID()
    );
    
    if (options.onToolCall) {
      this.executor.onPermissionRequest = async (toolCall, _toolName) => {
        return options.onToolCall!(toolCall);
      };
    }
  }

  async initialize(): Promise<void> {
    if (this.initialized) return;
    
    await this.executor.initialize();
    
    this.tools = this.executor.getToolSchemas().map(schema => ({
      type: 'function' as const,
      function: {
        name: schema.name,
        description: schema.description,
        parameters: schema.parameters,
      },
    }));
    
    this.initialized = true;
  }

  setStatus(status: AgentStatus): void {
    this.status = status;
    this.onStatusChange?.(status);
  }

  getStatus(): AgentStatus {
    return this.status;
  }

  setSystemPrompt(prompt: string): void {
    this.context.setSystemPrompt(prompt);
  }

  registerTool(tool: ToolDefinition): void {
    this.tools.push(tool);
  }
  
  getToolSchemas(): ToolDefinition[] {
    return this.tools;
  }

  async *sendMessage(content: string): AsyncGenerator<AgentEvent> {
    if (!this.initialized) {
      await this.initialize();
    }
    
    this.abortController = new AbortController();

    this.context.addUserMessage(content);
    this.setStatus('thinking');

    try {
      await this.context.compactIfNeeded(this.provider);
    } catch (error) {
      logger.warn('Context compaction failed, continuing without compression:', error as Error);
    }

    let iteration = 0;
    const maxIterations = this.config.agent.maxIterations;

    try {
      while (iteration < maxIterations) {
        iteration++;

        const messages = this.context.getMessages();
        let currentContent = '';
        const toolCallsAccumulator: ProviderToolCall[] = [];

        try {
          const response = await this.provider.chat(
            messages,
            this.tools,
            (delta: StreamDelta) => {
              if (delta.content) {
                currentContent += delta.content;
              }
              if (delta.tool_calls) {
                for (const tc of delta.tool_calls) {
                  if (tc.id && tc.function?.name) {
                    const existing = toolCallsAccumulator.find(t => t.id === tc.id);
                    if (existing) {
                      existing.function.arguments += tc.function.arguments || '';
                    } else {
                      toolCallsAccumulator.push(tc as ProviderToolCall);
                    }
                  } else if (tc.function?.arguments && toolCallsAccumulator.length > 0) {
                    const last = toolCallsAccumulator[toolCallsAccumulator.length - 1];
                    last.function.arguments += tc.function.arguments;
                  }
                }
              }
            },
            { signal: this.abortController.signal }
          );

          if (currentContent) {
            yield { type: 'text', content: currentContent };
          }

          this.context.addAssistantMessage(response.content, response.tool_calls);

          if (response.tool_calls && response.tool_calls.length > 0) {
            this.setStatus('tool_use');

            for (const providerToolCall of response.tool_calls) {
              yield { type: 'tool_call', tool: providerToolCall };

              const toolCall: ToolCall = {
                id: providerToolCall.id,
                name: providerToolCall.function.name,
                arguments: this.parseArgs(providerToolCall.function.arguments),
              };

              const result = await this.executor.execute(toolCall);
              yield { type: 'tool_result', result };

              this.context.addToolMessage(providerToolCall.id, result.output);
            }

            this.setStatus('thinking');
          } else {
            break;
          }
        } catch (error) {
          if (error instanceof Error && error.name === 'AbortError') {
            yield { type: 'done' };
            break;
          }
          throw error;
        }
      }

      if (iteration >= maxIterations) {
        logger.warn(`Reached max iterations: ${maxIterations}`);
      }

      yield { type: 'done' };
    } catch (error) {
      yield { type: 'error', error: error as Error };
    } finally {
      this.setStatus('idle');
      this.abortController = null;
    }
  }

  private parseArgs(argsString: string): Record<string, unknown> {
    try {
      return JSON.parse(argsString);
    } catch {
      return {};
    }
  }

  stop(): void {
    if (this.abortController) {
      this.abortController.abort();
    }
  }

  reset(): void {
    this.context.clear();
    this.setStatus('idle');
  }

  getContext(): ContextManager {
    return this.context;
  }
}
