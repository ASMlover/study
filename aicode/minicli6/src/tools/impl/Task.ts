import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { SubAgent, SubAgentResult } from '../../agent/SubAgent.js';

export class TaskTool extends Tool {
  readonly name = 'task';
  readonly description = 'Launch a sub-agent to handle a specific task. Use this when you need to delegate complex tasks that require multiple steps or focused exploration.';
  readonly parameters = {
    type: 'object' as const,
    properties: {
      description: {
        type: 'string' as const,
        description: 'Short description of the task (used for progress display)',
      },
      prompt: {
        type: 'string' as const,
        description: 'Detailed task description for the sub-agent to execute',
      },
      subagent_type: {
        type: 'string' as const,
        enum: ['general', 'explore'],
        description: 'Type of sub-agent: "general" for full capabilities, "explore" for read-only code exploration',
      },
    },
    required: ['description', 'prompt'],
  };
  permission: 'confirm' | 'auto' | 'deny' = 'confirm';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { description, prompt, subagent_type } = call.arguments;

    if (!description || !prompt) {
      return {
        success: false,
        output: '',
        error: 'Missing required parameters: description and prompt are required',
      };
    }

    const subAgentType = subagent_type || 'general';

    try {
      const subAgent = new SubAgent(context.config, {
        task: prompt,
        maxIterations: 10,
        timeout: 120000,
        subagentType: subAgentType,
      });

      const result = await subAgent.execute();

      return this.formatResult(result, description, subAgentType);
    } catch (error) {
      return {
        success: false,
        output: '',
        error: `Sub-agent execution failed: ${error instanceof Error ? error.message : String(error)}`,
      };
    }
  }

  private formatResult(result: SubAgentResult, description: string, subagentType: string): ToolResult {
    const statusEmoji = result.success ? '✓' : '✗';
    const header = `${statusEmoji} Sub-agent (${subagentType}): ${description}`;

    const stats = [
      `Duration: ${result.duration}ms`,
      `Iterations: ${result.iterations}`,
    ].join(' | ');

    const output = [
      header,
      stats,
      '─'.repeat(40),
      result.output || '(No output)',
    ].join('\n');

    return {
      success: result.success,
      output,
      metadata: {
        iterations: result.iterations,
        duration: result.duration,
        subagentType,
      },
    };
  }
}
