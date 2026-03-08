# Maple Conformance Test Suite (Draft v0.1)

Status: Draft (T20 planning deliverable, docs-only).

This directory defines the conformance test contract for Maple.
It is intentionally spec-driven and runtime-path agnostic at the document level.

## 1. Purpose

Conformance tests validate language behavior against normative specification documents:

1. `docs/spec/lexical.md`
2. `docs/spec/grammar.ebnf`
3. `docs/spec/semantics.md`
4. `docs/spec/errors.md`

Unlike feature demos, conformance tests are traceable to explicit spec rules.

## 2. Directory Layout (Planned)

```text
tests/conformance/
  README.md
  CASE_FORMAT.md
  MATRIX.md
  lexical/
  grammar/
  semantics/
  diagnostics/
  modules/
```

Category purpose:

1. `lexical/`: tokenization and lexical error behavior.
2. `grammar/`: parse acceptance/rejection and precedence contracts.
3. `semantics/`: closures, classes, inheritance, runtime behavior.
4. `diagnostics/`: phase/code/span diagnostic structure.
5. `modules/`: import caching, symbol import, circular dependency errors.

## 3. Execution Model Policy

Conformance baseline follows ADR-001:

1. Normative engine is bytecode VM path.
2. During migration, interpreter parity checks may exist but are non-normative.
3. A case passes only when normative path behavior matches expectation.

## 4. Case Metadata Contract

Each case file should include a structured header block (see `CASE_FORMAT.md`):

1. `id`: stable case identifier
2. `spec`: spec clause reference(s)
3. `expect`: `ok | compile_error | runtime_error`
4. `stdout`: expected standard output (optional)
5. `diagnostic.phase`: expected phase (for failing cases)
6. `diagnostic.code`: expected error code (for failing cases)

## 5. Matching Strategy

Matching strictness:

1. Primary key: result class (`ok/compile_error/runtime_error`)
2. Secondary key: diagnostic `phase + code`
3. Tertiary key: message text (stable but allowed migration window)

Path and platform normalization:

1. normalize path separators
2. strip non-deterministic absolute prefixes
3. keep line numbers deterministic; tolerate missing columns during migration if declared

## 6. CI Gate Policy (Planned)

Conformance CI checks should include:

1. smoke subset on every PR
2. full suite on protected branch merges
3. mandatory pass for normative VM path

No new language feature should merge without at least:

1. one positive conformance case
2. one negative conformance case
3. matrix mapping entry in `MATRIX.md`

## 7. Migration Notes

Current repository contains integration tests under `tests/integration` and scripts under `tests/scripts`.

Migration plan:

1. preserve existing integration tests for regression safety
2. incrementally mirror them into conformance format
3. once mirrored, enforce matrix coverage and structured diagnostics
