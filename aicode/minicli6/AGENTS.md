# MiniCLI Agents Guide

## Build & Test Commands
- **Build**: `npm run build`
- **Dev**: `npm run dev`
- **Test**: `npm test`
- **Lint**: `npm run lint`
- **TypeCheck**: `npm run typecheck`

## Project Overview

MiniCLI is an AI Agent CLI tool inspired by Claude Code, built with:
- **Runtime**: Node.js + TypeScript
- **TUI Framework**: Ink (React-style)
- **Config Format**: YAML
- **AI Provider**: GLM5 (智谱 AI)
- **Distribution**: esbuild/pkg bundling

## Architecture

```
src/
├── index.ts              # Entry point
├── cli.ts                # CLI commands (commander)
│
├── agent/                # Agent core
│   ├── Agent.ts          # Main agent class
│   ├── Loop.ts           # Agent main loop
│   ├── Executor.ts       # Tool executor
│   └── SubAgent.ts       # Sub-agent management
│
├── core/                 # Core modules
│   ├── Config.ts         # Configuration manager
│   ├── Context.ts        # Context management
│   ├── Compact.ts        # Context compression
│   └── Provider.ts       # GLM5 API provider
│
├── tools/                # Tool system
│   ├── Tool.ts           # Base tool class
│   ├── Registry.ts       # Tool registry
│   ├── Permission.ts     # Permission manager
│   └── impl/             # Tool implementations
│       ├── Bash.ts
│       ├── Read.ts
│       ├── Write.ts
│       ├── Edit.ts
│       ├── Glob.ts
│       ├── Grep.ts
│       └── Task.ts
│
├── skills/               # Skill system
│   ├── SkillManager.ts
│   └── Skill.ts
│
├── tasks/                # Task system
│   ├── TaskRunner.ts
│   ├── Scheduler.ts
│   ├── Background.ts
│   └── Orchestration.ts
│
├── tui/                  # TUI components
│   ├── App.tsx
│   └── components/
│
└── utils/                # Utilities
    ├── logger.ts
    ├── stream.ts
    └── diff.ts
```

## Code Conventions

### TypeScript
- Use strict mode
- Prefer interfaces over types for object shapes
- Use async/await over Promise.then
- Export functions/classes at end of file

### React/Ink Components
- Functional components with hooks
- Use TypeScript for props
- Keep components focused and small

### File Naming
- PascalCase for classes/components: `Agent.ts`, `App.tsx`
- camelCase for utilities: `logger.ts`, `stream.ts`

### Imports Order
1. Node built-ins
2. External packages
3. Internal modules (absolute paths)
4. Relative imports

## Tool Implementation Guide

### Creating a New Tool

```typescript
// src/tools/impl/MyTool.ts
import { Tool, ToolResult, ToolCall } from '../Tool';

export class MyTool extends Tool {
  name = 'my_tool';
  description = 'Description of what this tool does';
  
  parameters = {
    type: 'object',
    properties: {
      param1: {
        type: 'string',
        description: 'Parameter description',
      },
    },
    required: ['param1'],
  };

  async execute(call: ToolCall): Promise<ToolResult> {
    const { param1 } = call.arguments;
    
    // Implementation
    
    return {
      success: true,
      output: 'Result',
    };
  }
}
```

### Registering a Tool

```typescript
// In src/tools/Registry.ts
import { MyTool } from './impl/MyTool';

registry.register(new MyTool());
```

## GLM5 API Integration

### Configuration
```yaml
provider:
  type: glm5
  apiKey: ${GLM_API_KEY}
  baseUrl: https://open.bigmodel.cn/api/paas/v4
  model: glm-4-plus
```

### Streaming Response
- Use SSE (Server-Sent Events)
- Handle `content` delta for text
- Handle `tool_calls` for function calling

## Permission System

Tool permissions are managed at three levels:
1. **auto**: Execute without confirmation
2. **confirm**: Ask user each time
3. **deny**: Always reject

Permission can be granted:
- Per invocation
- Per session
- Permanently (stored in config)

## Context Management

### Token Limits
- GLM-4-Plus: 128K context
- Compact threshold: 100K tokens
- Preserve recent: 10 messages

### Compact Strategy
1. Keep system messages
2. Summarize old messages
3. Keep recent N messages
4. Preserve tool results

## TUI Components

### Main Components
- `App.tsx`: Root component
- `Chat.tsx`: Message list display
- `Input.tsx`: User input with history
- `Spinner.tsx`: Thinking animation
- `TodoList.tsx`: Task progress display
- `ToolConfirm.tsx`: Permission dialog
- `StreamText.tsx`: Streaming text renderer

### State Management
- Use React hooks (useState, useReducer)
- Agent state shared via context

## Error Handling

- Log errors with context
- Display user-friendly messages
- Allow retry for recoverable errors
- Graceful degradation for non-critical failures
