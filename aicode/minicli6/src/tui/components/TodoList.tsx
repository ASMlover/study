import React, { useMemo } from 'react';
import { Box, Text } from 'ink';
import { TodoItem, TodoStatus, TodoPriority } from '../../types/todo.js';
import {
  theme,
  getTodoStatusColor,
  getStatusIcon,
  getPriorityColor,
  getPriorityIcon,
  ui,
} from '../theme';

interface TodoListProps {
  items: TodoItem[];
}

export const TodoList: React.FC<TodoListProps> = React.memo(({ items }) => {
  const activeItems = useMemo(
    () => items.filter((item) => item.status !== 'cancelled'),
    [items]
  );

  const stats = useMemo(() => ({
    total: items.length,
    completed: items.filter((i) => i.status === 'completed').length,
    inProgress: items.filter((i) => i.status === 'in_progress').length,
    pending: items.filter((i) => i.status === 'pending').length,
  }), [items]);

  const progressPercent = Math.round((stats.completed / stats.total) * 100) || 0;
  const progressBar = ui.progressBar(progressPercent, 20);

  if (activeItems.length === 0) {
    return null;
  }

  return (
    <Box
      flexDirection="column"
      borderStyle="round"
      borderColor={theme.colors.info}
      paddingX={1}
      marginY={0}
    >
      <Box>
        <Text bold color={theme.colors.info}>
          {theme.icons.diamond} TASKS
        </Text>
        <Text> </Text>
        <Text dimColor color={theme.colors.dim}>
          {progressBar}
        </Text>
        <Text> </Text>
        <Text bold color={theme.colors.accent}>
          {progressPercent}%
        </Text>
      </Box>

      <Box>
        <Text dimColor color={theme.colors.dim}>
          {theme.borders.thinHorizontal.repeat(50)}
        </Text>
      </Box>

      {activeItems.slice(0, 5).map((item) => {
        const statusColor = getTodoStatusColor(item.status);
        const statusIcon = getStatusIcon(item.status);
        const priorityColor = getPriorityColor(item.priority);
        const priorityIcon = getPriorityIcon(item.priority);

        return (
          <Box key={item.id}>
            <Box width={3}>
              <Text color={priorityColor}>{priorityIcon}</Text>
            </Box>
            <Box width={3}>
              <Text color={statusColor}>{statusIcon}</Text>
            </Box>
            <Text
              color={item.status === 'completed' ? theme.colors.dim : theme.colors.text}
              strikethrough={item.status === 'completed'}
            >
              {item.content.length > 50 ? item.content.substring(0, 50) + '...' : item.content}
            </Text>
          </Box>
        );
      })}

      {activeItems.length > 5 && (
        <Box>
          <Text dimColor color={theme.colors.dim}>
            {theme.icons.info} +{activeItems.length - 5} more tasks
          </Text>
        </Box>
      )}
    </Box>
  );
});

TodoList.displayName = 'TodoList';
