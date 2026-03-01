# minicli7

Independent AI Agent CLI with Claude-Code-like interaction flow.

## Features

- Agent main loop
- Tool use with per-call permission confirmation
- Streaming model output with thinking spinner
- TODO plan list
- Sub-agent support
- Skill support (`.minicli/skills/*.md`)
- Context compaction
- Task system and orchestration
- Background jobs
- Local project config and GLM-5 model support

## Quick Start

1. Configure API key (PowerShell):

```powershell
$env:GLM_API_KEY="your_api_key"
```

2. Optional: edit local config at `.minicli/config.json`

3. Run:

```powershell
npm start
```

## Commands

- `/help`
- `/status`
- `/context`
- `/exit`
- `/todo add <text>`
- `/todo done <id>`
- `/todo list`
- `/task add <title> [--deps=1,2] [--goal=text]`
- `/task list`
- `/task run`
- `/bg run <shell-command>`
- `/bg list`
- `/skills list`
- `/skills show <name>`
- `/skills run <name> <input>`
- `/agent <goal>`
- `/compact`
- `/clear`

## Config

Config file is local to this project: `.minicli/config.json`

Example:

```json
{
  "model": "glm-5",
  "apiBaseUrl": "https://open.bigmodel.cn/api/paas/v4/chat/completions",
  "apiKeyEnv": "GLM_API_KEY",
  "maxContextTokens": 12000,
  "compactKeepRecentMessages": 12,
  "autoCompact": true,
  "permission": {
    "allowReadWithoutPrompt": false
  }
}
```
