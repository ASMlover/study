export type TodoStatus = 'pending' | 'in_progress' | 'completed' | 'cancelled';

export type TodoPriority = 'high' | 'medium' | 'low';

export interface TodoItem {
  id: string;
  content: string;
  status: TodoStatus;
  priority: TodoPriority;
  createdAt: Date;
  updatedAt: Date;
}

export interface TodoInput {
  content: string;
  status: TodoStatus;
  priority: TodoPriority;
}

export interface TodoStatusInfo {
  total: number;
  completed: number;
  inProgress: number;
  pending: number;
  cancelled: number;
}

export class TodoListManager {
  private items: TodoItem[] = [];

  add(input: TodoInput): TodoItem {
    const now = new Date();
    const item: TodoItem = {
      id: `todo-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`,
      content: input.content,
      status: input.status,
      priority: input.priority,
      createdAt: now,
      updatedAt: now,
    };
    this.items.push(item);
    this.sortItems();
    return item;
  }

  update(id: string, updates: Partial<Omit<TodoItem, 'id' | 'createdAt'>>): TodoItem | null {
    const index = this.items.findIndex(item => item.id === id);
    if (index === -1) {
      return null;
    }
    
    this.items[index] = {
      ...this.items[index],
      ...updates,
      updatedAt: new Date(),
    };
    
    this.sortItems();
    return this.items[index];
  }

  remove(id: string): boolean {
    const index = this.items.findIndex(item => item.id === id);
    if (index === -1) {
      return false;
    }
    this.items.splice(index, 1);
    return true;
  }

  clear(): void {
    this.items = [];
  }

  getItems(): TodoItem[] {
    return [...this.items];
  }

  setItems(items: TodoItem[]): void {
    this.items = items.map(item => ({
      ...item,
      createdAt: new Date(item.createdAt),
      updatedAt: new Date(item.updatedAt),
    }));
    this.sortItems();
  }

  getStatus(): TodoStatusInfo {
    return {
      total: this.items.length,
      completed: this.items.filter(i => i.status === 'completed').length,
      inProgress: this.items.filter(i => i.status === 'in_progress').length,
      pending: this.items.filter(i => i.status === 'pending').length,
      cancelled: this.items.filter(i => i.status === 'cancelled').length,
    };
  }

  private sortItems(): void {
    const priorityOrder: Record<TodoPriority, number> = {
      high: 0,
      medium: 1,
      low: 2,
    };
    
    const statusOrder: Record<TodoStatus, number> = {
      in_progress: 0,
      pending: 1,
      completed: 2,
      cancelled: 3,
    };

    this.items.sort((a, b) => {
      const statusDiff = statusOrder[a.status] - statusOrder[b.status];
      if (statusDiff !== 0) return statusDiff;
      return priorityOrder[a.priority] - priorityOrder[b.priority];
    });
  }
}
