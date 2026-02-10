# MiniCLI

MiniCLI is a cross-platform (Windows/Linux) lightweight coding CLI inspired by Claude Code.  
The project is developed in small runnable increments, with task-driven execution and strict test updates.

## Current Status
- Repository is in early scaffold stage.
- Planning and task breakdown are already defined.
- Implementation follows `tasks/T01.md` to `tasks/T40.md`.

## Project Documents
- `AGENTS.md`: repository rules and agent workflow requirements.
- `IMPLEMENTATION_PLAN.md`: architecture and phased implementation plan.
- `Plans.md`: live development plan and task status board.
- `Tests.md`: per-task test method and result records.
- `tasks/INDEX.md`: task index.
- `tasks/CHECKLIST.md`: executable checklist for each task round.

## Task-Driven Workflow
Use the following pattern for execution:

1. Read `Plans.md` and target task file (for example `tasks/T01.md`).
2. Implement feature scope and test scope from that task file.
3. Run required tests and verify acceptance criteria.
4. Update `Plans.md` at task round end.
5. If successful, update `Tests.md` in the same round.

Trigger convention:
- Input `go tasks/TXX.md` means implement the corresponding task feature and tests.

## Coding and File Rules
- All code files must use UTF-8 encoding.
- All code files must use LF line endings.
- All edited code files must remove trailing whitespace.

## Planned Milestones
- M1: Foundation and single-turn chat (`T01-T10`)
- M2: Session persistence (`T11-T16`)
- M3: Slash completion core (`T17-T21`)
- M4: Guarded tool execution (`T22-T25`)
- M5: Context management (`T26-T32`)
- M6: Full command set (`T33-T37`)
- M7: Packaging and CI (`T38-T40`)

## Suggested Next Step
Start with:
- `tasks/T01.md`

Then proceed sequentially while keeping `Plans.md` and `Tests.md` updated.
