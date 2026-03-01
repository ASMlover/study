import React, { useState, useCallback } from 'react';
import { Box, Text } from 'ink';
import TextInput from 'ink-text-input';
import { theme } from '../theme';

interface InputProps {
  onSubmit: (text: string) => void;
  disabled?: boolean;
  placeholder?: string;
}

export const Input: React.FC<InputProps> = React.memo(({
  onSubmit,
  disabled = false,
  placeholder = 'Type a message...',
}) => {
  const [value, setValue] = useState('');

  const handleSubmit = useCallback(
    (text: string) => {
      if (text.trim() && !disabled) {
        onSubmit(text.trim());
        setValue('');
      }
    },
    [disabled, onSubmit]
  );

  const handleChange = useCallback((text: string) => {
    setValue(text);
  }, []);

  const borderColor = disabled ? theme.colors.dim : theme.colors.primary;

  return (
    <Box flexDirection="column">
      <Box
        borderStyle="round"
        borderColor={borderColor}
        paddingX={1}
      >
        <Box marginRight={1}>
          <Text bold color={borderColor}>
            {theme.icons.arrow}
          </Text>
        </Box>
        {disabled ? (
          <Text dimColor color={theme.colors.dim}>
            {placeholder}
          </Text>
        ) : (
          <TextInput
            value={value}
            onChange={handleChange}
            onSubmit={handleSubmit}
            placeholder={placeholder}
            showCursor={!disabled}
          />
        )}
      </Box>
      <Box paddingLeft={1}>
        <Text dimColor color={theme.colors.dim}>
          {theme.icons.info} /help for commands
        </Text>
      </Box>
    </Box>
  );
});

Input.displayName = 'Input';
