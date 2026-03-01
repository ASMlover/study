import { TaskDefinition, TaskResult, TaskProgress } from './types.js';
import { TaskRunner } from './TaskRunner.js';
import type { MiniCLIConfig } from '../core/Config.js';

export type ScheduleType = 'immediate' | 'delayed' | 'cron';

export interface ScheduleConfig {
  type: ScheduleType;
  delay?: number;
  cron?: string;
  runAt?: Date;
}

export interface ScheduledTask {
  id: string;
  task: TaskDefinition;
  schedule: ScheduleConfig;
  nextRun?: Date;
  status: 'scheduled' | 'running' | 'completed' | 'cancelled';
}

export interface SchedulerConfig {
  maxQueueSize?: number;
  autoStart?: boolean;
}

export class TaskScheduler {
  private runner: TaskRunner;
  private queue: ScheduledTask[] = [];
  private completed: Map<string, TaskResult> = new Map();
  private timers: Map<string, NodeJS.Timeout> = new Map();
  private running: boolean = false;
  private schedulerConfig: Required<SchedulerConfig>;
  private idCounter: number = 0;

  constructor(cliConfig: MiniCLIConfig, schedulerConfig: SchedulerConfig = {}) {
    this.schedulerConfig = {
      maxQueueSize: schedulerConfig.maxQueueSize ?? 100,
      autoStart: schedulerConfig.autoStart ?? true,
    };
    this.runner = new TaskRunner(cliConfig);
    
    if (this.schedulerConfig.autoStart) {
      this.start();
    }
  }

  schedule(task: TaskDefinition, schedule?: ScheduleConfig): string {
    const scheduleConfig: ScheduleConfig = schedule || { type: 'immediate' };
    const scheduledTaskId = this.generateId();

    const scheduledTask: ScheduledTask = {
      id: scheduledTaskId,
      task,
      schedule: scheduleConfig,
      nextRun: this.calculateNextRun(scheduleConfig),
      status: 'scheduled',
    };

    if (this.queue.length >= this.schedulerConfig.maxQueueSize) {
      throw new Error('Task queue is full');
    }

    this.queue.push(scheduledTask);
    this.setupTimer(scheduledTask);

    return scheduledTaskId;
  }

  async executeNow(task: TaskDefinition): Promise<TaskResult> {
    return this.runner.run(task);
  }

  executeAfter(task: TaskDefinition, delay: number): string {
    return this.schedule(task, { type: 'delayed', delay });
  }

  executeAt(task: TaskDefinition, date: Date): string {
    return this.schedule(task, { type: 'delayed', runAt: date });
  }

  cancel(taskId: string): boolean {
    const timer = this.timers.get(taskId);
    if (timer) {
      clearTimeout(timer);
      this.timers.delete(taskId);
    }

    const index = this.queue.findIndex(st => st.task.id === taskId || st.id === taskId);
    if (index !== -1) {
      this.queue[index].status = 'cancelled';
      this.queue.splice(index, 1);
      return true;
    }

    return this.runner.cancel(taskId);
  }

  getQueue(): ScheduledTask[] {
    return [...this.queue];
  }

  getCompleted(): TaskResult[] {
    return Array.from(this.completed.values());
  }

  getProgress(taskId: string): TaskProgress | null {
    return this.runner.getStatus(taskId);
  }

  start(): void {
    this.running = true;
    this.processQueue();
  }

  stop(): void {
    this.running = false;
    this.clearAllTimers();
  }

  private setupTimer(scheduledTask: ScheduledTask): void {
    const { id, schedule, task } = scheduledTask;

    if (schedule.type === 'immediate') {
      this.processImmediate(scheduledTask);
      return;
    }

    const delay = this.calculateDelay(schedule);
    if (delay <= 0) {
      this.processImmediate(scheduledTask);
      return;
    }

    const timer = setTimeout(() => {
      this.timers.delete(id);
      this.processImmediate(scheduledTask);
    }, delay);

    this.timers.set(id, timer);
  }

  private calculateDelay(schedule: ScheduleConfig): number {
    if (schedule.type === 'delayed') {
      if (schedule.runAt) {
        return schedule.runAt.getTime() - Date.now();
      }
      return schedule.delay || 0;
    }
    return 0;
  }

  private calculateNextRun(schedule: ScheduleConfig): Date | undefined {
    if (schedule.type === 'immediate') {
      return new Date();
    }
    if (schedule.type === 'delayed') {
      if (schedule.runAt) {
        return schedule.runAt;
      }
      return new Date(Date.now() + (schedule.delay || 0));
    }
    return undefined;
  }

  private async processImmediate(scheduledTask: ScheduledTask): Promise<void> {
    if (!this.running) return;
    if (scheduledTask.status === 'cancelled') return;

    scheduledTask.status = 'running';

    try {
      const result = await this.runner.run(scheduledTask.task);
      scheduledTask.status = 'completed';
      this.completed.set(scheduledTask.id, result);
      this.removeFromQueue(scheduledTask.id);
    } catch (error) {
      scheduledTask.status = 'cancelled';
      const now = new Date();
      this.completed.set(scheduledTask.id, {
        taskId: scheduledTask.task.id,
        status: 'failed',
        error: error instanceof Error ? error.message : String(error),
        startTime: now,
        endTime: now,
        duration: 0,
        retries: 0,
      });
      this.removeFromQueue(scheduledTask.id);
    }
  }

  private processQueue(): void {
    for (const task of this.queue) {
      if (task.status === 'scheduled' && !this.timers.has(task.id)) {
        this.setupTimer(task);
      }
    }
  }

  private removeFromQueue(taskId: string): void {
    const index = this.queue.findIndex(t => t.id === taskId);
    if (index !== -1) {
      this.queue.splice(index, 1);
    }
  }

  private clearAllTimers(): void {
    for (const timer of this.timers.values()) {
      clearTimeout(timer);
    }
    this.timers.clear();
  }

  private generateId(): string {
    return `sched-${Date.now()}-${++this.idCounter}`;
  }

  destroy(): void {
    this.stop();
    this.queue = [];
    this.completed.clear();
  }
}
