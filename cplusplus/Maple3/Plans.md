# Maple3 Plans

## Current Focus
- Stabilize frontend quality gates (Scanner + CTest) and bootstrap parser/AST baseline.

## Plan
1. Keep repository docs/rules synchronized with the implemented build+test workflow.
2. Grow the interpreter pipeline (scanner -> parser -> runtime -> CLI) while keeping codebase small and readable.
3. Keep tests targeted and runnable via `ctest` for every feature increment.

## Progress Log
- 2026-02-05: Added project background expansion in `AGENTS.md`.
- 2026-02-05: Switched workflow rules to reference `Plans.md`.
- 2026-02-05: Created `Plans.md` with initial plan and progress log.
- 2026-02-06: Enabled CTest in `CMakeLists.txt` and registered `MapleScanTest`/`MapleParserTest`.
- 2026-02-06: Upgraded scanner diagnostics with file/line/column token metadata (`Token.*`, `Scanner.*`, `Errors.*`).
- 2026-02-06: Added minimal expression AST/parser (`Ast.*`, `Parser.*`) with parser recovery tests.
- 2026-02-06: Converted scanner smoke test to assertion-style checks and added parser tests.
- 2026-02-06: Added execution playbook files `Todos.md` and `tasks/task_01.md` ... `tasks/task_10.md`.
- 2026-02-06: Standardized per-task structure for implementation steps, testcase matrix, run commands, and `Todos.md` progress update rules.
- 2026-02-06: Removed deprecated roadmap docs `NEXT_STEPS.md` and `NEXT_STEPS.zh-CN.md` (migrated to `Todos.md` + `tasks/`).
- 2026-02-06: Added `Designs.md` to document current/target implementation architecture and module responsibilities.
- 2026-02-06: Updated `AGENTS.md` workflow rule to require `Designs.md` synchronization whenever implementation changes.
- 2026-02-06: Removed obsolete `SESSION_LOG.md` (project status is tracked in `Plans.md`/`Todos.md`).
- 2026-02-06: Added `Guides.md` for implementation development guidance and language usage examples.
- 2026-02-06: Updated `AGENTS.md` workflow rule to require `Guides.md` synchronization when language implementation changes.

## Next Steps
1. Execute `imp tasks/task_01.md` to build script regression framework and CTest integration.
2. Progress strictly in order from `task_01` to `task_10`, updating `Todos.md` after each done task.
3. Keep all gates green (`cmake`, build, `ctest`) before moving to the next task.
4. Remove temporary compiler macro override in `Macros.hh` after cross-compiler verification.
