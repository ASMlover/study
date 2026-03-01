# AGENTS

This project is an independent AI agent CLI implementation.

## Scope

- Keep this project self-contained under the current directory.
- Do not import implementation code from sibling repositories/directories.
- Prefer explicit, testable modules over hidden side effects.

## Runtime

- Node.js >= 18
- Entry point: `src/index.js`
- Start command: `npm start`

## Configuration

- Local config path: `.minicli/config.json`
- Default model target: `glm-5`
- API key sources:
  - `apiKey` in local config
  - environment variable configured by `apiKeyEnv`
  - fallback env: `GLM_API_KEY`

## Interaction Model

- Main loop:
  - user input
  - command routing or model call
  - optional tool call loop
  - render output
- Tool calls must be explicitly approved by user (`y/N`) before execution.
- Assistant response should support streaming output.
- Thinking phase should display spinner animation.

## Core Features

- Agent main loop
- Tool use with permission gate
- Todo list
- Sub-agent execution
- Skill loading and run (`.minicli/skills`)
- Context token estimation and compact
- Task orchestration
- Background jobs

## Coding Notes

- Keep file operations scoped to current project directory.
- Add concise comments only for non-obvious logic.
- Favor small functions and clear error messages.

