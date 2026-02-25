# Subagents for MiniCLI4

This directory defines role-based execution for MiniCLI4.

## Orchestration Order
1. `architect-agent`: lock scope, architecture, interfaces, and dependencies for the target task.
2. `commands-completion-agent` and `provider-agent`: freeze command and provider contracts.
3. `typescript-agent`: implement reference behavior first.
4. `python-agent` and `cpp-agent`: implement parity behavior against the reference.
5. `tui-agent`: align full-screen two-pane TUI interaction and shortcuts across implementations.
6. `qa-release-agent`: run shared contract tests, integration tests, and release readiness checks.

## Collaboration Rules
- Each role must stay in scope. Cross-role edits require a short handoff note in task history.
- Shared behavior changes must be proposed to `architect-agent` and reflected in `Plans.md`.
- No language-specific shortcuts that break parity unless explicitly approved and documented.

## Handoff Template
- Task ID:
- Role:
- Inputs consumed:
- Decisions made:
- Outputs produced:
- Risks / follow-ups:
