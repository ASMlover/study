import { TaskDefinition, TaskResult, TaskProgress } from './types.js';
import { SubAgent, SubAgentResult } from '../agent/SubAgent.js';
import type { MiniCLIConfig } from '../core/Config.js';
import { AgentEvent } from '../agent/Agent.js';

export interface TaskRunnerOptions {
  onProgress?: (progress: TaskProgress) => void;
  onComplete?: (result: TaskResult) => void;
}

export class TaskRunner {
  private config: MiniCLIConfig;
  private options: TaskRunnerOptions;
  private activeTasks: Map<string, SubAgent> = new Map();
  private taskProgress: Map<string, TaskProgress> = new Map();
  private abortControllers: Map<string, AbortController> = new Map();

  constructor(config: MiniCLIConfig, options?: TaskRunnerOptions) {
    this.config = config;
    this.options = options || {};
  }

  async run(task: TaskDefinition): Promise<TaskResult> {
    const startTime = new Date();
    let retries = 0;
    const maxRetries = task.maxRetries ?? 0;
    const retryDelay = task.retryDelay ?? 1000;

    this.updateProgress(task.id, 'running', 0, 'Starting task');

    while (retries <= maxRetries) {
      try {
        const result = await this.executeTask(task, startTime, retries);
        
        if (result.status === 'completed') {
          this.options.onComplete?.(result);
          return result;
        }

        if (result.status === 'cancelled') {
          return result;
        }

        if (retries < maxRetries) {
          retries++;
          this.updateProgress(task.id, 'running', 0, `Retrying (${retries}/${maxRetries})`);
          await this.delay(retryDelay * retries);
        } else {
          return result;
        }
      } catch (error) {
        if (retries < maxRetries) {
          retries++;
          this.updateProgress(task.id, 'running', 0, `Retrying (${retries}/${maxRetries})`);
          await this.delay(retryDelay * retries);
        } else {
          const endTime = new Date();
          const result: TaskResult = {
            taskId: task.id,
            status: 'failed',
            error: error instanceof Error ? error.message : String(error),
            startTime,
            endTime,
            duration: endTime.getTime() - startTime.getTime(),
            retries,
          };
          this.updateProgress(task.id, 'failed', 0, result.error);
          this.options.onComplete?.(result);
          return result;
        }
      }
    }

    const endTime = new Date();
    const failedResult: TaskResult = {
      taskId: task.id,
      status: 'failed',
      error: 'Max retries exceeded',
      startTime,
      endTime,
      duration: endTime.getTime() - startTime.getTime(),
      retries,
    };
    this.options.onComplete?.(failedResult);
    return failedResult;
  }

  private async executeTask(
    task: TaskDefinition,
    startTime: Date,
    currentRetry: number
  ): Promise<TaskResult> {
    const abortController = new AbortController();
    this.abortControllers.set(task.id, abortController);

    const subAgent = new SubAgent(this.config, {
      task: task.prompt,
      timeout: task.timeout ?? this.config.agent.timeout,
      onProgress: (event: AgentEvent) => {
        this.handleAgentEvent(task.id, event);
        if (abortController.signal.aborted) {
          throw new Error('Task cancelled');
        }
      },
    });

    this.activeTasks.set(task.id, subAgent);

    try {
      const subResult: SubAgentResult = await subAgent.execute();
      const endTime = new Date();

      const result: TaskResult = {
        taskId: task.id,
        status: subResult.success ? 'completed' : 'failed',
        output: subResult.output,
        error: subResult.success ? undefined : subResult.output,
        startTime,
        endTime,
        duration: endTime.getTime() - startTime.getTime(),
        retries: currentRetry,
      };

      this.updateProgress(
        task.id,
        result.status,
        100,
        subResult.success ? 'Task completed' : 'Task failed'
      );

      return result;
    } finally {
      this.activeTasks.delete(task.id);
      this.abortControllers.delete(task.id);
    }
  }

  private handleAgentEvent(taskId: string, event: AgentEvent): void {
    switch (event.type) {
      case 'tool_call':
        this.updateProgress(taskId, 'running', 50, `Executing tool: ${event.tool.function.name}`);
        break;
      case 'text':
        this.updateProgress(taskId, 'running', 75, 'Processing response');
        break;
      case 'done':
        this.updateProgress(taskId, 'completed', 100, 'Task completed');
        break;
      case 'error':
        this.updateProgress(taskId, 'failed', 0, event.error?.message || 'Error occurred');
        break;
      case 'tool_result':
        this.updateProgress(taskId, 'running', 80, 'Tool execution completed');
        break;
    }
  }

  private updateProgress(
    taskId: string,
    status: TaskProgress['status'],
    progress: number,
    message?: string
  ): void {
    const taskProgress: TaskProgress = {
      taskId,
      status,
      progress,
      message,
    };

    this.taskProgress.set(taskId, taskProgress);
    this.options.onProgress?.(taskProgress);
  }

  cancel(taskId: string): boolean {
    const abortController = this.abortControllers.get(taskId);
    if (abortController) {
      abortController.abort();
      this.updateProgress(taskId, 'cancelled', 0, 'Task cancelled by user');
      this.activeTasks.delete(taskId);
      return true;
    }
    return false;
  }

  getStatus(taskId: string): TaskProgress | null {
    return this.taskProgress.get(taskId) || null;
  }

  isRunning(taskId: string): boolean {
    return this.activeTasks.has(taskId);
  }

  private delay(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }
}
