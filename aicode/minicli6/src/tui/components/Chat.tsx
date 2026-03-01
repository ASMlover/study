import React from 'react';
import { Box, Text, Static } from 'ink';
import { Message as MessageType } from '../../types';
import { Message } from './Message';
import { theme } from '../theme';

interface ChatProps {
  messages: MessageType[];
  streamingMessage?: MessageType;
}

export const Chat: React.FC<ChatProps> = React.memo(({ messages, streamingMessage }) => {
  if (messages.length === 0 && !streamingMessage) {
    return (
      <Box flexGrow={1} flexDirection="column" justifyContent="center" alignItems="center">
        <Box marginBottom={1}>
          <Text bold color={theme.colors.primary}>
            {theme.icons.star} Welcome to MiniCLI
          </Text>
        </Box>
        <Box marginBottom={1}>
          <Text dimColor color={theme.colors.dim}>
            {theme.borders.thinHorizontal.repeat(40)}
          </Text>
        </Box>
        <Box>
          <Text color={theme.colors.text}>Start a conversation by typing a message</Text>
        </Box>
        <Box marginTop={1}>
          <Text dimColor color={theme.colors.dim}>
            {theme.icons.arrowDouble} Type /help for available commands
          </Text>
        </Box>
      </Box>
    );
  }

  return (
    <Box flexDirection="column" flexGrow={1} overflowY="hidden">
      <Static items={messages}>
        {(message) => <Message key={message.id} message={message} />}
      </Static>
      {streamingMessage && <Message message={streamingMessage} isStreaming={true} />}
    </Box>
  );
});

Chat.displayName = 'Chat';
