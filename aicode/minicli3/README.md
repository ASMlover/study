# MiniCLI3

MiniCLI3 is a standalone AI agent CLI built for GLM-5 only.

## Highlights
- Python 3.14+
- Project-local config and sessions in `./.minicli3/`
- Streaming AI responses in REPL/chat mode
- Rich slash commands with tab completion
- Restricted-by-default local tool policy

## Install

```bash
pip install -e .
```

## Quick Start

```bash
minicli3
minicli3 chat "hello"
minicli3 config set api_key your-key
```

## Data Layout
- `./.minicli3/config.json`
- `./.minicli3/sessions/*.json`
- `./.minicli3/logs/audit.log`
