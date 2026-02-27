# MiniCLI4 Plans

Status legend: `pending`, `in_progress`, `blocked`, `done`

## M1 - Repo Bootstrap and Governance

### T01 - Initialize governance docs
- Status: `done`
- OwnerAgent: `architect-agent`
- DependsOn: none
- Definition of Done:
  - `AGENTS.md` exists with project scope, safety, and execution contract.
  - `subagents/` role docs exist and define boundaries.
- Test Cases:
  - Manual: files exist and are readable.
  - Manual: role responsibilities do not overlap ambiguously.

### T02 - Initialize planning and tracking
- Status: `done`
- OwnerAgent: `architect-agent`
- DependsOn: `T01`
- Definition of Done:
  - `Plans.md` contains milestones, task IDs, and dependency links.
  - Each task has status, owner, DoD, and test cases.
- Test Cases:
  - Manual: milestone flow is topologically executable.

## M2 - Shared Spec Contracts

### T03 - Define config contract
- Status: `done`
- OwnerAgent: `provider-agent`
- DependsOn: `T02`
- Definition of Done:
  - Add `spec/config.schema.json`.
  - Fields include local project config and glm-5 lock.
- Test Cases:
  - Schema validation for valid and invalid config examples.

### T04 - Define command contract and grammar
- Status: `done`
- OwnerAgent: `commands-completion-agent`
- DependsOn: `T03`
- Definition of Done:
  - Add `spec/commands.schema.json` with 30-command baseline.
  - Include usage, subcommands, aliases, and permission tags.
- Test Cases:
  - Validate all command entries against schema.

### T05 - Define events and tool-calling contract
- Status: `done`
- OwnerAgent: `provider-agent`
- DependsOn: `T04`
- Definition of Done:
  - Add `spec/events.schema.json` and `spec/tool-calls.schema.json`.
  - Cover streaming chunks, final messages, and tool-call arguments.
- Test Cases:
  - Validate canonical streaming and tool-call fixtures.

### T06 - Define completion vectors
- Status: `done`
- OwnerAgent: `commands-completion-agent`
- DependsOn: `T04`
- Definition of Done:
  - Add `spec/test-vectors/completion.*.json`.
  - Include command prefix, subcommand, dynamic arg, and path completion cases.
- Test Cases:
  - Vector runner asserts exact candidate sets and ordering.

## M3 - TypeScript Reference Implementation

### T07 - Scaffold TypeScript workspace
- Status: `done`
- OwnerAgent: `typescript-agent`
- DependsOn: `T06`
- Definition of Done:
  - `typescript/` project builds and runs CLI entry.
- Test Cases:
  - `npm run build`
  - `npm test`

### T08 - Implement provider streaming and retry policy
- Status: `done`
- OwnerAgent: `typescript-agent`
- DependsOn: `T07`, `T05`
- Definition of Done:
  - GLM-5-only provider with SSE streaming and error mapping.
- Test Cases:
  - Mock provider tests for 401/429/5xx/timeout.

### T09 - Implement agent orchestration and tool loop
- Status: `done`
- OwnerAgent: `typescript-agent`
- DependsOn: `T08`
- Definition of Done:
  - Multi-agent supervisor loop with tool-call iterations and stop conditions.
- Test Cases:
  - Integration test with mocked tool-calling sequence.

### T10 - Implement slash commands and completion
- Status: `done`
- OwnerAgent: `typescript-agent`
- DependsOn: `T09`, `T04`, `T06`
- Definition of Done:
  - Full command baseline implemented with contextual tab completion.
- Test Cases:
  - Contract and completion vector tests pass.

### T11 - Implement full-screen two-pane TUI
- Status: `done`
- OwnerAgent: `tui-agent`
- DependsOn: `T10`
- Definition of Done:
  - Left chat pane, right status pane, bottom input and keybindings.
- Test Cases:
  - Interaction tests for Tab/Shift+Tab/Ctrl+C/Ctrl+L/Esc/F1.

## M4 - Python 3.14+ Parity Implementation

### T12 - Scaffold Python workspace
- Status: `done`
- OwnerAgent: `python-agent`
- DependsOn: `T11`
- Definition of Done:
  - `python/` package with CLI entrypoint and test harness.
- Test Cases:
  - `pytest`
