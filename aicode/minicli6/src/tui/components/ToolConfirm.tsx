import React, { useState, useEffect } from 'react';
import { Box, Text, useInput } from 'ink';
import { ToolCall } from '../../types/index.js';

interface ToolConfirmProps {
  toolCall: ToolCall;
  toolDescription: string;
  onDecision: (allowed: boolean, scope: 'once' | 'session' | 'permanent') => void;
}

const OPTIONS = [
  { key: '1', label: 'Allow once', allowed: true, scope: 'once' as const },
  { key: '2', label: 'Allow for session', allowed: true, scope: 'session' as const },
  { key: '3', label: 'Always allow', allowed: true, scope: 'permanent' as const },
  { key: '4', label: 'Deny', allowed: false, scope: 'once' as const },
  { key: '5', label: 'Deny for session', allowed: false, scope: 'session' as const },
  { key: '6', label: 'Always deny', allowed: false, scope: 'permanent' as const },
];

export const ToolConfirm: React.FC<ToolConfirmProps> = ({
  toolCall,
  toolDescription,
  onDecision,
}) => {
  const [selected, setSelected] = useState(0);

  useInput((input, key) => {
    if (key.upArrow) {
      setSelected((prev) => (prev > 0 ? prev - 1 : OPTIONS.length - 1));
    } else if (key.downArrow) {
      setSelected((prev) => (prev < OPTIONS.length - 1 ? prev + 1 : 0));
    } else if (key.return) {
      const option = OPTIONS[selected];
      onDecision(option.allowed, option.scope);
    } else if (key.escape) {
      onDecision(false, 'once');
    } else {
      const num = parseInt(input, 10);
      if (num >= 1 && num <= OPTIONS.length) {
        const option = OPTIONS[num - 1];
        onDecision(option.allowed, option.scope);
      }
    }
  });

  return (
    <Box flexDirection="column" borderStyle="round" borderColor="yellow" paddingX={1}>
      <Text bold color="yellow">
        Tool Permission Request
      </Text>
      <Box marginTop={1}>
        <Text bold>Tool: </Text>
        <Text color="cyan">{toolCall.name}</Text>
      </Box>
      <Box>
        <Text bold>Description: </Text>
        <Text dimColor>{toolDescription}</Text>
      </Box>
      <Box flexDirection="column" marginTop={1}>
        <Text bold>Arguments:</Text>
        <Box marginLeft={2}>
          <Text dimColor>{JSON.stringify(toolCall.arguments, null, 2)}</Text>
        </Box>
      </Box>
      <Box flexDirection="column" marginTop={1}>
        <Text bold>Options:</Text>
        {OPTIONS.map((option, index) => (
          <Box key={option.key} marginLeft={2}>
            <Text
              color={index === selected ? 'green' : undefined}
              bold={index === selected}
            >
              {index === selected ? '> ' : '  '}
              [{option.key}] {option.label}
            </Text>
          </Box>
        ))}
      </Box>
      <Box marginTop={1}>
        <Text dimColor>
          Use arrow keys to navigate, Enter to confirm, Esc to deny
        </Text>
      </Box>
    </Box>
  );
};
