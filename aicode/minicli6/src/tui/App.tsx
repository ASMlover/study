import React, { useState, useCallback, useEffect, useRef } from 'react';
import { Box, Text, useApp } from 'ink';
import { Message, ToolCall } from '../types';
import { TodoItem } from '../types/todo';
import { Chat } from './components/Chat';
import { Input } from './components/Input';
import { Header } from './components/Header';
import { StatusBar } from './components/StatusBar';
import { LoadingSpinner } from './components/Spinner';
import { ToolConfirm } from './components/ToolConfirm';
import { TodoList } from './components/TodoList';
import { Agent, AgentStatus, AgentEvent } from '../agent/Agent.js';
import { ConfigManager, MiniCLIConfig } from '../core/Config.js';
import { PermissionDecision } from '../agent/Executor.js';
import { ToolCall as ToolSystemCall } from '../tools/Tool.js';
import { theme } from './theme.js';

const HELP_TEXT = `
MiniCLI - AI Agent CLI Tool

Commands:
  /help     Show this help message
  /exit     Exit the CLI
  /clear    Clear the chat history
  /reset    Reset the agent and start fresh
  /status   Show current status and statistics

Shortcuts:
  Ctrl+C    Exit the CLI
  Ctrl+L    Clear screen

Available Tools:
  bash      Execute shell commands
  read      Read files
  write     Write files
  edit      Edit files
  glob      Find files by pattern
  grep      Search file contents
  task      Run sub-agent tasks
  todowrite Manage todo list
  skill     Execute skills
  background Run background tasks

For more information, visit: https://github.com/yourname/minicli
`;

const TOOL_DESCRIPTIONS: Record<string, string> = {
  bash: 'Execute shell commands in a persistent shell session',
  read: 'Read a file or directory from the local filesystem',
  write: 'Write content to a file on the local filesystem',
  edit: 'Perform exact string replacements in files',
  glob: 'Fast file pattern matching tool',
  grep: 'Fast content search tool using regular expressions',
  todowrite: 'Manage a todo list for tracking task progress',
};

function getToolDescription(name: string): string {
  return TOOL_DESCRIPTIONS[name] || 'Unknown tool';
}

function convertToolCall(tc: ToolSystemCall): ToolCall {
  return {
    id: tc.id,
    name: tc.name,
    arguments: tc.arguments,
  };
}

interface AppProps {
  initialPrompt?: string;
  configPath?: string;
  model?: string;
  debug?: boolean;
}

const MainContent: React.FC<{
  messages: Message[];
  streamingMessage?: Message;
  todos: TodoItem[];
  status: AgentStatus;
}> = React.memo(({ messages, streamingMessage, todos, status }) => (
  <>
    <Box flexGrow={1}>
      <Chat messages={messages} streamingMessage={streamingMessage} />
    </Box>

    {todos.length > 0 && (
      <Box marginBottom={1}>
        <TodoList items={todos} />
      </Box>
    )}

    {status === 'thinking' && (
      <Box marginBottom={1}>
        <LoadingSpinner status="thinking" text="Thinking..." />
      </Box>
    )}

    {status === 'tool_use' && (
      <Box marginBottom={1}>
        <LoadingSpinner status="tool_use" text="Executing tool..." />
      </Box>
    )}
  </>
));

MainContent.displayName = 'MainContent';

