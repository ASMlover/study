# Diagnostics Golden Normalization Rules (Draft v0.1)

Status: Draft.

These rules normalize diagnostics before golden comparison.

## 1. Path Normalization

1. convert `\` to `/`
2. strip machine-specific absolute prefixes when configured
3. preserve relative file identity where possible

## 2. Newline Normalization

1. normalize all textual fields to `\n`
2. trim trailing carriage returns

## 3. Field Normalization

1. ensure `phase` is lowercase
2. ensure `code` uppercase format `MSxxxx`
3. coerce integer-like line/column/length to integers

## 4. Optional Field Policy

When case policy allows:

1. missing `column` treated as wildcard
2. missing `length` treated as wildcard

Wildcard policy must be explicit in test metadata.

## 5. Ordering

1. sort diagnostics by `(line, column, code, message)` with missing column treated as `0`
2. preserve stable order for equivalent keys by original sequence

## 6. Message Comparison Modes

Supported modes:

1. `exact`: full string equality
2. `contains`: expected substring inclusion

Default:

1. `exact` for new structured diagnostics
2. `contains` only for approved migration cases
