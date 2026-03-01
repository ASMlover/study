import React, { useState, useCallback } from 'react';
import { Box, Text } from 'ink';
import TextInput from 'ink-text-input';

interface InputProps {
  onSubmit: (text: string) => void;
  disabled?: boolean;
  placeholder?: string;
}

export const Input: React.FC<InputProps> = ({ 
  onSubmit, 
  disabled = false, 
  placeholder = 'Type a message...' 
}) => {
  const [value, setValue] = useState('');

  const handleSubmit = useCallback((text: string) => {
    if (text.trim() && !disabled) {
      onSubmit(text.trim());
      setValue('');
    }
  }, [disabled, onSubmit]);

  return (
    <Box borderStyle="single" borderColor="gray" paddingX={1}>
      <Text bold color="cyan">{'> '}</Text>
      {disabled ? (
        <Text dimColor>{placeholder}</Text>
      ) : (
        <TextInput
          value={value}
          onChange={setValue}
          onSubmit={handleSubmit}
          placeholder={placeholder}
          showCursor={!disabled}
        />
      )}
    </Box>
  );
};
