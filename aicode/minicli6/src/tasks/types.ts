export type TaskStatus = 'pending' | 'running' | 'completed' | 'failed' | 'cancelled';

export type TaskPriority = 'high' | 'medium' | 'low';

export interface TaskDefinition {
  id: string;
  name: string;
  description?: string;
  priority: TaskPriority;
  
  prompt: string;
  tools?: string[];
  
  timeout?: number;
  maxRetries?: number;
  retryDelay?: number;
  
  dependencies?: string[];
}

export interface TaskResult {
  taskId: string;
  status: TaskStatus;
  output?: string;
  error?: string;
  startTime: Date;
  endTime?: Date;
  duration?: number;
  retries: number;
}

export interface TaskProgress {
  taskId: string;
  status: TaskStatus;
  progress: number;
  message?: string;
}
