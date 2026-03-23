# Diagnostics Golden Normalization Rules (v0.2)

Status: Implemented.

These rules normalize diagnostics before golden comparison.

## 1. Path Normalization

1. convert `\` to `/`
2. compare normalized relative file identity

## 2. Text Normalization

1. normalize textual fields to `\n`
2. trim trailing carriage returns

## 3. Field Normalization

1. normalize `phase` to lowercase
2. normalize `code` to uppercase `MSxxxx`
3. coerce integer-like line/column/length to integers

## 4. Optional Field Policy

1. `span.column` is optional unless provided in golden
2. `span.length` is optional unless provided in golden

## 5. Ordering

1. deterministic first-diagnostic comparison is required for single-error fixtures
2. when multi-diagnostic fixtures are added, ordering must be source-order deterministic

## 6. Message Policy

1. message text is not a required match key
2. message changes do not fail tests when `phase + code + span` still match
