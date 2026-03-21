# Newline-as-Statement-Terminator Design

> Version: 1.0 | Date: 2026-03-21

---

## 1. Goal

Replace mandatory `;` with newline as statement terminator. Retain `;` for multiple statements on one line.

```maple
// Before                          // After
var x = 10;                        var x = 10
print x;                           print x
var a = 1; var b = 2;              var a = 1; var b = 2
```

## 2. Approach: Scanner-Level ASI (Go-Style)

Scanner inserts synthetic `TOKEN_SEMICOLON` at newlines; compiler unchanged.

| Approach | Verdict |
|----------|---------|
| **Go-style ASI** | Scanner-only, deterministic, fits Pratt parser naturally |
| Python-style NEWLINE | Requires indent-based blocks, massive compiler rewrite |
| JS-style ASI | Ambiguous, error-prone |

## 3. ASI Rules

### 3.1 Core Rule

> On `\n`, if the **previous token** is in the ASI-trigger set, emit a synthetic `TOKEN_SEMICOLON`.

### 3.2 ASI-Trigger Set (insert `;` after newline)

| Category | Tokens |
|----------|--------|
| Literals | `NUMBER`, `STRING`, `TRUE`, `FALSE`, `NIL` |
| Identifiers | `IDENTIFIER` |
| Terminator keywords | `BREAK`, `CONTINUE`, `RETURN` |
| Closers | `)`, `]`, `}` |
| Postfix (if added) | `++`, `--` |

### 3.3 Non-Trigger Set (no insert, natural continuation)

| Category | Tokens |
|----------|--------|
| Binary/assignment ops | `+` `-` `*` `/` `%` `=` `==` `!=` `<` `>` `<=` `>=` `+=` `-=` `*=` `/=` `and` `or` |
| Openers | `(` `[` `{` |
| Punctuation | `,` `.` `:` `?` |
| Non-terminal keywords | `var` `fun` `class` `if` `else` `while` `for` `print` `import` `from` `as` `try` `catch` `finally` `defer` `switch` `case` `default` `throw` `in` |

### 3.4 Line Continuation

`\` + `\n` suppresses ASI:

```maple
var result = compute() \
  + offset
```

## 4. Special Cases

### 4.1 `return` / `break` / `continue`

ASI-triggers — bare on a line inserts `;` (return value must be on same line):

```maple
return         // → return;  (nil)
return 42      // → return 42;
```

### 4.2 Control Flow Paren Suppression

`if`/`while`/`for`/`switch`/`catch` condition `)` must not trigger ASI. Scanner maintains `paren_suppress_depth_` counter; ASI suppressed while depth > 0.

```maple
if (x > 0)
  print x       // no `;` after `)`
```

### 4.3 Block Braces & REPL

- `{` non-trigger, `}` trigger (spurious `;` harmless)
- REPL: end-of-input treated as `\n`; non-trigger at line end prompts continuation

## 5. Grammar Update

```ebnf
TERM           → ";" | <<ASI>>        // explicit or scanner-inserted semicolon
varDecl        → "var" IDENTIFIER ( "=" expression )? TERM
exprStmt       → expression TERM
printStmt      → "print" expression TERM
returnStmt     → "return" expression? TERM
```

Grammar rules unchanged — `TERM` replaces `";"`.

## 6. Risk Analysis

| Risk | Mitigation |
|------|------------|
| `return` + newline silently returns nil | Same as Go; document clearly |
| `if (cond)` breaks across newline | Paren-suppression counter |
| Existing tests fail | `;` remains valid; fully backward-compatible |
| Spurious `;` after `}` | Harmless; parser already handles it |

---

## Progress Tracking

| Phase | Task | Description | Status |
|-------|------|-------------|--------|
| **1 — Core ASI** | 1.1 | Scanner infra: add `prev_type_`, `is_asi_trigger()`; no behavior change | [ ] |
| | 1.2 | Basic ASI: `skip_whitespace()` detects `\n` + trigger → `pending_asi_` → synthetic `;` | [ ] |
| | 1.3 | Test `tests/newline_basic.ms`: no-`;` statements, same-line `;` split, blank lines | [ ] |
| **2 — Multi-line** | 2.1 | Test `tests/newline_multiline.ms`: operator / comma / chained-call / bracket continuation | [ ] |
| | 2.2 | `\` line continuation: `skip_whitespace()` detects `\` + `\n` → skip, no ASI flag | [ ] |
| **3 — Control Flow** | 3.1 | Paren suppression: `paren_suppress_depth_` counter, suppress ASI while depth > 0 | [ ] |
| | 3.2 | Test `tests/newline_control_flow.ms`: nested / `if-else` / `for`-in-`if` scenarios | [ ] |
| **4 — Keywords & Edges** | 4.1 | Test `return`/`break`/`continue`: bare line vs same-line value | [ ] |
| | 4.2 | Test block / class / function: `}` ASI, cross-line declarations | [ ] |
| **5 — REPL & Wrap-up** | 5.1 | REPL ASI: insert `;` at EOF per trigger rules; prompt continuation on non-trigger | [ ] |
| | 5.2 | Full regression: all existing `tests/*.ms` (with `;`) pass `ctest` 100% | [ ] |
| | 5.3 | Doc update: `REQUIREMENTS.md` grammar + `CLAUDE.md` examples | [ ] |
