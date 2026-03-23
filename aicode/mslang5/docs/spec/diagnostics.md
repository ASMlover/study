# Maple Diagnostics Schema (Spec v0.2)

Status: Implemented baseline schema with v0.2 matching policy (updated on 2026-03-24).

This document standardizes structured diagnostics for Maple.
It complements `docs/spec/errors.md` with concrete schema and matcher behavior.

## 1. Canonical Diagnostic Record

Each diagnostic record has:

1. `phase`: `lex | parse | resolve | runtime | module`
2. `code`: `MSxxxx`
3. `message`: human-readable detail (non-normative for compatibility)
4. `span`:
   - `file` (optional for REPL)
   - `line` (1-based)
   - `column` (1-based; optional during migration)
   - `length` (optional during migration)
5. `notes`: optional list of related hints/causes

## 2. JSON Schema (Informal)

```json
{
  "phase": "parse",
  "code": "MS2001",
  "message": "expected expression",
  "span": {
    "file": "script.ms",
    "line": 4,
    "column": 12,
    "length": 1
  },
  "notes": [
    "while parsing assignment target"
  ]
}
```

## 3. Field Requirements

Required now:

1. `phase`
2. `code`
3. `span.line`

Migration-optional:

1. `message`
2. `span.column`
3. `span.length`

## 4. Compatibility Contract

Compatibility keys:

1. `phase + code` are normative and must remain stable.
2. `span.line` is required for deterministic matching.
3. `message` is informational and must not be a required golden-match key.
4. additional `notes` are backward-compatible.

Breaking changes:

1. changing meaning of existing code
2. removing required fields
3. changing phase classification for the same condition without migration path

Diagnostics normalization map:

1. canonical and transitional mapping rules are defined in `docs/spec/diagnostics-normalization-v0.2.md`.

## 5. Multi-Diagnostic Ordering

When multiple diagnostics are emitted:

1. sort by source order (`line`, `column`)
2. stable tie-break by discovery order
3. deterministic output across platforms

## 6. Test Matcher Contract

Diagnostics tests should match in this order:

1. `phase` (strict)
2. `code` (strict)
3. `span.file` and `span.line` (strict after path normalization)
4. `span.column` and `span.length` (strict only when expected values are provided)
5. `message` must not be required by default

## 7. Mapping to Exit Status

1. any `lex|parse|resolve` diagnostic => compile error exit category
2. any `runtime|module` diagnostic during execution => runtime error exit category

## 8. Recommended Human Rendering

Text rendering should include:

1. `[phase code] message`
2. `file:line[:column]`
3. optional notes as indented follow-ups

Example:

```text
[runtime MS4002] expected 2 arguments but got 1
script.ms:3
note: function add declared with arity 2
```
