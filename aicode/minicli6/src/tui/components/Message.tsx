import React, { useMemo } from 'react';
import { Box, Text } from 'ink';
import chalk from 'chalk';
import { marked } from 'marked';
import TerminalRenderer from 'marked-terminal';
import { Message as MessageType } from '../../types';
import { StreamText } from './StreamText';
import { theme, getRoleConfig } from '../theme';

interface MessageProps {
  message: MessageType;
  isStreaming?: boolean;
}

const renderer = new TerminalRenderer({
  showSectionPrefix: false,
  tab: 2,
  width: 76,
  reflowText: true,
  code: chalk.hex(theme.colors.accent),
  blockquote: chalk.hex(theme.colors.dim).italic,
  html: chalk.hex(theme.colors.info),
  heading: chalk.hex(theme.colors.primary).bold,
  firstHeading: chalk.hex(theme.colors.primary).bold,
  hr: chalk.hex(theme.colors.dim),
  listitem: chalk.hex(theme.colors.text),
  list: chalk.hex(theme.colors.text),
  paragraph: chalk.hex(theme.colors.text),
  table: chalk.hex(theme.colors.text),
});

marked.setOptions({ renderer });

const formatTime = (date: Date): string => {
  return date.toLocaleTimeString('zh-CN', {
    hour: '2-digit',
    minute: '2-digit',
  });
};

const truncateContent = (content: string, maxLength: number = 200): string => {
  if (content.length <= maxLength) return content;
  return content.substring(0, maxLength) + '...';
};

export const Message: React.FC<MessageProps> = React.memo(({ message, isStreaming = false }) => {
  const roleConfig = useMemo(() => getRoleConfig(message.role), [message.role]);

  const renderedContent = useMemo(() => {
    if (!message.content) return '';
    try {
      return marked(message.content) as string;
    } catch {
      return message.content;
    }
  }, [message.content]);

  const toolCallsRendered = useMemo(() => {
    if (!message.toolCalls || message.toolCalls.length === 0) return null;
    return (
      <Box flexDirection="column" marginTop={1}>
        {message.toolCalls.map((tool, index) => (
          <Box key={tool.id || index} flexDirection="column">
            <Box>
              <Text>
                <Text color={roleConfig.color}>{theme.icons.arrowDouble} </Text>
                <Text bold color={theme.colors.tool}>
                  {tool.name}
                </Text>
              </Text>
            </Box>
            <Box marginLeft={3}>
              <Text dimColor color={theme.colors.dim}>
                {truncateContent(JSON.stringify(tool.arguments, null, 0), 100)}
              </Text>
            </Box>
          </Box>
        ))}
      </Box>
    );
  }, [message.toolCalls, roleConfig.color]);

  const toolResultRendered = useMemo(() => {
    if (message.role !== 'tool') return null;
    const isSuccess = !message.content.toLowerCase().includes('error');
    const resultColor = isSuccess ? theme.colors.success : theme.colors.error;
    const resultIcon = isSuccess ? theme.icons.success : theme.icons.error;

    return (
      <Box flexDirection="column">
        <Box>
          <Text color={resultColor}>
            {resultIcon} {isSuccess ? 'Success' : 'Error'}
          </Text>
        </Box>
        <Box marginLeft={2}>
          <Text dimColor>{truncateContent(message.content, 300)}</Text>
        </Box>
      </Box>
    );
  }, [message.role, message.content]);

  return (
    <Box flexDirection="column" paddingY={0} marginY={1}>
      <Box>
        <Box marginRight={1}>
          <Text bold color={roleConfig.color}>
            {roleConfig.icon} {roleConfig.label}
          </Text>
        </Box>
        <Text dimColor color={theme.colors.dim}>
          {formatTime(message.timestamp)}
        </Text>
      </Box>

      <Box
        marginLeft={2}
        paddingLeft={1}
        borderStyle="round"
        borderColor={roleConfig.color}
        flexDirection="column"
      >
        {message.role === 'tool' ? (
          toolResultRendered
        ) : isStreaming ? (
          <StreamText text={message.content} isStreaming={isStreaming} />
        ) : (
          <Text>{renderedContent}</Text>
        )}
        {toolCallsRendered}
      </Box>
    </Box>
  );
});

Message.displayName = 'Message';
