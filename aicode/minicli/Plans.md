# Development Plans

## Purpose
- Track the implementation plan and progress for MiniCLI.
- Keep one source of truth for scope, status, blockers, and next actions.

## Plan Rules
- Update this file whenever a task status changes.
- Keep entries short, factual, and dated.
- Do not mark a task `Done` until code, tests, and docs are complete.

## Status Legend
- `TODO`: not started
- `DOING`: in progress
- `BLOCKED`: waiting on dependency or decision
- `DONE`: finished and verified

## Milestones
- M1: Foundation and single-turn chat (`T01-T10`)
- M2: Session persistence (`T11-T16`)
- M3: Slash completion core (`T17-T21`)
- M4: Guarded tool execution (`T22-T25`)
- M5: Context management (`T26-T32`)
- M6: Full command set (`T33-T37`)
- M7: Packaging and CI (`T38-T40`)

## Task Board
| Task | Title | Milestone | Status | Owner | Start Date | End Date | Notes |
|---|---|---|---|---|---|---|---|
| T01 | 初始化 CLI 工程骨架 | M1 | DONE | Codex | 2026-02-10 | 2026-02-10 | Node22+TS5 scaffold, CLI entry, --version, unit/integration tests |
| T02 | REPL 空循环 | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Implemented readline REPL loop, input echo, EOF/SIGINT safe exit, and tests |
| T03 | /help 与 /exit | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added REPL command routing with /help and /exit, unknown command hints, and tests |
| T04 | 双层配置加载 | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added global+project config loading, merge override, and comment-aware parsing with tests |
| T05 | 配置校验与报错 | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added schema validation, default injection, warning formatting, and resilient startup for malformed config |
| T06 | LLMProvider 接口与 Mock | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added provider interface, mock provider, multi-turn request assembly, and REPL integration |
| T07 | GLM OpenAI-compatible 通路 | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added GLM OpenAI-compatible provider with request/response mapping and mock API integration tests |
| T08 | /login 与 /model | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added /login API key persistence and /model show/set with validation and tests |
| T09 | 单轮问答闭环 | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added single-turn Q&A flow helpers, empty reply fallback, reply truncation, and tests |
| T10 | 超时与错误映射 | M1 | DONE | Codex | 2026-02-12 | 2026-02-12 | Added timeout/status error mapping with retry policy and coverage for retryable/non-retryable paths |
| T11 | SQLite 与 migration v1 | M2 | DONE | Codex | 2026-02-12 | 2026-02-12 | Added SQLite initialization, migration v1 tables/indexes, startup auto-init, and tests |
| T12 | 消息持久化仓储 | M2 | DONE | Codex | 2026-02-12 | 2026-02-12 | Implemented SessionRepository/MessageRepository with pagination, sorting, and transaction rollback tests |
| T13 | /new | M2 | DONE | Codex | 2026-02-12 | 2026-02-12 | Added `/new [title]` creation+switch, unique title strategy, timestamped default naming, and message persistence to current session |
| T14 | /sessions | M2 | DONE | Codex | 2026-02-12 | 2026-02-12 | Added `/sessions` with recent-first ordering, current marker, pagination/filter args, and list formatting |
| T15 | /switch | M2 | DONE | Codex | 2026-02-12 | 2026-02-12 | Added id/index session switch, duplicate/no-target handling, and switch-write integration coverage |
| T16 | /history | M2 | TODO | - | - | - | - |
| T17 | 命令注册中心 | M3 | TODO | - | - | - | - |
| T18 | / 补全 v1 | M3 | TODO | - | - | - | - |
| T19 | Tab 接受补全 | M3 | TODO | - | - | - | - |
| T20 | 候选导航与取消 | M3 | TODO | - | - | - | - |
| T21 | 频次排序 | M3 | TODO | - | - | - | - |
| T22 | /run 只读执行器 | M4 | TODO | - | - | - | - |
| T23 | 风险分级器 | M4 | TODO | - | - | - | - |
| T24 | 确认流程 | M4 | TODO | - | - | - | - |
| T25 | 审计记录 | M4 | TODO | - | - | - | - |
| T26 | /add | M5 | TODO | - | - | - | - |
| T27 | /drop | M5 | TODO | - | - | - | - |
| T28 | /files | M5 | TODO | - | - | - | - |
| T29 | /grep | M5 | TODO | - | - | - | - |
| T30 | /tree | M5 | TODO | - | - | - | - |
| T31 | 上下文组装器 | M5 | TODO | - | - | - | - |
| T32 | Token 预算裁剪 | M5 | TODO | - | - | - | - |
| T33 | JSON Schema 命令注册 | M6 | TODO | - | - | - | - |
| T34 | 命令扩展到 18 条 | M6 | TODO | - | - | - | - |
| T35 | 扩展到 27 条 + 参数补全 | M6 | TODO | - | - | - | - |
| T36 | /config | M6 | TODO | - | - | - | - |
| T37 | /export | M6 | TODO | - | - | - | - |
| T38 | Shell 统一适配层 | M7 | TODO | - | - | - | - |
| T39 | 打包发布产物 | M7 | TODO | - | - | - | - |
| T40 | CI 质量门禁 | M7 | TODO | - | - | - | - |

## Change Log
| Date | Task | Update | By |
|---|---|---|---|
| 2026-02-10 | Init | Created planning tracker | - |
| 2026-02-10 | T01 | Completed Node22+TS5 scaffold with CLI entrypoint and passing tests | Codex |
| 2026-02-11 | T02 | Completed REPL loop with echo, long-line truncation, EOF/SIGINT handling, and tests | Codex |
| 2026-02-11 | T03 | Completed /help and /exit command routing with unit/integration coverage | Codex |
| 2026-02-11 | T04 | Completed dual-layer config loading with project-over-global merge behavior and tests | Codex |
| 2026-02-11 | T05 | Completed config schema validation and non-crashing warning flow for invalid config | Codex |
| 2026-02-11 | T06 | Completed LLMProvider contract and mock-backed REPL Q&A flow with tests | Codex |
| 2026-02-11 | T07 | Completed GLM OpenAI-compatible provider path with unit and mock API integration coverage | Codex |
| 2026-02-11 | T08 | Completed /login key persistence, masking, /model show/set flow, and unit/integration coverage | Codex |
| 2026-02-11 | T09 | Completed single-turn chat loop with input classification, reply rendering/fallback, truncation, and tests | Codex |
| 2026-02-11 | Follow-up | Relaxed model validation to allow arbitrary non-empty AI model names in config and `/model` | Codex |
| 2026-02-12 | T10 | Completed timeout handling, 401/429/5xx error mapping, retry policy, and 429-retry integration coverage | Codex |
| 2026-02-12 | T11 | Completed SQLite initialization + migration v1 (`sessions/messages/command_history`), startup auto-init, and migration/error-path tests | Codex |
| 2026-02-12 | ci T11 | Created task commit for T11 implementation and tests | Codex |
| 2026-02-12 | T12 | Completed session/message persistence repositories with create/list/pagination and transactional rollback coverage | Codex |
| 2026-02-12 | ci T12 | Created task commit for T12 implementation and tests | Codex |
| 2026-02-12 | T13 | Completed `/new` command with session switch, default timestamp title, duplicate naming policy, and integration-verified new-session message persistence | Codex |
| 2026-02-12 | T14 | Completed `/sessions` command with recent ordering, current-session marker, pagination/filter options, and integration list-output coverage | Codex |
| 2026-02-12 | T15 | Completed `/switch` command with `#id` and list-index targeting, repeated-switch/no-target guards, and integration-verified switched-session persistence | Codex |
