import React, { useState, useCallback } from 'react';
import { Box, Text, useInput } from 'ink';
import { ToolCall } from '../../types/index.js';
import { theme } from '../theme';

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

const truncateArgs = (args: unknown, maxLength: number = 150): string => {
  const str = JSON.stringify(args, null, 0);
  if (str.length <= maxLength) return str;
  return str.substring(0, maxLength) + '...';
};

export const ToolConfirm: React.FC<ToolConfirmProps> = React.memo(({
  toolCall,
  toolDescription,
  onDecision,
}) => {
  const [selected, setSelected] = useState(0);

  const handleUp = useCallback(() => {
    setSelected((prev) => (prev > 0 ? prev - 1 : OPTIONS.length - 1));
  }, []);

  const handleDown = useCallback(() => {
    setSelected((prev) => (prev < OPTIONS.length - 1 ? prev + 1 : 0));
  }, []);

  const handleEnter = useCallback(() => {
    const option = OPTIONS[selected];
    onDecision(option.allowed, option.scope);
  }, [selected, onDecision]);

  const handleEscape = useCallback(() => {
    onDecision(false, 'once');
  }, [onDecision]);

  const handleNumber = useCallback((num: number) => {
    if (num >= 1 && num <= OPTIONS.length) {
      const option = OPTIONS[num - 1];
      onDecision(option.allowed, option.scope);
    }
  }, [onDecision]);

  useInput((input, key) => {
    if (key.upArrow) {
      handleUp();
    } else if (key.downArrow) {
      handleDown();
    } else if (key.return) {
      handleEnter();
    } else if (key.escape) {
      handleEscape();
    } else {
      const num = parseInt(input, 10);
      if (!isNaN(num)) {
        handleNumber(num);
      }
    }
  });

  return (
    <Box
      flexDirection="column"
      borderStyle="round"
      borderColor={theme.colors.warning}
      paddingX={1}
    >
      <Box>
        <Text bold color={theme.colors.warning}>
          {theme.icons.warning} TOOL PERMISSION REQUEST
        </Text>
      </Box>

      <Box>
        <Text dimColor color={theme.colors.dim}>
          {theme.borders.thinHorizontal.repeat(55)}
        </Text>
      </Box>

      <Box>
        <Box width={12}>
          <Text bold color={theme.colors.dim}>
            Tool:
          </Text>
        </Box>
        <Text bold color={theme.colors.accent}>
          {toolCall.name}
        </Text>
      </Box>

      <Box>
        <Box width={12}>
          <Text bold color={theme.colors.dim}>
            Action:
          </Text>
        </Box>
        <Text dimColor>{toolDescription}</Text>
      </Box>

      <Box flexDirection="column" marginTop={1}>
        <Text bold color={theme.colors.dim}>
          Arguments:
        </Text>
        <Box marginLeft={2}>
          <Text dimColor>{truncateArgs(toolCall.arguments)}</Text>
        </Box>
      </Box>

      <Box marginTop={1}>
        <Text dimColor color={theme.colors.dim}>
          {theme.borders.thinHorizontal.repeat(55)}
        </Text>
      </Box>

      <Box flexDirection="column">
        <Text bold color={theme.colors.text}>
          Options:
        </Text>
        {OPTIONS.map((option, index) => {
          const isSelected = index === selected;
          const color = option.allowed ? theme.colors.success : theme.colors.error;

          return (
            <Box key={option.key} marginLeft={1}>
              <Text
                color={isSelected ? theme.colors.primary : theme.colors.dim}
                bold={isSelected}
              >
                {isSelected ? theme.icons.arrow + ' ' : '  '}
              </Text>
              <Text color={theme.colors.dim}>[{option.key}]</Text>
              <Text> </Text>
              <Text color={isSelected ? color : theme.colors.text}>{option.label}</Text>
            </Box>
          );
        })}
      </Box>

      <Box marginTop={1}>
        <Text dimColor color={theme.colors.dim}>
          {theme.icons.info} Use arrows or [1-6] to select, Enter to confirm, Esc to deny
        </Text>
      </Box>
    </Box>
  );
});

ToolConfirm.displayName = 'ToolConfirm';
