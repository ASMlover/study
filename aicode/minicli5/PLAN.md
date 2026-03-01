# MiniCLI5 Development Plan

## Current Status

v0.1.0 — Core REPL with GLM provider, tool-calling agent loop, session management, and basic skills.

## Completed

- [x] Project scaffolding (TypeScript, ESM, tsx dev mode)
- [x] GLM provider with streaming and retry logic
- [x] Tool system: read-file, write-file, list-dir, grep, glob, shell, project-tree
- [x] Agent loop with iterative tool-call cycle
- [x] Multi-agent architecture: planner, coder, reviewer, orchestrator, sub-agent
- [x] TUI: theme, spinner, prompt with tab-completion, permission confirmation
- [x] Session persistence and restoration
- [x] Context compaction (auto and manual)
- [x] Cross-session memory
- [x] Task management (create, update, list, dependencies)
- [x] Skills system: /commit, /simplify, /plan
- [x] Slash commands: /help, /clear, /status, /tasks, /compact, /session, /skills

## TODO

### High Priority

- [ ] Add edit-file tool (patch-based editing instead of full rewrite)
- [ ] Improve error handling in agent loop (graceful recovery from malformed tool calls)
- [ ] Add token usage display after each turn
- [ ] Support multiple providers (OpenAI, Anthropic, Ollama)

### Medium Priority

- [ ] Add `/diff` skill for reviewing git changes
- [ ] Implement background task execution (run tools in parallel)
- [ ] Add file-watching mode for auto-context updates
- [ ] Improve context compaction with structured summaries
- [ ] Add `.minicli5ignore` for excluding files from tools

### Low Priority

- [ ] Add unit tests
- [ ] Add CI/CD pipeline
- [ ] Publish as npm package
- [ ] Plugin system for custom tools and skills
- [ ] Web UI alternative to terminal TUI

## Design Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Runtime | Node.js + tsx | Fast iteration, TypeScript native |
| LLM API | OpenAI-compatible | GLM supports it; easy to add more providers |
| Config format | JSON | Simple, no extra deps |
| Session storage | File-based JSON | No database dependency |
| Output target | stderr for TUI | Keeps stdout clean for piping |
