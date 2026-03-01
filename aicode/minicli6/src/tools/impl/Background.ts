import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { BackgroundTaskManager, BackgroundTask } from '../../tasks/Background.js';

let managerInstance: BackgroundTaskManager | null = null;

function getManager(context: ToolContext): BackgroundTaskManager {
  if (!managerInstance) {
    managerInstance = new BackgroundTaskManager(context.config, {
      maxConcurrent: 3,
      pollInterval: 100,
    });
  }
  return managerInstance;
}

export class BackgroundTool extends Tool {
  readonly name = 'background';
  readonly description = 'Run tasks in the background and manage them. Use this for long-running operations that can continue while you work on other things.';
  readonly parameters = {
    type: 'object' as const,
    properties: {
      action: {
        type: 'string' as const,
        enum: ['start', 'status', 'list', 'cancel', 'output', 'stats'],
        description: 'Action to perform: start (run new task), status (check task), list (all tasks), cancel (stop task), output (get task output), stats (get statistics)',
      },
      taskId: {
        type: 'string' as const,
        description: 'Task ID (required for status/cancel/output actions)',
      },
      prompt: {
        type: 'string' as const,
        description: 'Task prompt for the sub-agent (required for start action)',
      },
      name: {
        type: 'string' as const,
        description: 'Optional name for the task (for start action)',
      },
    },
    required: ['action'],
  };
  permission: 'confirm' | 'auto' | 'deny' = 'confirm';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { action, taskId, prompt, name } = call.arguments;
    const manager = getManager(context);

    switch (action) {
      case 'start':
        return this.handleStart(manager, prompt, name);
      case 'status':
        return this.handleStatus(manager, taskId);
      case 'list':
        return this.handleList(manager);
      case 'cancel':
        return this.handleCancel(manager, taskId);
      case 'output':
        return this.handleOutput(manager, taskId);
      case 'stats':
        return this.handleStats(manager);
      default:
        return {
          success: false,
          output: '',
          error: `Unknown action: ${action}`,
        };
    }
  }

  private handleStart(manager: BackgroundTaskManager, prompt?: string, name?: string): ToolResult {
    if (!prompt) {
      return {
        success: false,
        output: '',
        error: 'prompt is required for start action',
      };
    }

    const taskId = manager.start({
      prompt,
      name: name || 'Background Task',
    });

    return {
      success: true,
      output: `Background task started with ID: ${taskId}\nUse 'background' tool with action 'status' and taskId '${taskId}' to check progress.`,
      metadata: { taskId },
    };
  }

  private handleStatus(manager: BackgroundTaskManager, taskId?: string): ToolResult {
    if (!taskId) {
      return {
        success: false,
        output: '',
        error: 'taskId is required for status action',
      };
    }

    const task = manager.getStatus(taskId);
    if (!task) {
      return {
        success: false,
        output: '',
        error: `Task not found: ${taskId}`,
      };
    }

    const lines = [
      `Task: ${task.definition.name}`,
      `ID: ${task.id}`,
      `Status: ${task.status.status}`,
      `Progress: ${task.status.progress}%`,
      `Started: ${task.startTime.toISOString()}`,
    ];

    if (task.status.message) {
      lines.push(`Message: ${task.status.message}`);
    }

    if (task.result) {
      lines.push('---');
      lines.push(`Completed: ${task.result.endTime?.toISOString() || 'N/A'}`);
      lines.push(`Duration: ${task.result.duration || 0}ms`);
      if (task.result.error) {
        lines.push(`Error: ${task.result.error}`);
      }
    }

    return {
      success: true,
      output: lines.join('\n'),
      metadata: {
        taskId: task.id,
        status: task.status.status,
        progress: task.status.progress,
      },
    };
  }

  private handleList(manager: BackgroundTaskManager): ToolResult {
    const tasks = manager.getAll();
    const stats = manager.getStats();

    if (tasks.length === 0) {
      return {
        success: true,
        output: 'No background tasks.',
      };
    }

    const lines = [
      `Background Tasks (${stats.running} running, ${stats.pending} pending, ${stats.completed} completed, ${stats.failed} failed)`,
      '',
    ];

    for (const task of tasks) {
      const statusIcon = this.getStatusIcon(task.status.status);
      const progress = task.status.progress > 0 ? ` (${task.status.progress}%)` : '';
      lines.push(`  ${statusIcon} ${task.id.slice(0, 8)}... ${task.definition.name}${progress}`);
      if (task.status.message) {
        lines.push(`      ${task.status.message}`);
      }
    }

    return {
      success: true,
      output: lines.join('\n'),
      metadata: { stats },
    };
  }

  private handleCancel(manager: BackgroundTaskManager, taskId?: string): ToolResult {
    if (!taskId) {
      return {
        success: false,
        output: '',
        error: 'taskId is required for cancel action',
      };
    }

    const cancelled = manager.cancel(taskId);
    
    return {
      success: cancelled,
      output: cancelled 
        ? `Task ${taskId} cancelled successfully`
        : `Failed to cancel task ${taskId} (not found or already completed)`,
    };
  }

  private handleOutput(manager: BackgroundTaskManager, taskId?: string): ToolResult {
    if (!taskId) {
      return {
        success: false,
        output: '',
        error: 'taskId is required for output action',
      };
    }

    const task = manager.getStatus(taskId);
    if (!task) {
      return {
        success: false,
        output: '',
        error: `Task not found: ${taskId}`,
      };
    }

    const output = manager.getOutput(taskId);
    
    if (output.length === 0) {
      return {
        success: true,
        output: `No output available yet for task ${taskId}`,
        metadata: { taskId, status: task.status.status },
      };
    }

    return {
      success: true,
      output: output.join('\n\n---\n\n'),
      metadata: { taskId, status: task.status.status },
    };
  }

  private handleStats(manager: BackgroundTaskManager): ToolResult {
    const stats = manager.getStats();
    
    const lines = [
      'Background Task Statistics:',
      `  Total: ${stats.total}`,
      `  Running: ${stats.running}`,
      `  Pending: ${stats.pending}`,
      `  Completed: ${stats.completed}`,
      `  Failed: ${stats.failed}`,
    ];

    return {
      success: true,
      output: lines.join('\n'),
      metadata: { stats },
    };
  }

  private getStatusIcon(status: string): string {
    switch (status) {
      case 'pending': return '○';
      case 'running': return '◐';
      case 'completed': return '●';
      case 'failed': return '✗';
      case 'cancelled': return '⊘';
      default: return '?';
    }
  }
}
