# Task 05 - AST and Expression Parser

## Status

Complete. Verified on 2026-03-29 with the acceptance commands in this document.

## Goal

Define the AST and make expression parsing stable before declaration and statement parsing is added.

## Design Links

- AST shape, expression grammar, and parser responsibilities: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 02
2. Task 04

## Scope

1. Define AST node kinds and arena allocation rules.
2. Parse primary, unary, binary, logical, assignment, call, property, and index
   expressions.
3. Support `fn` expressions, `self`, and `super.name` syntax.
4. Support `list`, `tuple`, and `map` literals in expression context.
5. Emit structured parse diagnostics in the `MS2xxx` family.

## Implementation Boundaries

1. This task owns expression parsing only.
2. Statement and declaration parsing stays in Task 06.
3. AST nodes are arena-allocated and never individually freed.
4. Parser output is syntax-only; no name binding or static legality checks belong here.

## File Ownership

1. `include/ms/ast.h`
2. `include/ms/parser.h`
3. optional `src/frontend/ast.c` helpers
4. `src/frontend/parser_expr.c` or equivalent expression parser split
5. parser-expression tests under `tests/unit/` and `tests/ms/parser_expr/`

## AST and Parser Contract

1. Every AST node must carry `kind`, `node_id`, `line`, `column`, and `end_column`.
2. Expression precedence, associativity, and assignment-target restrictions must
   be explicit and covered by tests.
3. Tuple vs grouping must be unambiguous: `(a)` is grouping, `(a,)` is tuple.
4. Map literals are valid only in expression context; block parsing stays in Task 06.
5. `==`, `!=`, property access, index access, and chained assignment must match
   the design grammar.
6. Export a reusable expression entrypoint such as `parse_expression(parser, min_prec)` so Task 06 does not duplicate grammar logic.

## TDD Plan

1. Start with operator-precedence, associativity, and assignment tests.
2. Add AST-shape snapshot tests for calls, indexing, and property access.
3. Add negative tests for malformed tuples, maps, invalid assignment targets,
   function expressions, and `super.name` syntax.
4. Add `.ms` parser fixtures with a normalized AST dump or equivalent stable
   representation.

## Acceptance

1. Expression precedence, associativity, and assignment grammar match the design
   baseline.
2. `self`, `super.name`, `fn (...) { ... }`, and container literals produce correct AST shapes.
3. Parse failures emit `phase=parse` diagnostics with `MS2xxx` codes.
4. Invalid assignment targets are rejected at parse time.
5. The task is not complete until build passes, tests pass, and all edited files
   are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "parser_expr|ast"
```

## Out of Scope

1. Statement and declaration parsing.
2. Resolver logic.
3. Bytecode generation.
