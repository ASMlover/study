import { Tool, ToolCall, ToolResult, ToolContext } from '../Tool.js';
import { TodoItem, TodoListManager, TodoInput } from '../../types/todo.js';

const globalTodoStore = new Map<string, TodoListManager>();

export function getSessionTodoManager(sessionId: string): TodoListManager {
  if (!globalTodoStore.has(sessionId)) {
    globalTodoStore.set(sessionId, new TodoListManager());
  }
  return globalTodoStore.get(sessionId)!;
}

export function clearSessionTodo(sessionId: string): void {
  globalTodoStore.delete(sessionId);
}

interface TodoInputArg {
  content: string;
  status: 'pending' | 'in_progress' | 'completed' | 'cancelled';
  priority: 'high' | 'medium' | 'low';
  id?: string;
}

export class TodoWriteTool extends Tool {
  name = 'todowrite';
  description = `Manage a todo list for tracking task progress. Use this tool to:
- Create a new todo list with tasks
- Update existing tasks status
- Track completion progress

Tasks should be specific and actionable. Update task status as you work on them.`;

  parameters = {
    type: 'object' as const,
    properties: {
      todos: {
        type: 'array' as const,
        description: 'List of todo items to set. This will replace the entire todo list.',
        items: {
          type: 'object' as const,
          description: 'A todo item',
          properties: {
            content: {
              type: 'string' as const,
              description: 'The content/description of the todo item',
            },
            status: {
              type: 'string' as const,
              enum: ['pending', 'in_progress', 'completed', 'cancelled'],
              description: 'Current status of the todo item',
            },
            priority: {
              type: 'string' as const,
              enum: ['high', 'medium', 'low'],
              description: 'Priority level of the todo item',
            },
          },
          required: ['content', 'status', 'priority'],
        },
      },
    },
    required: ['todos'],
  };

  permission: 'auto' = 'auto';

  async execute(call: ToolCall, context: ToolContext): Promise<ToolResult> {
    const { todos } = call.arguments as { todos: TodoInputArg[] };
    const sessionId = context.sessionId;
    const manager = getSessionTodoManager(sessionId);

    if (!Array.isArray(todos)) {
      return {
        success: false,
        output: '',
        error: 'todos must be an array',
      };
    }

    const newItems: TodoItem[] = todos.map((todo, index) => {
      const now = new Date();
      return {
        id: todo.id || `todo-${Date.now()}-${index}-${Math.random().toString(36).substr(2, 9)}`,
        content: todo.content,
        status: todo.status as TodoInput['status'],
        priority: todo.priority as TodoInput['priority'],
        createdAt: now,
        updatedAt: now,
      };
    });

    manager.setItems(newItems);
    const status = manager.getStatus();

    const statusText = `Todo list updated: ${status.completed}/${status.total} completed, ${status.inProgress} in progress, ${status.pending} pending`;

    return {
      success: true,
      output: statusText,
      metadata: {
        todos: manager.getItems(),
        status,
      },
    };
  }
}
