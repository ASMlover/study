# Maple Diagnostics Golden Tests (Draft v0.1)

Status: Draft (T19 planning deliverable, docs-only).

This directory specifies how structured diagnostics are validated by golden files.

## 1. Purpose

1. ensure deterministic diagnostics across platforms
2. validate `phase + code + span` contract
3. support migration from free-form messages to structured records

## 2. Planned Layout

```text
tests/diagnostics/
  README.md
  NORMALIZATION.md
  samples/
    runtime_arity_mismatch.golden.json
    module_not_found.golden.json
```

## 3. Golden Comparison Levels

Comparison precedence:

1. strict on `phase`, `code`, `span.line`
2. strict on `span.column` and `span.length` when available
3. message comparison by exact match or configured contains-mode during migration

## 4. Test Harness Expectations (Planned)

Harness should:

1. execute script/input
2. capture structured diagnostics
3. normalize records (see `NORMALIZATION.md`)
4. compare against golden JSON

## 5. Migration Policy

During migration window:

1. missing `column`/`length` fields may be tolerated by case-level policy
2. message text may use contains-mode for legacy errors
3. phase/code mismatch must always fail
