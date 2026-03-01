# MiniCLI5

AI Agent CLI Tool inspired by Claude Code, built with TypeScript.

## Quick Start

```bash
npm install
npm run dev      # Development mode (tsx)
npm run build    # Compile TypeScript
npm start        # Run compiled output
```

## Project Structure

```
src/
├── index.ts              # Entry point, REPL loop, slash commands
├── types.ts              # All type definitions (Message, Tool, Provider, Task, etc.)
├── config.ts             # Config loading/saving from .minicli5/config.json
├── agent/
│   ├── loop.ts           # Main agent loop (tool-call cycle)
│   ├── orchestrator.ts   # Multi-stage orchestration
│   ├── planner.ts        # Planning agent
│   ├── reviewer.ts       # Code review agent
│   ├── coder.ts          # Coding agent
│   └── sub-agent.ts      # Sub-agent spawning
├── provider/
│   ├── base.ts           # Provider base interface
│   ├── glm.ts            # GLM (BigModel) provider
│   └── retry.ts          # Retry logic
├── tools/
│   ├── registry.ts       # Tool registry
│   ├── definitions.ts    # Tool definitions
│   ├── read-file.ts      # File reading tool
│   ├── write-file.ts     # File writing tool
│   ├── list-dir.ts       # Directory listing tool
│   ├── grep.ts           # Content search tool
│   ├── glob.ts           # File pattern matching tool
│   ├── shell.ts          # Shell execution tool
│   └── project-tree.ts   # Project tree tool
├── tui/
│   ├── theme.ts          # Terminal color theme
│   ├── spinner.ts        # Loading spinner
│   ├── renderer.ts       # Output rendering
│   ├── prompt.ts         # User input prompt
│   └── confirm.ts        # Permission confirmation
├── context/
│   ├── session.ts        # Session persistence
│   ├── compact.ts        # Context window compaction
│   └── memory.ts         # Cross-session memory
├── tasks/
│   ├── manager.ts        # Task CRUD operations
│   ├── scheduler.ts      # Task scheduling/display
│   └── background.ts     # Background task execution
├── skills/
│   ├── registry.ts       # Skill registry
│   ├── commit.ts         # Git commit skill
│   ├── simplify.ts       # Code simplification skill
│   └── plan.ts           # Planning skill
└── utils/
    ├── logger.ts         # File-based logger
    ├── tokens.ts         # Token estimation
    └── path.ts           # Path utilities
```

## Architecture

- **Provider**: Uses GLM (BigModel) API via OpenAI-compatible chat completions endpoint
- **Agent Loop**: Iterative tool-calling loop — sends messages to LLM, executes tool calls, feeds results back
- **Tools**: Read-only tools run freely; write/shell tools require user approval via `confirmPermission`
- **Context Management**: Auto-compacts conversation when token usage exceeds `compact_threshold` ratio
- **Sessions**: Persisted to `.minicli5/sessions/` as JSON, including turns and tasks
- **Skills**: Slash-command extensions (`/commit`, `/simplify`, `/plan`) that inject prompt templates

## Key Conventions

- ESM (`"type": "module"`) — all imports use `.js` extension
- Config stored in `.minicli5/config.json` per project root
- All TUI output goes to `stderr`; only structured data to `stdout`
- No external test framework — manually test with `npm run dev`
