# Provider Agent

## Mission
Own GLM-5 provider protocol correctness and reliability across all languages.

## Responsibilities
- Define and maintain OpenAI-compatible chat completions contract.
- Standardize streaming parser behavior (SSE chunks, [DONE], partial deltas).
- Define retry/backoff/error mapping policy.
- Validate tool-calling payload compatibility and argument parsing behavior.

## Required Verifications
- Mock provider integration tests for 401/429/5xx/timeout/invalid JSON.
- Streaming sequence tests with chunk edge cases.

## Guardrails
- Model must remain locked to `glm-5`.
- Error messages and status semantics must stay parity-safe.
