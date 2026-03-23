# Maple Error and Diagnostics Specification (Spec v0.2)

Status: Implemented baseline with v0.2 stability tiers (updated on 2026-03-24).

This document defines Maple's error taxonomy and diagnostic contract.
The purpose is stable behavior across platforms and implementations, verified by diagnostics and stdlib error-path tests.

## 0. Stability Tiers (v0.2)

Tier definitions:

1. `stable`: compatibility-locked in v0.2.
2. `provisional`: accepted in v0.2 but may be normalized in v0.3 with migration notes.

| Contract Area | Tier | Notes |
|---|---|---|
| `phase` families (`lex/parse/resolve/runtime/module`) | stable | Primary compatibility key. |
| Code ranges (`MS1xxx` to `MS5xxx`) | stable | Family ownership is fixed in v0.2. |
| Existing baseline codes (`MS3001~MS3004`, `MS4001~MS4005`, `MS5001~MS5004`) | stable | Used by conformance/integration suites. |
| Parse invalid-assignment specificity (`MS2003`) | provisional | Canonical target is `MS2003`; temporary normalization alias from `MS2001` is allowed. |
| Human message wording | provisional | Keep intent stable; test anchor remains `phase + code`. |

## 0.1 Compatibility Notes

Tier mapping policy for normative clauses in this document:

1. Default rule: all numbered normative clauses are `stable` unless explicitly marked `provisional`.
2. `provisional` clauses in this version:
   - Section 5.2, transitional normalization alias from `MS2001` to canonical `MS2003`.
   - Section 3, `span.column` and `span.length` fields where implementation is still in planned state.
   - Message wording precision (meaning must remain stable, exact phrasing may evolve).
3. Tests must anchor on `phase + code`; message text checks are secondary.

## 1. Error Phases

Every error belongs to exactly one phase:

1. `lex` (tokenization)
2. `parse` (grammar construction)
3. `resolve` (static semantic checks)
4. `runtime` (evaluation/execution)
5. `module` (module load/resolve contract)

## 2. Error Codes

### 2.1 Code Format

Diagnostic code format:

1. `MS` + four digits
2. Example: `MS1001`

### 2.2 Reserved Ranges

1. `MS1xxx`: lexical errors
2. `MS2xxx`: parse errors
3. `MS3xxx`: resolve/static semantic errors
4. `MS4xxx`: runtime errors
5. `MS5xxx`: module errors

## 3. Diagnostic Data Model

Each diagnostic must expose:

1. `phase`
2. `code`
3. `message`
4. `span`:
   - `file` (optional for REPL input)
   - `line` (1-based)
   - `column` (1-based; provisional while planned if not yet implemented)
   - `length` (token/lexeme span; provisional while planned if not yet implemented)
5. `notes` (optional array of supplemental hints)

Recommended JSON shape:

```json
{
  "phase": "runtime",
  "code": "MS4002",
  "message": "expected 2 arguments but got 1",
  "span": { "file": "main.ms", "line": 3, "column": 7, "length": 5 },
  "notes": []
}
```

## 4. Message Stability Policy

Stability requirements:

1. `phase` and `code` are normative compatibility keys.
2. `message` text should remain stable; minor wording can change only with release note entry.
3. Tests should prefer matching by `phase + code`; message can be secondary match.

## 5. Baseline Error Catalog (v0.2)

### 5.1 Lex Errors (`MS1xxx`)

1. `MS1001`: unexpected character
2. `MS1002`: unterminated string literal

### 5.2 Parse Errors (`MS2xxx`)

1. `MS2001`: expected expression
2. `MS2002`: expected token (generic)
3. `MS2003`: invalid assignment target (canonical target; normalization alias may map from `MS2001` in transitional paths)

### 5.3 Resolve Errors (`MS3xxx`)

1. `MS3001`: return outside function
2. `MS3002`: this used outside class method context
3. `MS3003`: super used outside subclass method context
4. `MS3004`: class cannot inherit from itself

### 5.4 Runtime Errors (`MS4xxx`)

1. `MS4001`: undefined variable
2. `MS4002`: arity mismatch
3. `MS4003`: invalid operand types / runtime type contract violation
4. `MS4004`: undefined property
5. `MS4005`: non-callable value invocation

### 5.5 Module Errors (`MS5xxx`)

1. `MS5001`: module not found
2. `MS5002`: symbol not found in module exports
3. `MS5003`: circular module dependency
4. `MS5004`: module initialization failed

## 6. Error Propagation Rules

1. Lex/parse errors prevent execution and produce compile-failure result.
2. Resolve errors prevent execution and produce compile-failure result.
3. Runtime/module errors terminate current execution path with runtime-failure result.
4. Module load failure should include module context in message or notes.

## 7. Recovery Expectations

### 7.1 Parser Recovery

Parser should attempt synchronization after statement/declaration boundaries to report multiple syntax errors.

### 7.2 Runtime Recovery

Runtime errors are terminal for the current script/module execution context unless REPL defines per-line isolation policy.

## 8. Conformance and Tooling

Diagnostics conformance tests should verify:

1. phase classification correctness
2. code correctness
3. line accuracy (column when available)
4. deterministic behavior across Windows/Linux

Golden tests should store normalized records.

Suggested normalization:

1. path separator normalization
2. absolute path stripping
3. optional column field tolerance during migration period
4. code normalization and phase fallback behavior per `docs/spec/diagnostics-normalization-v0.2.md`

## 9. Versioning Rules

1. Adding a new error code is backward-compatible.
2. Reassigning existing code meaning is breaking.
3. Removing a code is breaking.
4. Breaking changes require spec version bump and migration notes.




