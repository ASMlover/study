# MiniCLI4 Agent Collaboration Rules

MiniCLI4 is a cross-language AI agent CLI project inspired by Claude Code. This repository targets strict feature parity across TypeScript, Python 3.14+, and C++ implementations.

## Scope and Goals
- Re-implement from scratch with robust agent capabilities and TUI interaction.
- Support only `glm-5` as the model backend.
- Keep config and runtime state under project-local `.minicli4/`.
- Provide rich slash commands with tab completion.
- Support streaming AI responses.
- Deliver three implementations: TypeScript, Python 3.14+, and C++.

## Working Contract
- Read `Plans.md` before any implementation turn.
- Follow `subagents/README.md` orchestration and role boundaries.
- Keep behavior aligned with shared contracts in `spec/` (to be created by tasks).
- Prefer additive changes; do not silently change command semantics.

## Safety and Permissions
- Default policy is least privilege.
- Read/search tools may run by default.
- Write-file and shell-exec operations must pass explicit confirmation or allowlist checks.
- Never allow path traversal outside project root.
- Persist audit trails for privileged operations in `.minicli4/logs/audit.log`.

## Delivery Standards
- All three language implementations must pass shared contract tests.
- Command behavior, completion behavior, and streaming behavior must be parity-verified.
- Keep docs and plans current in every milestone closeout.

## Execution Trigger Convention
- `go Txx`: implement task `Txx` from `Plans.md` including tests.
- `go Mx`: execute milestone `Mx` task batch in dependency order.
- `review`: run code review mode focused on risk, regressions, and missing tests.

## Required Updates per Task
- Update task status in `Plans.md`.
- Record verification notes in test logs/doc sections.
- If task scope or assumptions changed, append a short decision note under the task.
