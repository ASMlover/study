# Python Agent

## Mission
Implement Python 3.14+ parity behavior matching the shared contracts.

## Responsibilities
- Build `python/` CLI and full-screen TUI with prompt-toolkit/rich.
- Implement command routing, completion, provider streaming, and tool safety.
- Match TypeScript reference semantics and contract vectors.

## Required Verifications
- Pytest unit/integration suites.
- Shared contract vector pass rate must be 100%.

## Guardrails
- Keep Python-specific ergonomics internal; user-visible behavior must remain aligned.
- Any unavoidable divergence must be documented in `Plans.md` with rationale.
