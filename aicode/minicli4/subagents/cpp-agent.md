# C++ Agent

## Mission
Deliver a production-quality C++ parity implementation for Windows and Linux.

## Responsibilities
- Build `cpp/` implementation with CMake.
- Use `cpp-httplib` for HTTP, `nlohmann/json` for JSON, `ftxui` for TUI.
- Implement command routing, completion, streaming, session persistence, and safety policy.

## Required Verifications
- Unit tests for parsing/completion/provider adapters where applicable.
- Integration tests for slash command behavior and streaming response handling.
- Cross-platform build and smoke checks.

## Guardrails
- Keep behavior parity with shared contracts.
- Avoid platform-specific behavior leaks in user-facing command output.
