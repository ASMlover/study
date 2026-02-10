# Task Execution Checklist

## Purpose
- Convert `AGENTS.md` workflow rules into a repeatable, executable checklist.
- Use this checklist for every task round.

## Round Start Checklist
- [ ] Read `Plans.md` before touching code.
- [ ] Confirm target task file (for example: `tasks/T01.md`).
- [ ] If input is `go tasks/TXX.md`, use `tasks/TXX.md` as the single source of truth for scope.

## Development Checklist
- [ ] Implement the feature scope defined in `tasks/TXX.md`.
- [ ] Implement the corresponding test cases defined in `tasks/TXX.md`.
- [ ] Ensure all code files use UTF-8 encoding.
- [ ] Ensure all code files use LF line endings.
- [ ] Ensure trailing whitespace is removed from all edited code files.

## Validation Checklist
- [ ] Run unit tests for the changed scope.
- [ ] Run integration tests for the changed scope.
- [ ] Run E2E/smoke tests if required by `tasks/TXX.md`.
- [ ] Verify the task acceptance criteria in `tasks/TXX.md` are met.

## Round End Checklist
- [ ] Update `Plans.md` with current task status, dates, and notes.
- [ ] If the task finished successfully, update `Tests.md` with:
- [ ] test methods used
- [ ] commands executed
- [ ] expected vs actual results
- [ ] coverage/result summary
- [ ] Record blockers (if any) in `Plans.md` and keep task status consistent.

## Quick Command Trigger Rule
- [ ] Input `go tasks/TXX.md` means:
- [ ] implement the feature in `tasks/TXX.md`
- [ ] implement and run its corresponding tests
- [ ] update `Plans.md` at round end
- [ ] update `Tests.md` if successful
