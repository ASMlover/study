# Conformance Case Format (Draft v0.1)

Status: Draft.

This document defines how each conformance case should be represented.

## 1. File Convention

Recommended naming:

1. `<domain>_<topic>_<scenario>.ms`
2. Example: `semantics_closure_capture_basic.ms`

Each case file uses a metadata header in leading line comments.

## 2. Metadata Header

Header syntax:

```text
// @id: SEM-CLOSURE-001
// @spec: semantics.closure.capture
// @spec: semantics.scope.lexical
// @expect: ok
// @stdout:
// 11
// 12
```

For failing case:

```text
// @id: RUN-ARITY-001
// @spec: semantics.functions.arity
// @spec: errors.MS4002
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4002
// @diag.message_contains: expected 2 arguments but got 1
```

Required fields:

1. `@id`
2. `@spec` (at least one)
3. `@expect`

Conditional fields:

1. `@stdout` for `ok`
2. `@diag.phase` and `@diag.code` for failing cases
3. `@diag.message_contains` optional transitional matcher

## 3. Expectation Semantics

`@expect` allowed values:

1. `ok`
2. `compile_error`
3. `runtime_error`

Definition:

1. `ok`: execution completes and output matches.
2. `compile_error`: lex/parse/resolve failure before runtime execution.
3. `runtime_error`: runtime/module failure during execution.

## 4. Specification Reference Rules

`@spec` values should map to one of:

1. semantic clause id (`semantics.closure.capture`)
2. lexical clause id (`lexical.identifiers`)
3. grammar clause id (`grammar.assignment`)
4. error code (`errors.MS4002`)

Each `@spec` reference must appear in `MATRIX.md`.

## 5. Output and Diagnostic Matching

Output:

1. Compare exactly after newline normalization (`\r\n` -> `\n`).

Diagnostics:

1. Must match `phase + code`.
2. Message substring is optional during migration.
3. File path differences should not fail case if line and code are stable.

## 6. Example Cases

Example success:

```text
// @id: CLS-INHERIT-001
// @spec: semantics.class.inheritance
// @expect: ok
// @stdout:
// AB
class A {
  method() { return "A"; }
}
class B < A {
  method() { return "B"; }
  chain() { return super.method() + this.method(); }
}
print B().chain();
```

Example failure:

```text
// @id: CLS-SUPER-ERR-001
// @spec: semantics.class.super
// @spec: errors.MS4004
// @expect: runtime_error
// @diag.phase: runtime
// @diag.code: MS4004
// @diag.message_contains: undefined property
class A {}
class B < A {
  method() { return super.nope(); }
}
print B().method();
```
