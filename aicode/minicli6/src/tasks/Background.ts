import { TaskDefinition, TaskResult, TaskProgress } from './types.js';
import { TaskRunner } from './TaskRunner.js';
import { logger } from '../utils/logger.js';
import { randomUUID } from 'crypto';

export interface BackgroundTask {
  id: string;
  definition: TaskDefinition;
  status: TaskProgress;
  result?: TaskResult;
  startTime: Date;
  outputBuffer: string[];
}

export interface BackgroundTaskOptions {
  maxConcurrent: number;
  pollInterval: number;
  onTaskStart?: (task: BackgroundTask) => void;
  onTaskProgress?: (task: BackgroundTask) => void;
  onTaskComplete?: (task: BackgroundTask) => void;
  onTaskError?: (task: BackgroundTask, error: Error) => void;
}

const DEFAULT_OPTIONS: BackgroundTaskOptions = {
  maxConcurrent: 3,
  pollInterval: 100,
};

export class BackgroundTaskManager {
  private runner: TaskRunner;
  private options: BackgroundTaskOptions;
  private tasks: Map<string, BackgroundTask> = new Map();
  private running: number = 0;
  private queue: string[] = [];
  private completionResolvers: Map<string, (result: TaskResult) => void> = new Map();
  private allCompletionResolvers: Array<() => void> = [];

  constructor(config: any, options?: Partial<BackgroundTaskOptions>) {
    this.options = { ...DEFAULT_OPTIONS, ...options };
    
    this.runner = new TaskRunner(config, {
      onProgress: (progress: TaskProgress) => {
        this.handleProgress(progress);
      },
      onComplete: (result: TaskResult) => {
        this.handleComplete(result);
      },
    });
  }

  start(taskInput: Partial<TaskDefinition> & { prompt: string }): string {
    const taskId = taskInput.id || randomUUID();
    
    const definition: TaskDefinition = {
      id: taskId,
      name: taskInput.name || `Background Task ${taskId.slice(0, 8)}`,
      description: taskInput.description,
      priority: taskInput.priority || 'medium',
      prompt: taskInput.prompt,
      tools: taskInput.tools,
      timeout: taskInput.timeout,
      maxRetries: taskInput.maxRetries,
      retryDelay: taskInput.retryDelay,
      dependencies: taskInput.dependencies,
    };

    const task: BackgroundTask = {
      id: taskId,
      definition,
      status: {
        taskId,
        status: 'pending',
        progress: 0,
        message: 'Queued',
      },
      startTime: new Date(),
      outputBuffer: [],
    };

    this.tasks.set(taskId, task);
    this.queue.push(taskId);
    
    logger.debug(`Background task ${taskId} queued`);
    
    this.processQueue();
    
    return taskId;
  }

  private async processQueue(): Promise<void> {
    while (this.queue.length > 0 && this.running < this.options.maxConcurrent) {
      const taskId = this.queue.shift();
      if (!taskId) break;

      const task = this.tasks.get(taskId);
      if (!task) continue;

      this.running++;
      task.status.status = 'running';
      task.status.message = 'Starting...';
      
      this.options.onTaskStart?.(task);
      
      this.runTask(task);
    }
  }

  private async runTask(task: BackgroundTask): Promise<void> {
    try {
      const result = await this.runner.run(task.definition);
      task.result = result;
      task.status.status = result.status;
      task.status.progress = 100;
      
      if (result.output) {
        task.outputBuffer.push(result.output);
      }
      
      if (result.status === 'completed') {
        this.options.onTaskComplete?.(task);
      } else if (result.status === 'failed') {
        this.options.onTaskError?.(task, new Error(result.error || 'Task failed'));
      }
    } catch (error) {
      task.status.status = 'failed';
      task.status.message = error instanceof Error ? error.message : String(error);
      task.result = {
        taskId: task.id,
        status: 'failed',
        error: task.status.message,
        startTime: task.startTime,
        endTime: new Date(),
        retries: 0,
      };
      
      this.options.onTaskError?.(task, error instanceof Error ? error : new Error(String(error)));
    } finally {
      this.running--;
      
      const resolver = this.completionResolvers.get(task.id);
      if (resolver && task.result) {
        resolver(task.result);
        this.completionResolvers.delete(task.id);
      }
      
      this.checkAllComplete();
      
      this.processQueue();
    }
  }

