# Maple Diagnostics Golden Tests (v0.2)

Status: Implemented.

This directory defines how structured diagnostics are validated by golden files.

## 1. Purpose

1. ensure deterministic diagnostics across platforms
2. validate the `phase + code + normalized span` contract
3. keep golden matching independent from free-form message wording

## 2. Layout

```text
tests/diagnostics/
  README.md
  NORMALIZATION.md
  samples/
    parse_expected_expression.golden.json
    resolve_return_outside_function.golden.json
    runtime_arity_mismatch.golden.json
    module_not_found.golden.json
```

## 3. Golden Comparison Levels

Comparison precedence:

1. strict on `phase` and `code`
2. strict on `span.file` and `span.line` after path normalization
3. strict on `span.column` and `span.length` only when expected values are present
4. message text is informational and non-blocking

## 4. Test Harness Behavior

Harness must:

1. execute script input
2. capture structured diagnostics
3. normalize records (see `NORMALIZATION.md`)
4. compare against golden JSON using phase/code/span contract

## 5. Fixture Coverage

Current golden fixtures cover:

1. parse failure (`MS2001`)
2. resolve failure (`MS3001`)
3. runtime failure (`MS4002`)
4. module failure (`MS5001`)
