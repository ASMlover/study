import { TaskDefinition, TaskResult } from './types.js';
import { TaskRunner } from './TaskRunner.js';
import type { MiniCLIConfig } from '../core/Config.js';

export type OrchestrationStrategy =
  | 'sequential'
  | 'parallel'
  | 'dag';

export interface OrchestrationOptions {
  strategy: OrchestrationStrategy;
  continueOnError?: boolean;
  maxParallel?: number;
  timeout?: number;

  onTaskStart?: (taskId: string) => void;
  onTaskComplete?: (taskId: string, result: TaskResult) => void;
  onAllComplete?: (results: Map<string, TaskResult>) => void;
}

export interface Workflow {
  id: string;
  name: string;
  tasks: TaskDefinition[];
  options: OrchestrationOptions;

  dependencies?: Map<string, string[]>;
}

export class TaskOrchestrator {
  private runner: TaskRunner;
  private config: MiniCLIConfig;

  constructor(config: MiniCLIConfig) {
    this.config = config;
    this.runner = new TaskRunner(config);
  }

  async execute(workflow: Workflow): Promise<Map<string, TaskResult>> {
    const { strategy } = workflow.options;

    if (!this.validateWorkflow(workflow)) {
      throw new Error('Invalid workflow: duplicate task IDs detected');
    }

    if (strategy === 'dag' && workflow.dependencies) {
      if (!this.validateDag(workflow.tasks, workflow.dependencies)) {
        throw new Error('Invalid DAG: cycle detected or missing task dependency');
      }
    }

    let results: Map<string, TaskResult>;

    const timeout = workflow.options.timeout;
    const timeoutPromise = timeout
      ? new Promise<never>((_, reject) =>
          setTimeout(() => reject(new Error('Workflow timeout exceeded')), timeout)
        )
      : null;

    const executePromise = (async (): Promise<Map<string, TaskResult>> => {
      switch (strategy) {
        case 'sequential':
          return this.executeSequential(workflow.tasks, workflow.options);
        case 'parallel':
          return this.executeParallel(workflow.tasks, workflow.options);
        case 'dag':
          return this.executeDag(
            workflow.tasks,
            workflow.dependencies || new Map(),
            workflow.options
          );
        default:
          throw new Error(`Unknown strategy: ${strategy}`);
      }
    })();

    results = timeoutPromise
      ? await Promise.race([executePromise, timeoutPromise])
      : await executePromise;

    workflow.options.onAllComplete?.(results);
    return results;
  }

  private async executeSequential(
    tasks: TaskDefinition[],
    options: OrchestrationOptions
  ): Promise<Map<string, TaskResult>> {
    const results = new Map<string, TaskResult>();

    for (const task of tasks) {
      options.onTaskStart?.(task.id);

      try {
        const result = await this.runner.run(task);
        results.set(task.id, result);
        options.onTaskComplete?.(task.id, result);

        if (result.status === 'failed' && !options.continueOnError) {
          this.cancelRemainingTasks(tasks, results);
          break;
        }
      } catch (error) {
        const failedResult: TaskResult = {
          taskId: task.id,
          status: 'failed',
          error: error instanceof Error ? error.message : String(error),
          startTime: new Date(),
          endTime: new Date(),
          duration: 0,
          retries: 0,
        };
        results.set(task.id, failedResult);
        options.onTaskComplete?.(task.id, failedResult);

        if (!options.continueOnError) {
          break;
        }
      }
    }

    return results;
  }

  private async executeParallel(
    tasks: TaskDefinition[],
    options: OrchestrationOptions
  ): Promise<Map<string, TaskResult>> {
    const results = new Map<string, TaskResult>();
    const maxParallel = options.maxParallel ?? tasks.length;

    const batches: TaskDefinition[][] = [];
    for (let i = 0; i < tasks.length; i += maxParallel) {
      batches.push(tasks.slice(i, i + maxParallel));
    }

    for (const batch of batches) {
      const promises = batch.map(async (task) => {
        options.onTaskStart?.(task.id);

        try {
          const result = await this.runner.run(task);
          options.onTaskComplete?.(task.id, result);
          return result;
        } catch (error) {
          const failedResult: TaskResult = {
            taskId: task.id,
            status: 'failed',
            error: error instanceof Error ? error.message : String(error),
            startTime: new Date(),
            endTime: new Date(),
            duration: 0,
            retries: 0,
          };
          options.onTaskComplete?.(task.id, failedResult);
          return failedResult;
        }
      });

      const batchResults = await Promise.all(promises);

      for (const result of batchResults) {
        results.set(result.taskId, result);
      }

      const hasFailure = batchResults.some((r) => r.status === 'failed');
      if (hasFailure && !options.continueOnError) {
        break;
      }
    }

    return results;
  }

