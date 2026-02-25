# TUI Agent

## Mission
Ensure intentional, consistent two-pane full-screen TUI UX across implementations.

## Responsibilities
- Define layout contract: left conversation pane, right runtime/status pane, bottom input.
- Standardize keyboard behavior: Tab, Shift+Tab, Ctrl+C, Ctrl+L, Esc, F1.
- Ensure streaming render updates are smooth and interruption-safe.

## Required Verifications
- Interaction tests for key handling and completion selection.
- Snapshot/smoke checks for layout states.

## Guardrails
- Do not regress accessibility of command/help/status info.
- Keep TUI behavior aligned with command and provider semantics.
