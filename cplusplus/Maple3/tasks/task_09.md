# Task 09 - Full CLI: run / repl / test

## Objective
Deliver a complete command-line interface with script run, interactive REPL, and built-in test runner command.

## Scope
### In Scope
1. CLI command parsing and help text.
2. `run <file.ms>`, `repl`, and `test <path>` behaviors.
3. Exit code conventions and error surface.

### Out Of Scope
1. Advanced REPL editor features (multi-line editing, history persistence).
2. Debugger integration.

## Code Changes
1. `Main.cc` (or `Cli.hh` + `Cli.cc`)
2. `tests/CliTest.cc`
3. `tests/scripts/cli/cli_run_ok.ms`
4. `tests/scripts/cli/cli_run_error.ms`
5. `tests/expected/cli/cli_run_ok.out`
6. `tests/expected/cli/cli_run_error.out`
7. `tests/repl/repl_session_01.in`
8. `tests/repl/repl_session_01.out`

## Implementation Steps
1. Add CLI dispatcher with stable subcommands and usage docs.
2. Implement `run` path to parse/resolve/interpret target script.
3. Implement `repl` loop with line-by-line evaluation.
4. Implement `test` path that invokes script suite runner.
5. Add CLI tests for argument handling and exit statuses.

## Testcases
### Unit/Component (C++)
1. Argument parser recognizes valid subcommands.
2. Unknown subcommand prints help and non-zero exit.
3. REPL command processor handles empty and invalid lines.

### E2E Script
1. `Maple run tests/scripts/cli/cli_run_ok.ms` matches expected output.
2. `Maple test tests/scripts` returns success on green suite.

### Negative/Error
1. Missing script file in `run` reports error and non-zero exit code.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleCliTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. CLI supports `run`, `repl`, `test` reliably.
2. CLI tests and full suite pass.

## Post-Completion Update
1. Update `Todos.md` row 09: `Status=done`, set `Done Date`.
2. Add progress log entry with CLI command evidence and test status.
3. Set row 10 to `in_progress` only after global gate is green.

## Rollback Notes
1. If REPL path is unstable, keep `run` and `test` working and isolate REPL fixes before task closure.
