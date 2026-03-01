import React, { useEffect, useState } from 'react';
import { Text } from 'ink';
import chalk from 'chalk';
import { marked } from 'marked';
import TerminalRenderer from 'marked-terminal';

interface StreamTextProps {
  text: string;
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

export const StreamText: React.FC<StreamTextProps> = ({ text, isStreaming = false }) => {
  const [displayedText, setDisplayedText] = useState('');
  const [cursorPosition, setCursorPosition] = useState(0);

  useEffect(() => {
    if (!isStreaming) {
      setDisplayedText(text);
      return;
    }

    if (text.length > cursorPosition) {
      const timer = setTimeout(() => {
        setCursorPosition(prev => Math.min(prev + 1, text.length));
        setDisplayedText(text.substring(0, cursorPosition + 1));
      }, 10);
      return () => clearTimeout(timer);
    }
  }, [text, isStreaming, cursorPosition]);

  const renderMarkdown = (content: string): string => {
    try {
      return marked(content) as string;
    } catch {
      return content;
    }
  };

  return (
    <Text>
      {renderMarkdown(displayedText)}
      {isStreaming && chalk.dim('▌')}
    </Text>
  );
};
