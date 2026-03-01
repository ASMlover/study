import React from 'react';
import { Box, Text } from 'ink';
import chalk from 'chalk';

interface StatusBarProps {
  status: string;
  tokens?: number;
  model?: string;
}

export const StatusBar: React.FC<StatusBarProps> = ({ status, tokens, model }) => {
  const getStatusColor = (status: string): string => {
    switch (status) {
      case 'thinking':
        return 'yellow';
      case 'tool_use':
        return 'blue';
      case 'waiting_input':
        return 'green';
      default:
        return 'gray';
    }
  };

  return (
    <Box borderStyle="single" borderColor="gray" paddingX={1}>
      <Box flexGrow={1}>
        <Text>
          <Text bold>Status: </Text>
          <Text color={getStatusColor(status)}>{status}</Text>
        </Text>
      </Box>
      {model && (
        <Box marginRight={2}>
          <Text dimColor>Model: {model}</Text>
        </Box>
      )}
      {tokens !== undefined && (
        <Box>
          <Text dimColor>Tokens: {tokens.toLocaleString()}</Text>
        </Box>
      )}
    </Box>
  );
};
