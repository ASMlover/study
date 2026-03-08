# Maple Lexical Specification (Draft v0.1)

Status: Draft (T14 planning deliverable).

This document defines lexical tokens, literals, comments, and reserved words for Maple.
This is a normative draft target for parser/runtime convergence.

## 1. Source Text and Encoding

1. Source files are UTF-8 encoded text.
2. Newline is `\n`; tools may accept `\r\n` and normalize internally.
3. Lexing is deterministic and left-to-right.

## 2. Whitespace and Comments

### 2.1 Whitespace

The following are ignored except as token separators:

1. Space (`U+0020`)
2. Horizontal tab (`\t`)
3. Carriage return (`\r`)
4. Line feed (`\n`)

Line counters increment on newline.

### 2.2 Comments

Maple supports line comments:

1. `//` starts a comment and consumes characters to end of line.

Block comments are not in v0.1 baseline.

## 3. Tokens

### 3.1 Single-Character Punctuation

1. `(`
2. `)`
3. `{`
4. `}`
5. `,`
6. `.`
7. `;`
8. `+`
9. `-`
10. `*`
11. `/`
12. `<`
13. `>`
14. `=`
15. `!`

### 3.2 Multi-Character Operators (planned in grammar baseline)

1. `==`
2. `!=`
3. `<=`
4. `>=`

Implementations that do not yet support these operators must report lex/parse errors consistently.

## 4. Literals

### 4.1 Number Literal

Number lexical forms:

1. Integer: `0`, `1`, `42`
2. Fractional: `3.14`, `0.5`

Scientific notation is not part of v0.1 baseline unless explicitly added by later spec revision.

### 4.2 String Literal

Strings are delimited by double quotes:

1. `"hello"`
2. `"line"`

Unterminated string literals are lexical errors.

Escape-sequence policy:

1. v0.1 baseline treats escapes as TBD unless implemented and documented in a follow-up revision.
2. Parsers may initially support raw character sequences inside quotes.

### 4.3 Boolean and Nil Literals

1. `true`
2. `false`
3. `nil`

## 5. Identifiers

Identifier lexical rule:

1. First character: ASCII letter (`A-Z`, `a-z`) or underscore (`_`)
2. Remaining characters: ASCII letter, digit (`0-9`), or underscore

Examples:

1. `name`
2. `_tmp`
3. `snake_case_2`

## 6. Keywords and Reserved Words

### 6.1 Core Keywords (v0.1)

1. `var`
2. `fun`
3. `return`
4. `class`
5. `this`
6. `super`
7. `print`
8. `import`
9. `from`
10. `as`
11. `true`
12. `false`
13. `nil`

### 6.2 Planned Control-Flow Keywords (grammar target)

1. `if`
2. `else`
3. `while`
4. `for`
5. `and`
6. `or`

If not yet implemented, they are reserved for forward compatibility and should not be repurposed as identifiers.

## 7. Tokenization Principles

1. Maximal munch applies for multi-character operators (for example `==` over `=` + `=`).
2. Lexical errors produce error tokens with source line information.
3. Lexing should continue after recoverable lexical errors when possible, to surface multiple diagnostics.

## 8. Lexical Errors (Baseline)

Examples:

1. Unexpected character not in lexical set.
2. Unterminated string.
3. Invalid numeric form (if parser/lexer chooses strict validation).

Minimum diagnostic information:

1. phase: `lex`
2. location: line (column planned)
3. human-readable message

## 9. Conformance Notes

Lexical conformance tests should include:

1. keyword recognition
2. identifier boundary cases
3. string and number happy/negative paths
4. comment stripping behavior
5. line tracking correctness
