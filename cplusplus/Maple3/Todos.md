# Maple3 Todo Board

## Execution Rule
1. Execute exactly one task at a time via `imp tasks/task_<index>.md`.
2. A task is complete only when all listed commands pass and all listed testcases pass.
3. After a successful task, update this file immediately (`Status`, `Done Date`, `Progress Log`).
4. If a task fails, set status to `blocked` and document the blocker before stopping.

## Global Gate (must pass for every task)
1. `cmake -S . -B build`
2. `cmake --build build --config Debug`
3. `ctest --test-dir build --output-on-failure -C Debug`
4. `build/Debug/Maple.exe` remains runnable for the current milestone behavior.

## Task Table
| Index | Task | Status | DependsOn | Primary Targets | Primary Tests | Done Date |
|---|---|---|---|---|---|---|
| 01 | Testing and script regression framework | pending | - | `CMakeLists.txt`, `tests/ScriptRunner*`, `tests/scripts/smoke/*` | `MapleScriptSuite`, `MapleScriptRunnerTest` | - |
| 02 | Parser statements and declarations | pending | 01 | `Ast.*`, `Parser.*` | `MapleParserTest`, `MapleParserStatementTest` | - |
| 03 | Resolver scope binding | pending | 02 | `Resolver.*` | `MapleResolverTest` | - |
| 04 | Runtime value and environment | pending | 03 | `Value.*`, `Environment.*` | `MapleValueTest`, `MapleEnvironmentTest` | - |
| 05 | Interpreter core execution | pending | 04 | `Interpreter.*` | `MapleInterpreterCoreTest`, `MapleScriptSuite` | - |
| 06 | Functions and closures | pending | 05 | `Ast.*`, `Parser.*`, `Interpreter.*`, `Resolver.*` | `MapleInterpreterFunctionTest`, `MapleScriptSuite` | - |
| 07 | Class/this/super | pending | 06 | `Ast.*`, `Parser.*`, `Interpreter.*`, `Resolver.*` | `MapleInterpreterClassTest`, `MapleScriptSuite` | - |
| 08 | Module import system | pending | 07 | `ModuleLoader.*`, `Parser.*`, `Interpreter.*` | `MapleModuleLoaderTest`, `MapleScriptSuite` | - |
| 09 | Full CLI: run/repl/test | pending | 08 | `Main.cc` or `Cli.*` | `MapleCliTest`, `MapleScriptSuite` | - |
| 10 | Full language regression pack | pending | 09 | `tests/scripts/language/*`, `tests/expected/language/*`, `tests/README.md` | `MapleScriptSuite` | - |

## Progress Log
- 2026-02-06: Initialized `Todos.md` and `tasks/task_01.md` ... `tasks/task_10.md` execution playbook.
