# MiniCLI3 Development Plans / MiniCLI3 开发计划

## Purpose / 目的
- Track milestone progress for a standalone GLM-5 AI agent CLI.
- Keep one concise source of truth for status and delivery notes.

## Update Rules / 更新规则
- Update this file whenever milestone status changes.
- Keep notes short, factual, and dated.
- Use milestone-only tracking (no fine-grained task IDs).

## Status Legend / 状态说明
- `TODO`: not started
- `DOING`: in progress
- `BLOCKED`: blocked by dependency/decision
- `DONE`: finished and verified

## Milestones / 里程碑
| Milestone | Scope | Status | Owner | Start Date | End Date | Notes |
|---|---|---|---|---|---|---|
| M1 Foundations | Project scaffold, config/session core, CLI boot | DONE | Codex | 2026-02-24 | 2026-02-24 | Python 3.14+, standalone package |
| M2 Provider & Streaming | GLM-5 provider, retries, streaming render | DONE | Codex | 2026-02-24 | 2026-02-24 | OpenAI-compatible endpoint |
| M3 Slash Commands Core | 20-30 slash commands, help, routing | DONE | Codex | 2026-02-24 | 2026-02-24 | 20 built-in slash commands |
| M4 Completion & UX | Tab completion for command/subcommand/key args | DONE | Codex | 2026-02-24 | 2026-02-24 | prompt_toolkit + dynamic candidates |
| M5 Tools & Safety | read/write/grep/list/run with restricted policy | DONE | Codex | 2026-02-24 | 2026-02-24 | restricted paths + shell allowlist |
| M6 Quality & Packaging | tests, docs, packaging checks | DONE | Codex | 2026-02-24 | 2026-02-24 | pytest green, docs updated |

## Change Log / 变更日志
| Date | Milestone | Update | By |
|---|---|---|---|
| 2026-02-24 | M1 | Initialized plans board and subagents delivery workflow | Codex |
| 2026-02-24 | M1-M6 | Implemented standalone Python 3.14+ MiniCLI3 with REPL, streaming GLM-5 provider, slash commands, completion, restricted tools, and passing tests | Codex |