  private handleProgress(progress: TaskProgress): void {
    const task = this.tasks.get(progress.taskId);
    if (task) {
      task.status = progress;
      this.options.onTaskProgress?.(task);
    }
  }

  private handleComplete(result: TaskResult): void {
    const task = this.tasks.get(result.taskId);
    if (task) {
      task.result = result;
      if (result.output) {
        task.outputBuffer.push(result.output);
      }
    }
  }

  getStatus(taskId: string): BackgroundTask | null {
    return this.tasks.get(taskId) || null;
  }

  getAll(): BackgroundTask[] {
    return Array.from(this.tasks.values());
  }

  async await(taskId: string): Promise<TaskResult> {
    const task = this.tasks.get(taskId);
    if (!task) {
      return {
        taskId,
        status: 'failed',
        error: 'Task not found',
        startTime: new Date(),
        retries: 0,
      };
    }

    if (task.result) {
      return task.result;
    }

    return new Promise((resolve) => {
      this.completionResolvers.set(taskId, resolve);
    });
  }

  async awaitAll(): Promise<TaskResult[]> {
    const pendingTasks = Array.from(this.tasks.values()).filter(t => !t.result);
    
    if (pendingTasks.length === 0) {
      return Array.from(this.tasks.values()).map(t => t.result!);
    }

    return new Promise((resolve) => {
      this.allCompletionResolvers.push(() => {
        if (this.getPendingCount() === 0) {
          resolve(Array.from(this.tasks.values()).map(t => t.result!));
        }
      });
    });
  }

  cancel(taskId: string): boolean {
    const task = this.tasks.get(taskId);
    if (!task) return false;

    if (task.status.status === 'pending') {
      const queueIndex = this.queue.indexOf(taskId);
      if (queueIndex !== -1) {
        this.queue.splice(queueIndex, 1);
      }
      task.status.status = 'cancelled';
      task.status.message = 'Cancelled by user';
      task.result = {
        taskId,
        status: 'cancelled',
        startTime: task.startTime,
        endTime: new Date(),
        retries: 0,
      };
      return true;
    }

    if (task.status.status === 'running') {
      const cancelled = this.runner.cancel(taskId);
      if (cancelled) {
        task.status.status = 'cancelled';
        task.status.message = 'Cancelled by user';
        task.result = {
          taskId,
          status: 'cancelled',
          startTime: task.startTime,
          endTime: new Date(),
          retries: 0,
        };
      }
      return cancelled;
    }

    return false;
  }

  getOutput(taskId: string): string[] {
    const task = this.tasks.get(taskId);
    return task ? [...task.outputBuffer] : [];
  }

  cleanup(): void {
    const completedStatuses: TaskProgress['status'][] = ['completed', 'failed', 'cancelled'];
    
    for (const [id, task] of this.tasks) {
      if (completedStatuses.includes(task.status.status)) {
        this.tasks.delete(id);
      }
    }
    
    logger.debug('Cleaned up completed background tasks');
  }

  getStats(): {
    total: number;
    running: number;
    pending: number;
    completed: number;
    failed: number;
  } {
    let running = 0;
    let pending = 0;
    let completed = 0;
    let failed = 0;

    for (const task of this.tasks.values()) {
      switch (task.status.status) {
        case 'running':
          running++;
          break;
        case 'pending':
          pending++;
          break;
        case 'completed':
          completed++;
          break;
        case 'failed':
        case 'cancelled':
          failed++;
          break;
      }
    }

    return {
      total: this.tasks.size,
      running,
      pending,
      completed,
      failed,
    };
  }

  private getPendingCount(): number {
    let count = 0;
    for (const task of this.tasks.values()) {
      if (!task.result) count++;
    }
    return count;
  }

  private checkAllComplete(): void {
    if (this.getPendingCount() === 0) {
      for (const resolver of this.allCompletionResolvers) {
        resolver();
      }
      this.allCompletionResolvers = [];
    }
  }
}
