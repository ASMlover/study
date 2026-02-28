import type { Task, TaskStatus } from "../types.js";

let nextId = 1;
const tasks = new Map<string, Task>();

export function createTask(opts: {
  subject: string;
  description: string;
  owner?: string;
  blockedBy?: string[];
  metadata?: Record<string, unknown>;
}): Task {
  const id = String(nextId++);
  const task: Task = {
    id,
    subject: opts.subject,
    description: opts.description,
    status: "pending",
    owner: opts.owner,
    blockedBy: opts.blockedBy ?? [],
    blocks: [],
    metadata: opts.metadata ?? {},
    createdAt: Date.now(),
    updatedAt: Date.now(),
  };

  // Update reverse dependencies
  for (const depId of task.blockedBy) {
    const dep = tasks.get(depId);
    if (dep && !dep.blocks.includes(id)) {
      dep.blocks.push(id);
    }
  }

  tasks.set(id, task);
  return task;
}

export function getTask(id: string): Task | undefined {
  return tasks.get(id);
}

export function updateTask(id: string, updates: {
  status?: TaskStatus;
  subject?: string;
  description?: string;
  owner?: string;
  addBlockedBy?: string[];
  addBlocks?: string[];
  metadata?: Record<string, unknown>;
}): Task | undefined {
  const task = tasks.get(id);
  if (!task) return undefined;

  if (updates.status) task.status = updates.status;
  if (updates.subject) task.subject = updates.subject;
  if (updates.description) task.description = updates.description;
  if (updates.owner !== undefined) task.owner = updates.owner;

  if (updates.addBlockedBy) {
    for (const depId of updates.addBlockedBy) {
      if (!task.blockedBy.includes(depId)) {
        task.blockedBy.push(depId);
        const dep = tasks.get(depId);
        if (dep && !dep.blocks.includes(id)) {
          dep.blocks.push(id);
        }
      }
    }
  }

  if (updates.addBlocks) {
    for (const blockId of updates.addBlocks) {
      if (!task.blocks.includes(blockId)) {
        task.blocks.push(blockId);
        const blocked = tasks.get(blockId);
        if (blocked && !blocked.blockedBy.includes(id)) {
          blocked.blockedBy.push(id);
        }
      }
    }
  }

  if (updates.metadata) {
    for (const [k, v] of Object.entries(updates.metadata)) {
      if (v === null) {
        delete task.metadata[k];
      } else {
        task.metadata[k] = v;
      }
    }
  }

  task.updatedAt = Date.now();
  return task;
}

export function deleteTask(id: string): boolean {
  const task = tasks.get(id);
  if (!task) return false;

  // Remove from dependency lists
  for (const depId of task.blockedBy) {
    const dep = tasks.get(depId);
    if (dep) dep.blocks = dep.blocks.filter(b => b !== id);
  }
  for (const blockId of task.blocks) {
    const blocked = tasks.get(blockId);
    if (blocked) blocked.blockedBy = blocked.blockedBy.filter(b => b !== id);
  }

  tasks.delete(id);
  return true;
}

export function listTasks(): Task[] {
  return [...tasks.values()].filter(t => t.status !== "deleted");
}

export function isBlocked(task: Task): boolean {
  return task.blockedBy.some(depId => {
    const dep = tasks.get(depId);
    return dep && dep.status !== "completed" && dep.status !== "deleted";
  });
}

export function loadTasks(taskList: Task[]): void {
  tasks.clear();
  nextId = 1;
  for (const t of taskList) {
    tasks.set(t.id, t);
    const num = parseInt(t.id, 10);
    if (num >= nextId) nextId = num + 1;
  }
}

export function exportTasks(): Task[] {
  return [...tasks.values()];
}
