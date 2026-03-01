import React from 'react';
import { Box, Text } from 'ink';
import { theme } from '../theme';

interface HeaderProps {
  model?: string;
}

const VERSION = '0.1.0';
const TITLE = 'MiniCLI';
const SUBTITLE = 'AI Agent Interface';

export const Header: React.FC<HeaderProps> = React.memo(({ model }) => {
  return (
    <Box flexDirection="column" marginBottom={1}>
      <Box>
        <Text bold color={theme.colors.primary}>
          {theme.borders.corner.topLeft}
          {theme.borders.doubleHorizontal.repeat(2)}
          {theme.borders.connector.left}
        </Text>
        <Text> </Text>
        <Text bold color={theme.colors.primary}>
          {theme.icons.star}
        </Text>
        <Text> </Text>
        <Text bold color={theme.colors.primary}>
          {TITLE}
        </Text>
        <Text dimColor color={theme.colors.dim}>
          {' '}
          v{VERSION}
        </Text>
        <Text> </Text>
        <Text dimColor color={theme.colors.dim}>
          {SUBTITLE}
        </Text>
        {model && (
          <>
            <Text> </Text>
            <Text color={theme.colors.info}>
              {theme.icons.arrowDouble} {model}
            </Text>
          </>
        )}
      </Box>
      <Box>
        <Text dimColor color={theme.colors.dim}>
          {theme.borders.vertical}
          {'  '}
          {theme.borders.thinHorizontal.repeat(70)}
        </Text>
      </Box>
    </Box>
  );
});

Header.displayName = 'Header';
