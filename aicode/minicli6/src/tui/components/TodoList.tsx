import React from 'react';
import { Box, Text } from 'ink';
import { TodoItem, TodoStatus, TodoPriority } from '../../types/todo.js';

interface TodoListProps {
  items: TodoItem[];
}

const statusIcons: Record<TodoStatus, string> = {
  pending: '○',
  in_progress: '◐',
  completed: '●',
  cancelled: '✕',
};

const statusColors: Record<TodoStatus, string> = {
  pending: 'gray',
  in_progress: 'yellow',
  completed: 'green',
  cancelled: 'red',
};

const priorityColors: Record<TodoPriority, string> = {
  high: 'red',
  medium: 'yellow',
  low: 'gray',
};

const priorityLabels: Record<TodoPriority, string> = {
  high: '!',
  medium: '-',
  low: '·',
};

export const TodoList: React.FC<TodoListProps> = ({ items }) => {
  const activeItems = items.filter(item => item.status !== 'cancelled');
  
  if (activeItems.length === 0) {
    return null;
  }

  const status = {
    total: items.length,
    completed: items.filter(i => i.status === 'completed').length,
    inProgress: items.filter(i => i.status === 'in_progress').length,
    pending: items.filter(i => i.status === 'pending').length,
  };

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="cyan" paddingX={1} marginY={1}>
      <Box>
        <Text bold color="cyan">Todo List</Text>
        <Text> </Text>
        <Text dimColor>
          ({status.completed}/{status.total} completed, {status.inProgress} in progress)
        </Text>
      </Box>
      {activeItems.map(item => (
        <Box key={item.id}>
          <Text color={priorityColors[item.priority]}>
            {priorityLabels[item.priority]}
          </Text>
          <Text> </Text>
          <Text color={statusColors[item.status]}>
            {statusIcons[item.status]}
          </Text>
          <Text
            color={item.status === 'completed' ? 'gray' : 'white'}
            strikethrough={item.status === 'completed'}
          >
            {' '}
            {item.content}
          </Text>
        </Box>
      ))}
    </Box>
  );
};
