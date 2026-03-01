import React, { useEffect, useRef } from 'react';
import { Box, Text } from 'ink';
import { Message as MessageType } from '../../types';
import { Message } from './Message';

interface ChatProps {
  messages: MessageType[];
  streamingMessage?: MessageType;
}

export const Chat: React.FC<ChatProps> = ({ messages, streamingMessage }) => {
  return (
    <Box flexDirection="column" flexGrow={1} overflowY="hidden">
      {messages.length === 0 && !streamingMessage ? (
        <Box flexGrow={1} justifyContent="center" alignItems="center">
          <Text dimColor>开始对话...</Text>
        </Box>
      ) : (
        <>
          {messages.map((message) => (
            <Message key={message.id} message={message} />
          ))}
          {streamingMessage && (
            <Message message={streamingMessage} isStreaming={true} />
          )}
        </>
      )}
    </Box>
  );
};