  private async executeDag(
    tasks: TaskDefinition[],
    dependencies: Map<string, string[]>,
    options: OrchestrationOptions
  ): Promise<Map<string, TaskResult>> {
    const results = new Map<string, TaskResult>();
    const taskMap = new Map(tasks.map((t) => [t.id, t]));
    const completed = new Set<string>();
    const pending = new Set(tasks.map((t) => t.id));
    const running = new Set<string>();

    const maxParallel = options.maxParallel ?? tasks.length;

    const canRun = (taskId: string): boolean => {
      const deps = dependencies.get(taskId) || [];
      return deps.every((dep) => completed.has(dep));
    };

    const hasDependencyFailed = (taskId: string): boolean => {
      const deps = dependencies.get(taskId) || [];
      return deps.some((dep) => {
        const result = results.get(dep);
        return result && result.status === 'failed';
      });
    };

    while (pending.size > 0 || running.size > 0) {
      const readyTasks: TaskDefinition[] = [];

      for (const taskId of pending) {
        if (canRun(taskId) && !hasDependencyFailed(taskId)) {
          readyTasks.push(taskMap.get(taskId)!);
        } else if (hasDependencyFailed(taskId)) {
          const skippedResult: TaskResult = {
            taskId,
            status: 'cancelled',
            error: 'Dependency failed',
            startTime: new Date(),
            endTime: new Date(),
            duration: 0,
            retries: 0,
          };
          results.set(taskId, skippedResult);
          options.onTaskComplete?.(taskId, skippedResult);
          pending.delete(taskId);
        }
      }

      const toRun = readyTasks.slice(0, maxParallel - running.size);
      if (toRun.length === 0 && running.size === 0) {
        break;
      }

      const runPromises = toRun.map(async (task) => {
        pending.delete(task.id);
        running.add(task.id);
        options.onTaskStart?.(task.id);

        try {
          const result = await this.runner.run(task);
          results.set(task.id, result);
          options.onTaskComplete?.(task.id, result);
          return result;
        } catch (error) {
          const failedResult: TaskResult = {
            taskId: task.id,
            status: 'failed',
            error: error instanceof Error ? error.message : String(error),
            startTime: new Date(),
            endTime: new Date(),
            duration: 0,
            retries: 0,
          };
          results.set(task.id, failedResult);
          options.onTaskComplete?.(task.id, failedResult);
          return failedResult;
        } finally {
          running.delete(task.id);
        }
      });

      if (runPromises.length > 0) {
        const runResults = await Promise.all(runPromises);

        for (const result of runResults) {
          if (result.status === 'completed') {
            completed.add(result.taskId);
          } else if (result.status === 'failed' && !options.continueOnError) {
            for (const taskId of pending) {
              const skippedResult: TaskResult = {
                taskId,
                status: 'cancelled',
                error: 'Workflow stopped due to failure',
                startTime: new Date(),
                endTime: new Date(),
                duration: 0,
                retries: 0,
              };
              results.set(taskId, skippedResult);
            }
            pending.clear();
            break;
          }
        }
      } else {
        await this.delay(100);
      }
    }

    return results;
  }

  createWorkflow(
    name: string,
    tasks: TaskDefinition[],
    strategy: OrchestrationStrategy
  ): Workflow {
    return {
      id: `workflow-${Date.now()}`,
      name,
      tasks,
      options: { strategy },
    };
  }

  validateDag(tasks: TaskDefinition[], dependencies: Map<string, string[]>): boolean {
    const taskIds = new Set(tasks.map((t) => t.id));

    for (const [taskId, deps] of dependencies) {
      if (!taskIds.has(taskId)) {
        return false;
      }
      for (const dep of deps) {
        if (!taskIds.has(dep)) {
          return false;
        }
      }
    }

    return !this.hasCycle(tasks, dependencies);
  }

  private validateWorkflow(workflow: Workflow): boolean {
    const taskIds = new Set<string>();
    for (const task of workflow.tasks) {
      if (taskIds.has(task.id)) {
        return false;
      }
      taskIds.add(task.id);
    }
    return true;
  }

  private hasCycle(tasks: TaskDefinition[], dependencies: Map<string, string[]>): boolean {
    const WHITE = 0;
    const GRAY = 1;
    const BLACK = 2;

    const color = new Map<string, number>();
    const taskIds = tasks.map((t) => t.id);

    for (const id of taskIds) {
      color.set(id, WHITE);
    }

    const dfs = (taskId: string): boolean => {
      color.set(taskId, GRAY);

      const deps = dependencies.get(taskId) || [];
      for (const dep of deps) {
        const depColor = color.get(dep);

        if (depColor === GRAY) {
          return true;
        }

        if (depColor === WHITE && dfs(dep)) {
          return true;
        }
      }

      color.set(taskId, BLACK);
      return false;
    };

    for (const id of taskIds) {
      if (color.get(id) === WHITE) {
        if (dfs(id)) {
          return true;
        }
      }
    }

    return false;
  }

  private cancelRemainingTasks(
    tasks: TaskDefinition[],
    completedResults: Map<string, TaskResult>
  ): void {
    for (const task of tasks) {
      if (!completedResults.has(task.id)) {
        const cancelledResult: TaskResult = {
          taskId: task.id,
          status: 'cancelled',
          error: 'Skipped due to previous failure',
          startTime: new Date(),
          endTime: new Date(),
          duration: 0,
          retries: 0,
        };
        completedResults.set(task.id, cancelledResult);
      }
    }
  }

  private delay(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }
}
