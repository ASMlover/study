# Task 04 - Lexer

## Status

Complete. Verified on 2026-03-29 with the acceptance commands in this document.

## Goal

Implement the tokenization layer as the only input source for the parser.

## Design Links

- Surface syntax, keywords, and diagnostics phases: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 02

## Scope

1. Define token kinds, source slices, and source spans.
2. Tokenize keywords, identifiers, numbers, strings, operators, and delimiters.
3. Support all current keywords, including `fn`, `self`, `super`, `break`, and `continue`.
4. Emit `NEWLINE` and `EOF` tokens.
5. Emit structured lex diagnostics in the `MS1xxx` family.

## Implementation Boundaries

1. The lexer emits tokens and lex diagnostics only.
2. It must not build AST nodes or perform parse recovery.
3. Token text should stay as source slices rather than heap runtime strings.
4. If string escapes are supported in v1, keep the initial set small and fixed in this task. A safe default is `\\`, `\"`, `\n`, and `\t`. Any expansion should update [../mslangc-design.md](../mslangc-design.md) first.

## File Ownership

1. `include/ms/token.h`
2. `include/ms/lexer.h`
3. `src/frontend/lexer.c`
4. lexer unit and golden tests under `tests/unit/` and `tests/ms/lexer/`

## Token and Error Contract

1. Every token records line, column, and end-column data.
2. `NEWLINE` emission must be deterministic and testable.
3. Unterminated strings and invalid characters must emit `phase=lex` diagnostics with `MS1xxx` codes.
4. The keyword table must match the current language design exactly.

## TDD Plan

1. Start with token-sequence tests from small `.ms` fixtures.
2. Add negative tests for unterminated strings and invalid characters.
3. Add targeted tests for newline handling and keyword recognition.
4. Keep token-stream snapshots compact and normalized so parser tasks can reuse them.

## Acceptance

1. The lexer recognizes the full keyword set and basic punctuation from the design doc.
2. `.ms` lexer fixtures and C tests pass under `ctest`.
3. Lex diagnostics are stable enough to support later golden testing.
4. The task is not complete until build passes, tests pass, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "lexer|lex"
```

## Out of Scope

1. AST construction.
2. Semantic checks.
3. Runtime string objects.