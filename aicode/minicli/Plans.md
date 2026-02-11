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
| T01 | 鍒濆鍖?CLI 宸ョ▼楠ㄦ灦 | M1 | DONE | Codex | 2026-02-10 | 2026-02-10 | Node22+TS5 scaffold, CLI entry, --version, unit/integration tests |
| T02 | REPL 绌哄惊鐜?| M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Implemented readline REPL loop, input echo, EOF/SIGINT safe exit, and tests |
| T03 | /help 与 /exit | M1 | DONE | Codex | 2026-02-11 | 2026-02-11 | Added REPL command routing with /help and /exit, unknown command hints, and tests |
| T04 | 鍙屽眰閰嶇疆鍔犺浇 | M1 | TODO | - | - | - | - |
| T05 | 閰嶇疆鏍￠獙涓庢姤閿?| M1 | TODO | - | - | - | - |
| T06 | LLMProvider 鎺ュ彛涓?Mock | M1 | TODO | - | - | - | - |
| T07 | GLM OpenAI-compatible 閫氳矾 | M1 | TODO | - | - | - | - |
| T08 | /login 涓?/model | M1 | TODO | - | - | - | - |
| T09 | 鍗曡疆闂瓟闂幆 | M1 | TODO | - | - | - | - |
| T10 | 瓒呮椂涓庨敊璇槧灏?| M1 | TODO | - | - | - | - |
| T11 | SQLite 涓?migration v1 | M2 | TODO | - | - | - | - |
| T12 | 娑堟伅鎸佷箙鍖栦粨鍌?| M2 | TODO | - | - | - | - |
| T13 | /new | M2 | TODO | - | - | - | - |
| T14 | /sessions | M2 | TODO | - | - | - | - |
| T15 | /switch | M2 | TODO | - | - | - | - |
| T16 | /history | M2 | TODO | - | - | - | - |
| T17 | 鍛戒护娉ㄥ唽涓績 | M3 | TODO | - | - | - | - |
| T18 | / 琛ュ叏 v1 | M3 | TODO | - | - | - | - |
| T19 | Tab 鎺ュ彈琛ュ叏 | M3 | TODO | - | - | - | - |
| T20 | 鍊欓€夊鑸笌鍙栨秷 | M3 | TODO | - | - | - | - |
| T21 | 棰戞鎺掑簭 | M3 | TODO | - | - | - | - |
| T22 | /run 鍙鎵ц鍣?| M4 | TODO | - | - | - | - |
| T23 | 椋庨櫓鍒嗙骇鍣?| M4 | TODO | - | - | - | - |
| T24 | 纭娴佺▼ | M4 | TODO | - | - | - | - |
| T25 | 瀹¤璁板綍 | M4 | TODO | - | - | - | - |
| T26 | /add | M5 | TODO | - | - | - | - |
| T27 | /drop | M5 | TODO | - | - | - | - |
| T28 | /files | M5 | TODO | - | - | - | - |
| T29 | /grep | M5 | TODO | - | - | - | - |
| T30 | /tree | M5 | TODO | - | - | - | - |
| T31 | 涓婁笅鏂囩粍瑁呭櫒 | M5 | TODO | - | - | - | - |
| T32 | Token 棰勭畻瑁佸壀 | M5 | TODO | - | - | - | - |
| T33 | JSON Schema 鍛戒护娉ㄥ唽 | M6 | TODO | - | - | - | - |
| T34 | 鍛戒护鎵╁睍鍒?18 鏉?| M6 | TODO | - | - | - | - |
| T35 | 鎵╁睍鍒?27 鏉?+ 鍙傛暟琛ュ叏 | M6 | TODO | - | - | - | - |
| T36 | /config | M6 | TODO | - | - | - | - |
| T37 | /export | M6 | TODO | - | - | - | - |
| T38 | Shell 缁熶竴閫傞厤灞?| M7 | TODO | - | - | - | - |
| T39 | 鎵撳寘鍙戝竷浜х墿 | M7 | TODO | - | - | - | - |
| T40 | CI 璐ㄩ噺闂ㄧ | M7 | TODO | - | - | - | - |

## Change Log
| Date | Task | Update | By |
|---|---|---|---|
| 2026-02-10 | Init | Created planning tracker | - |
| 2026-02-10 | T01 | Completed Node22+TS5 scaffold with CLI entrypoint and passing tests | Codex |
| 2026-02-11 | T02 | Completed REPL loop with echo, long-line truncation, EOF/SIGINT handling, and tests | Codex |
| 2026-02-11 | T03 | Completed /help and /exit command routing with unit/integration coverage | Codex |