- Verification:
  - Implemented `python/pyproject.toml` and `python/src/minicli4_py` package with CLI entrypoint (`minicli4-py`).
  - `pytest` passed in `python/` workspace (see `Tests.md`).

### T13 - Implement provider, orchestration, and commands parity
- Status: `done`
- OwnerAgent: `python-agent`
- DependsOn: `T12`, `T03`, `T04`, `T05`, `T06`
- Definition of Done:
  - Python behavior aligns with TypeScript reference and contracts.
- Test Cases:
  - Shared vector suite + integration tests pass.
- Verification:
  - Added Python parity modules for provider, orchestration, slash commands, completion, and tooling.
  - Added tests for completion vectors from `spec/test-vectors/completion.*.json`, command behavior, provider mapping, agent loop, and CLI integration.
  - `pytest` passed in `python/` workspace (see `Tests.md`).

### T14 - Implement Python full-screen TUI parity
- Status: `done`
- OwnerAgent: `tui-agent`
- DependsOn: `T13`
- Definition of Done:
  - Prompt-toolkit/rich based TUI matches interaction contract.
- Test Cases:
  - Key handling and render-flow integration tests.
- Verification:
  - Implemented append-only Python TUI parity surface (status line, thinking indicator, stream lifecycle, and explicit status printing).
  - Added unit tests validating render flow and status update semantics.
- Decision Note:
  - 2026-02-26: Delivered TUI interaction parity using a stdlib TTY renderer to keep Python workspace dependency-light; prompt-toolkit/rich wiring can be layered without changing command/runtime contracts.

## M5 - C++ Parity Implementation

### T15 - Scaffold C++ workspace and dependencies
- Status: `done`
- OwnerAgent: `cpp-agent`
- DependsOn: `T11`
- Definition of Done:
  - `cpp/` builds on Windows and Linux using CMake.
- Test Cases:
  - Debug build + smoke execution.
- Verification:
  - Implemented new `cpp/` workspace with CMake-based library, CLI entrypoint (`minicli4-cpp`), and unit/integration-style test harness.
  - Confirmed build generation and Debug compilation from project root using:
    - `cmake -S cpp -B build/cpp`
    - `cmake --build build/cpp --config Debug`
  - Confirmed smoke CLI execution:
    - `build\\cpp\\Debug\\minicli4-cpp.exe --version`

### T16 - Implement provider, orchestration, commands, completion parity
- Status: `done`
- OwnerAgent: `cpp-agent`
- DependsOn: `T15`, `T03`, `T04`, `T05`, `T06`
- Definition of Done:
  - C++ feature parity with shared contracts and TS reference behavior.
- Test Cases:
  - Shared vectors + integration tests pass.
- Verification:
  - Added C++ parity modules for runtime config, session lifecycle, tool registry safety gates, GLM provider envelope + SSE chunk parser, multi-stage agent orchestration, slash commands, and completion.
  - Implemented and validated 30-command slash baseline with command semantics aligned to TypeScript/Python contracts.
  - Added completion coverage for command prefixes, subcommands, dynamic arguments, and path candidates aligned to vectors in `spec/test-vectors/completion.*.json`.
  - Added orchestration/provider tests covering staged agent execution, tool callback flow, and streamed delta parsing behavior.

### T17 - Implement C++ full-screen TUI parity
- Status: `done`
- OwnerAgent: `tui-agent`
- DependsOn: `T16`
- Definition of Done:
  - FTXUI two-pane TUI with standard keybindings and streaming rendering.
- Test Cases:
  - Interaction and state transition tests.
- Verification:
  - Implemented C++ append-only two-pane TUI parity surface with status line, thinking lifecycle, assistant streaming output, and explicit status rendering hooks.
  - Added C++ tests validating append-only render flow, input/status lifecycle, and streamed assistant chunk concatenation behavior.
  - Verified test suite pass:
    - `ctest --test-dir build/cpp -C Debug --output-on-failure`
- Decision Note:
  - 2026-02-27: Delivered M5 TUI parity with a lightweight ANSI append-only renderer in C++ to keep dependencies minimal in initial parity pass; explicit FTXUI integration can be layered later without changing command or orchestration contracts.

## M6 - Cross-language QA and Release

### T18 - Build shared contract test runner
- Status: `done`
- OwnerAgent: `qa-release-agent`
- DependsOn: `T14`, `T17`
- Definition of Done:
  - Single command runs contract vectors against all three implementations.
