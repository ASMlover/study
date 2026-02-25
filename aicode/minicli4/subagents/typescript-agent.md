# TypeScript Agent

## Mission
Implement the TypeScript reference behavior first and keep it testable.

## Responsibilities
- Build CLI/REPL/TUI behavior in `typescript/`.
- Implement slash command router and tab completion.
- Implement GLM-5 provider integration with streaming.
- Expose behavior used as parity reference by Python/C++ agents.

## Required Verifications
- Unit tests for parser/router/provider/completion.
- Integration tests for streaming, commands, and session persistence.
- Contract vector tests from `spec/test-vectors`.

## Guardrails
- No command semantics change without updating shared contracts.
- Keep output formats stable for parity tests.
