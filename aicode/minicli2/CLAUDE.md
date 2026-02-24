# MiniCLI2

AI Agent CLI tool based on GLM-5 model.

## Project Structure

```
minicli2/
├── .minicli2/                 # Project local config directory
│   ├── config.json            # Configuration file
│   ├── sessions/              # Session history
│   └── logs/                  # Logs
├── src/
│   ├── __init__.py
│   ├── cli.py                 # Entry point
│   ├── config.py              # Configuration management
│   ├── provider.py            # GLM API client
│   ├── session.py             # Session management
│   ├── repl.py                # REPL interaction
│   ├── tools/                 # Tools
│   │   ├── __init__.py
│   │   ├── bash.py            # Bash command execution
│   │   ├── file.py            # File read/write
│   │   ├── grep.py            # Code search
│   │   └── registry.py        # Tool registry
│   └── commands/              # CLI commands
│       ├── __init__.py
│       ├── chat.py            # Single-turn chat
│       ├── repl.py            # REPL mode
│       ├── session.py         # Session management
│       └── config.py           # Config commands
├── pyproject.toml
└── README.md
```

## Tech Stack

- Python 3.14+
- `typer` - CLI framework
- `rich` - Terminal output
- `httpx` - HTTP client
- `prompt_toolkit` - REPL interaction

## CLI Commands

```bash
# Interactive mode
minicli2                      # Enter REPL
minicli2 repl                 # Enter REPL (explicit)
minicli2 chat "your question" # Single-turn chat

# Session management
minicli2 session list         # List all sessions
minicli2 session new          # Create new session
minicli2 session switch <id> # Switch session
minicli2 session clear        # Clear current session

# Configuration
minicli2 config get <key>    # Get config
minicli2 config set <key> <value>  # Set config

# Help
minicli2 help                 # Show help
```

## REPL Slash Commands

| Command | Description |
|---------|-------------|
| `/exit` | Exit current REPL session |
| `/help` | Show help information |
| `/clear` | Clear current session history |
| `/session` | Switch or create new session |
| `/config` | View or modify config |
| `/grep` | Search code files |
| `/tools` | List all available tools |
| `/history` | View session history |
| `/context` | View context usage |

## Development

```bash
# Install in development mode
pip install -e .

# Run tests
pytest

# Run with verbose output
minicli2 --help
```

## Configuration

Configuration file: `.minicli2/config.json`

```json
{
  "api_key": "",
  "base_url": "https://open.bigmodel.cn/api/paas/v4",
  "model": "glm-5",
  "timeout_ms": 120000,
  "max_retries": 3,
  "tools_enabled": ["bash", "read", "write", "grep"]
}
```

Environment variable: `GLM_API_KEY`
