import React, { useMemo } from 'react';
import { Text } from 'ink';
import chalk from 'chalk';
import { marked } from 'marked';
import TerminalRenderer from 'marked-terminal';
import { theme } from '../theme';

interface StreamTextProps {
  text: string;
  isStreaming?: boolean;
}

const renderer = new TerminalRenderer({
  showSectionPrefix: false,
  tab: 2,
  width: 72,
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

export const StreamText: React.FC<StreamTextProps> = ({ text, isStreaming = false }) => {
  const renderedContent = useMemo(() => {
    if (!text) return '';
    try {
      return marked(text) as string;
    } catch {
      return text;
    }
  }, [text]);

  const cursor = isStreaming
    ? chalk.hex(theme.colors.primary)(theme.icons.cursor)
    : '';

  return (
    <Text>
      {renderedContent}
      {cursor}
    </Text>
  );
};
