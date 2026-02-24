# MiniCLI2

AI Agent CLI tool based on GLM-5 model with REPL interaction, tool calling, and session management.

## Installation

```bash
pip install -e .
```

## Quick Start

```bash
# Enter REPL interactive mode
minicli2

# Single-turn chat
minicli2 chat "Hello, how are you?"

# Session management
minicli2 session list
minicli2 session new my-session
minicli2 session switch my-session

# Configuration
minicli2 config list
minicli2 config set model glm-5
```

## Configuration

Create `.minicli2/config.json`:

```json
{
  "api_key": "your-api-key",
  "base_url": "https://open.bigmodel.cn/api/paas/v4",
  "model": "glm-5",
  "timeout_ms": 120000,
  "max_retries": 3,
  "tools_enabled": ["bash", "read", "write", "grep"]
}
```

Or set environment variable:

```bash
export GLM_API_KEY=your-api-key
```

## REPL Commands

| Command | Description |
|---------|-------------|
| `/exit` | Exit REPL |
| `/help` | Show help |
| `/clear` | Clear session |
| `/session` | Manage sessions |
| `/config` | Manage config |
| `/tools` | List tools |
| `/history` | View history |
| `/context` | View context usage |

## Tools

- **bash**: Execute shell commands
- **read**: Read files
- **write**: Write files
- **grep**: Search code

## Development

```bash
# Install dev dependencies
pip install -e ".[dev]"

# Run tests
pytest
```