export const App: React.FC<AppProps> = ({ initialPrompt, configPath, model: modelArg, debug }) => {
  const { exit } = useApp();
  const [agent, setAgent] = useState<Agent | null>(null);
  const [messages, setMessages] = useState<Message[]>([]);
  const [status, setStatus] = useState<AgentStatus>('idle');
  const [tokens, setTokens] = useState<number>(0);
  const [currentModel, setCurrentModel] = useState<string>('glm-4-plus');
  const [streamingMessage, setStreamingMessage] = useState<Message | undefined>();
  const [initError, setInitError] = useState<string | null>(null);
  const streamingRef = useRef<{ content: string; id: string } | null>(null);
  const configRef = useRef<MiniCLIConfig | null>(null);
  
  const [pendingToolCall, setPendingToolCall] = useState<ToolCall | null>(null);
  const pendingToolResolveRef = useRef<((decision: PermissionDecision) => void) | null>(null);
  const [todos, setTodos] = useState<TodoItem[]>([]);
  const lastToolCallRef = useRef<ToolCall | null>(null);
  const initialPromptSentRef = useRef(false);
  const agentRef = useRef<Agent | null>(null);

  useEffect(() => {
    const initAgent = async () => {
      try {
        const configManager = new ConfigManager();
        const config = await configManager.load(configPath);
        configRef.current = config;

        if (!config.provider.apiKey) {
          setInitError('API key not configured. Please set GLM_API_KEY environment variable or configure in ~/.minicli/config.yaml');
          return;
        }

        const effectiveModel = modelArg || config.provider.model;
        setCurrentModel(effectiveModel);

        const agentInstance = new Agent({
          config: { ...config, provider: { ...config.provider, model: effectiveModel } },
          onStatusChange: setStatus,
          onToolCall: async (toolCall: ToolSystemCall): Promise<PermissionDecision> => {
            return new Promise((resolve) => {
              setPendingToolCall(convertToolCall(toolCall));
              pendingToolResolveRef.current = resolve;
            });
          },
        });

        agentInstance.setSystemPrompt('You are a helpful AI assistant.');
        agentRef.current = agentInstance;
        setAgent(agentInstance);
      } catch (error) {
        setInitError(`Failed to initialize agent: ${(error as Error).message}`);
      }
    };

    initAgent();
  }, [configPath, modelArg]);

  const handleToolDecision = useCallback((allowed: boolean, scope: 'once' | 'session' | 'permanent') => {
    if (pendingToolResolveRef.current) {
      pendingToolResolveRef.current({ allowed, scope });
      pendingToolResolveRef.current = null;
    }
    setPendingToolCall(null);
  }, []);

  const handleInput = useCallback(async (text: string) => {
    if (text.startsWith('/')) {
      const command = text.toLowerCase().trim();
      
      switch (command) {
        case '/help':
          setMessages(prev => [...prev, {
            id: Date.now().toString(),
            role: 'assistant',
            content: HELP_TEXT,
            timestamp: new Date(),
          }]);
          return;
          
        case '/exit':
        case '/quit':
        case '/q':
          exit();
          return;
          
        case '/clear':
          setMessages([]);
          return;
          
        case '/reset':
          setMessages([]);
          setTodos([]);
          setTokens(0);
          if (agentRef.current) {
            agentRef.current.reset();
          }
          setMessages([{
            id: Date.now().toString(),
            role: 'assistant',
            content: 'Agent reset. Starting fresh session.',
            timestamp: new Date(),
          }]);
          return;
          
        case '/status':
          setMessages(prev => [...prev, {
            id: Date.now().toString(),
            role: 'assistant',
            content: `Status: ${status}\nMessages: ${prev.length}\nTokens: ~${tokens}\nModel: ${currentModel}`,
            timestamp: new Date(),
          }]);
          return;
          
        default:
          setMessages(prev => [...prev, {
            id: Date.now().toString(),
            role: 'assistant',
            content: `Unknown command: ${text}\nType /help for available commands.`,
            timestamp: new Date(),
          }]);
          return;
      }
    }

    const currentAgent = agentRef.current;
    if (!currentAgent) {
      return;
    }

    const userMessage: Message = {
      id: Date.now().toString(),
      role: 'user',
      content: text,
      timestamp: new Date(),
    };

    setMessages(prev => [...prev, userMessage]);

    const assistantId = (Date.now() + 1).toString();
    streamingRef.current = { content: '', id: assistantId };

    setStreamingMessage({
      id: assistantId,
      role: 'assistant',
      content: '',
      timestamp: new Date(),
    });

    try {
      for await (const event of currentAgent.sendMessage(text)) {
        switch (event.type) {
          case 'text':
            if (streamingRef.current) {
              streamingRef.current.content += event.content;
              setStreamingMessage(prev =>
                prev ? { ...prev, content: streamingRef.current!.content } : prev
              );
            }
            break;

          case 'tool_call':
            const toolMsg: Message = {
              id: `${assistantId}-tool-${event.tool.id}`,
              role: 'assistant',
              content: `[Calling tool: ${event.tool.function.name}]`,
              timestamp: new Date(),
              toolCalls: [
                {
                  id: event.tool.id,
                  name: event.tool.function.name,
                  arguments: JSON.parse(event.tool.function.arguments || '{}'),
                },
              ],
            };
            lastToolCallRef.current = toolMsg.toolCalls![0];
            setMessages(prev => [...prev, toolMsg]);
            break;

          case 'tool_result':
            const resultMsg: Message = {
              id: `${assistantId}-result-${Date.now()}`,
              role: 'tool',
              content: event.result.output,
              timestamp: new Date(),
              toolCallId: '',
            };
            setMessages(prev => [...prev, resultMsg]);
            
            if (lastToolCallRef.current?.name === 'todowrite' && event.result.metadata?.todos) {
              setTodos(event.result.metadata.todos);
            }
            break;

          case 'done':
            if (streamingRef.current && streamingRef.current.content) {
              setMessages(prev => [...prev, {
                id: assistantId,
                role: 'assistant',
                content: streamingRef.current!.content,
                timestamp: new Date(),
              }]);
            }
            setStreamingMessage(undefined);
            streamingRef.current = null;
            break;

          case 'error':
            setMessages(prev => [...prev, {
              id: `${assistantId}-error`,
              role: 'assistant',
              content: `Error: ${event.error.message}`,
              timestamp: new Date(),
            }]);
            setStreamingMessage(undefined);
            streamingRef.current = null;
            break;
        }
      }
    } catch (error) {
      setMessages(prev => [...prev, {
        id: `${assistantId}-error`,
        role: 'assistant',
        content: `Error: ${(error as Error).message}`,
        timestamp: new Date(),
      }]);
      setStreamingMessage(undefined);
      streamingRef.current = null;
    }

    setTokens(prev => prev + 100);
  }, [exit, currentModel, status, tokens]);

  useEffect(() => {
    if (agent && initialPrompt && !initialPromptSentRef.current) {
      initialPromptSentRef.current = true;
      handleInput(initialPrompt);
    }
  }, [agent, initialPrompt, handleInput]);

  const isInputDisabled = status !== 'idle' || !agent || pendingToolCall !== null;
  const inputPlaceholder = agent ? 'Type a message...' : 'Initializing...';

  if (initError) {
    return (
      <Box padding={1}>
        <Text color="red">{initError}</Text>
      </Box>
    );
  }

  return (
    <Box flexDirection="column" height="100%" padding={1}>
      <Header model={currentModel} />

      <MainContent
        messages={messages}
        streamingMessage={streamingMessage}
        todos={todos}
        status={status}
      />

      {pendingToolCall && (
        <Box marginBottom={1}>
          <ToolConfirm
            toolCall={pendingToolCall}
            toolDescription={getToolDescription(pendingToolCall.name)}
            onDecision={handleToolDecision}
          />
        </Box>
      )}

      <Input
        onSubmit={handleInput}
        disabled={isInputDisabled}
        placeholder={inputPlaceholder}
      />

      <StatusBar
        status={status}
        tokens={tokens}
        model={currentModel}
      />
    </Box>
  );
};
