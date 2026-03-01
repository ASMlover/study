import React from 'react';
import { Text } from 'ink';
import Spinner from 'ink-spinner';

interface SpinnerProps {
  text?: string;
}

export const LoadingSpinner: React.FC<SpinnerProps> = ({ text = '思考中...' }) => {
  return (
    <Text>
      <Text color="cyan">
        <Spinner type="dots" />
      </Text>
      {' '}
      {text}
    </Text>
  );
};
