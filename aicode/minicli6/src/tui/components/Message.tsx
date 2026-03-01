import React from 'react';
import { Box, Text } from 'ink';
import chalk from 'chalk';
import { marked } from 'marked';
import TerminalRenderer from 'marked-terminal';
import { Message as MessageType } from '../../types';
import { StreamText } from './StreamText';

interface MessageProps {
  message: MessageType;
  isStreaming?: boolean;
}

marked.setOptions({
  renderer: new TerminalRenderer({
    showSectionPrefix: false,
    tab: 2,
    width: 80,
    reflowText: true,
  }),
});

export const Message: React.FC<MessageProps> = ({ message, isStreaming = false }) => {
  const getRoleColor = (role: string): string => {
    switch (role) {
      case 'user':
        return 'green';
      case 'assistant':
        return 'blue';
      case 'system':
        return 'yellow';
      case 'tool':
        return 'magenta';
      default:
        return 'white';
    }
  };

  const getRoleIcon = (role: string): string => {
    switch (role) {
      case 'user':
        return '👤';
      case 'assistant':
        return '🤖';
      case 'system':
        return '⚙️';
      case 'tool':
        return '🔧';
      default:
        return '•';
    }
  };

  const formatTime = (date: Date): string => {
    return date.toLocaleTimeString('zh-CN', { 
      hour: '2-digit', 
      minute: '2-digit' 
    });
  };

  const renderMarkdown = (content: string): string => {
    try {
      return marked(content) as string;
    } catch {
      return content;
    }
  };

  const renderToolCalls = (toolCalls: any[]) => {
    return (
      <Box flexDirection="column" marginTop={1}>
        {toolCalls.map((tool, index) => (
          <Box key={tool.id || index} flexDirection="column">
            <Text color="magenta" bold>
              Tool: {tool.name}
            </Text>
            <Text dimColor>
              {JSON.stringify(tool.arguments, null, 2)}
            </Text>
          </Box>
        ))}
      </Box>
    );
  };

  return (
    <Box flexDirection="column" marginY={1}>
      <Box>
        <Text bold color={getRoleColor(message.role)}>
          {getRoleIcon(message.role)} {message.role.toUpperCase()}
        </Text>
        <Text dimColor> {formatTime(message.timestamp)}</Text>
      </Box>
      <Box marginLeft={2} flexDirection="column">
        {isStreaming ? (
          <StreamText text={message.content} isStreaming={isStreaming} />
        ) : (
          <Text>{renderMarkdown(message.content)}</Text>
        )}
        {message.toolCalls && message.toolCalls.length > 0 && renderToolCalls(message.toolCalls)}
      </Box>
    </Box>
  );
};
