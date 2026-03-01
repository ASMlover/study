import React from 'react';
import { Box, Text } from 'ink';
import { theme, getStatusColor } from '../theme';

interface StatusBarProps {
  status: string;
  tokens?: number;
  model?: string;
}

export const StatusBar: React.FC<StatusBarProps> = React.memo(({ status, tokens, model }) => {
  const statusColor = getStatusColor(status);

  const statusLabels: Record<string, string> = {
    idle: 'READY',
    thinking: 'THINKING',
    tool_use: 'TOOL USE',
    waiting_input: 'INPUT',
  };

  const statusIcons: Record<string, string> = {
    idle: theme.icons.completed,
    thinking: theme.icons.inProgress,
    tool_use: theme.icons.tool,
    waiting_input: theme.icons.pending,
  };

  return (
    <Box flexDirection="column">
      <Box>
        <Text dimColor color={theme.colors.dim}>
          {theme.borders.thinHorizontal.repeat(75)}
        </Text>
      </Box>
      <Box>
        <Box flexGrow={1}>
          <Box marginRight={2}>
            <Text color={statusColor}>
              {statusIcons[status] || theme.icons.info} {statusLabels[status] || status.toUpperCase()}
            </Text>
          </Box>
          {model && (
            <Box marginRight={2}>
              <Text dimColor color={theme.colors.dim}>
                {theme.icons.diamond} {model}
              </Text>
            </Box>
          )}
        </Box>
        {tokens !== undefined && (
          <Box>
            <Text dimColor color={theme.colors.dim}>
              {theme.icons.sparkle} {tokens.toLocaleString()} tokens
            </Text>
          </Box>
        )}
      </Box>
    </Box>
  );
});

StatusBar.displayName = 'StatusBar';
