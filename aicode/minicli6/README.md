# MiniCLI

AI Agent CLI tool powered by GLM5, inspired by Claude Code.

## Features

- 🤖 **AI Agent**: Intelligent agent with tool use capabilities
- 🔧 **Tool System**: Bash, Read, Write, Edit, Glob, Grep, and more
- 📋 **Todo List**: Track task progress with built-in todo management
- 🔄 **Context Compact**: Automatic context compression for long conversations
- 🎯 **Sub-Agent**: Spawn sub-agents for parallel task execution
- 📦 **Skills**: Extensible skill system for specialized tasks
- ⚡ **Background Tasks**: Run tasks in background with status tracking
- 🎨 **Beautiful TUI**: Interactive terminal UI with streaming output

## Installation

### From Source

```bash
git clone https://github.com/yourname/minicli.git
cd minicli
npm install
npm run build
npm link
```

### From Release

Download the executable for your platform from [Releases](releases).

## Quick Start

```bash
# Set your GLM API key
export GLM_API_KEY=your_api_key

# Start interactive session
minicli start

# Start with initial prompt
minicli start "Help me refactor this code"
```

## Configuration

Create `~/.minicli/config.yaml`:

```yaml
provider:
  type: glm5
  apiKey: ${GLM_API_KEY}
  baseUrl: https://open.bigmodel.cn/api/paas/v4
  model: glm-4-plus

agent:
  maxIterations: 50
  timeout: 300000

tools:
  permissions:
    bash: confirm
    read: auto
    write: confirm
    edit: confirm
```

## Available Tools

| Tool | Description | Permission |
|------|-------------|------------|
| bash | Execute shell commands | confirm |
| read | Read file contents | auto |
| write | Write to files | confirm |
| edit | Edit files by replacement | confirm |
| glob | Find files by pattern | auto |
| grep | Search file contents | auto |
| task | Run sub-agent tasks | confirm |
| todowrite | Manage todo list | auto |
| skill | Execute skills | confirm |
| background | Run background tasks | confirm |

## Skills

Skills are reusable task templates. Place them in `~/.minicli/skills/`.

Example skill:

```yaml
name: code-review
description: Perform code review
version: 1.0.0
parameters:
  - name: files
    type: string
    required: true
prompt: |
  Review these files: {{files}}
  Focus on: code quality, security, performance
```

## Development

```bash
npm run dev      # Development mode
npm run build    # Build
npm test         # Run tests
```

## License

MIT