- Test Cases:
  - Runner returns pass/fail per language and aggregate summary.
- Verification:
  - Added root cross-language runner `scripts/run_contracts.py`.
  - Runner executes shared completion vector contract checks for TypeScript and Python implementations, C++ ctest contract coverage, plus CLI smoke checks.
  - Runner prints pass/fail per language and aggregate summary, and can emit machine-readable report JSON with `--json-report`.
  - Local verification command:
    - `python scripts/run_contracts.py --json-report .minicli4/logs/contracts-local.json`
  - Local result:
    - `typescript: PASS`
    - `python: PASS`
    - `cpp: PASS`
    - `aggregate: PASS`

### T19 - Add CI matrix and release checks
- Status: `done`
- OwnerAgent: `qa-release-agent`
- DependsOn: `T18`
- Definition of Done:
  - Windows/Linux CI covers build, test, vectors, and smoke artifacts.
- Test Cases:
  - CI dry-run scripts pass locally where supported.
- Verification:
  - Added GitHub Actions matrix workflow at `.github/workflows/ci.yml` for `ubuntu-latest` and `windows-latest`.
  - Workflow sets up Node 22 and Python 3.14, installs TypeScript dependencies, and runs:
    - `python scripts/run_contracts.py --json-report .minicli4/logs/contracts-${{ matrix.os }}.json`
  - Workflow uploads:
    - Contract summary report artifact per OS.
    - TypeScript build artifact per OS.
    - C++ smoke binary artifact per OS.
  - Local CI-dry-run equivalent verification (where supported):
    - `python scripts/run_contracts.py --json-report .minicli4/logs/contracts-local.json` -> aggregate `PASS`.

### T20 - Milestone closeout and release notes
- Status: `done`
- OwnerAgent: `qa-release-agent`
- DependsOn: `T19`
- Definition of Done:
  - Release notes summarize parity status, known risks, and artifact checks.
- Test Cases:
  - Manual sign-off checklist completed.
- Verification:
  - Added milestone release notes: `release-notes/M6.md`.
  - Included parity summary, known risks, CI artifact checks, and sign-off checklist with completion status.

## Decision Notes
- 2026-02-25: M3 completed with TypeScript scaffold, provider streaming, multi-agent orchestration, 30-command baseline, completion, and two-pane TUI implementation.
- 2026-02-25: M2 completed with schema contracts and completion vectors under spec/.
- 2026-02-25: Repo uses root-level `subagents/` with 8 role files.
- 2026-02-25: Plan tracking uses milestone + task IDs.
- 2026-02-25: Strict cross-language command parity remains a hard requirement.
- 2026-02-25: TypeScript TUI iteration moved to append-only, colorized semantic labels (`[YOU]/[AI]/[SYS]`), and streaming markdown rendering for product-style CLI output.
- 2026-02-25: TypeScript TUI added light/dark theme detection (`MINICLI4_THEME`/`COLORFGBG`), markdown table+code-line rendering, and product-style cards for `/status` and `/doctor`.
- 2026-02-25: TypeScript config now supports `theme` via `/config set theme <dark|light>`, AI/system output prefixes were removed for cleaner product presentation, and thinking animation was refined to concise professional motion text.
- 2026-02-25: TypeScript TUI thinking spinner upgraded to icon-forward branded animation (`MiniCLI4` logo frames + stage icons + aura pulse) instead of plain character-only frame cycling.
- 2026-02-25: `/context` now reports estimated context usage percentage (`context_usage~`) based on estimated tokens vs `max_tokens`, enabling proactive archive/clear decisions.
- 2026-02-26: M4 completed with new `python/` implementation and test harness, completion vector parity checks against `spec/test-vectors`, provider/agent/command parity modules, and Python TUI interaction tests.
- 2026-02-27: M5 completed with new `cpp/` implementation (CMake scaffold, command/completion/provider/agent parity modules, and append-only TUI parity surface) plus passing C++ build/test/smoke verification.
- 2026-02-27: C++ TUI now includes explicit optional FTXUI backend wiring (`find_package(ftxui)` + runtime backend selection via `MINICLI4_TUI_BACKEND`), with ANSI append-only fallback preserved for environments without FTXUI.
- 2026-02-27: M6 shared QA runner uses shared completion vectors as contract baseline across TypeScript/Python and existing C++ ctest contract coverage, with cross-platform CI matrix and smoke artifact upload.




