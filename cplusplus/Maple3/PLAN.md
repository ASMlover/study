# PLAN

## Current Focus

- Stabilize the interpreter frontend (scanner) and build a clear path into parsing and runtime execution.

## Milestones

1) Scanner hardening
- Add token span info (line/column) and stronger error reporting.
- Add scanner edge-case tests.

2) Parser + AST
- Implement AST nodes and a recursive-descent parser.
- Add parser tests for expressions and statements.

3) Runtime interpreter
- Define runtime values, environments, and evaluator.
- Add runtime tests for arithmetic, variables, and control flow.

4) CLI + diagnostics
- Add file execution and REPL entry points.
- Standardize error formatting.

## In Progress

- None.

## Done

- Scanner tokenization, block comments, and string escapes.
- Standalone scanner smoke test target (`MapleScanTest`).

## Next Actions

- Define AST skeleton (`Ast.hh`/`Ast.cc`) and parser interface (`Parser.hh`/`Parser.cc`).
- Decide minimum language grammar for the first runnable slice.