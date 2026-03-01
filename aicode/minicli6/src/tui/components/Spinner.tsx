import React from 'react';
import { Text } from 'ink';
import Spinner from 'ink-spinner';
import { theme } from '../theme';

interface SpinnerProps {
  text?: string;
  status?: 'thinking' | 'tool_use' | 'loading';
}

const STATUS_CONFIG = {
  thinking: {
    color: theme.colors.warning,
    icon: theme.icons.inProgress,
    defaultText: 'Processing...',
  },
  tool_use: {
    color: theme.colors.info,
    icon: theme.icons.tool,
    defaultText: 'Executing tool...',
  },
  loading: {
    color: theme.colors.primary,
    icon: theme.icons.diamond,
    defaultText: 'Loading...',
  },
} as const;

export const LoadingSpinner: React.FC<SpinnerProps> = React.memo(({
  text,
  status = 'thinking',
}) => {
  const config = STATUS_CONFIG[status];
  const displayText = text || config.defaultText;

  return (
    <Text>
      <Text color={config.color}>
        <Spinner type="dots" />
      </Text>
      <Text> </Text>
      <Text color={config.color}>{config.icon}</Text>
      <Text> </Text>
      <Text dimColor>{displayText}</Text>
    </Text>
  );
});

LoadingSpinner.displayName = 'LoadingSpinner';
