# Task 06 - Declaration and Control-Flow Parser

## Goal

Complete statement and declaration parsing so the frontend can build a full AST for a source file.

## Design Links

- Core grammar, statement termination, and parser responsibilities: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 04
2. Task 05

## Scope

1. Parse `var`, `fn`, and `class` declarations, including `var name;` defaulting
   to implicit `nil`.
2. Parse `if`, `while`, `for`, `return`, `break`, `continue`, `print`, block, and expression statements.
3. Parse `import`, `import ... as ...`, and `from ... import ... as ...`.
4. Implement parser error recovery so one file can report multiple syntax errors.
5. Support statement termination by `;`, `NEWLINE`, or implicit termination where allowed.

## Implementation Boundaries

1. The parser builds AST nodes only.
2. It must not decide whether `return`, `break`, or `continue` are legal in the current scope; that belongs to Task 09.
3. It must not emit bytecode or runtime diagnostics.
4. The `for` header must require literal semicolon separators even when newline
   termination is enabled elsewhere.
5. Error recovery must stay in the parser layer and preserve useful spans for later diagnostics.

## File Ownership

1. `include/ms/parser.h`
2. `src/frontend/parser_decl_stmt.c`
3. `src/frontend/parser_recovery.c`
4. parser statement/declaration tests under `tests/unit/` and `tests/ms/parser/`

## Diagnostics Contract

1. All parser errors are `phase=parse` with `MS2xxx` codes.
2. Recovery continues from a documented synchronization set so later errors can still be reported.
3. Spans must preserve line, column, and length information.

## TDD Plan

1. Start with failing tests for each declaration and statement form.
2. Add newline, semicolon, implicit-termination, and `for`-header separator
   coverage early.
3. Add invalid `.ms` fixtures that prove multi-error recovery works.
4. Keep parser legality checks out of this task even when tests are tempting to add; those cases belong in resolver tests.

## Acceptance

1. The grammar in the design doc can be parsed into AST form.
2. Mixed declaration/statement files produce stable AST output.
3. Invalid syntax produces `MS2xxx` diagnostics while recovery continues.
4. `import a.b`, `import a.b as c`, and `from a.b import x as y` parse according
   to the design baseline.
5. The task is not complete until build passes, tests pass, and all edited files
   are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "parser|decl_stmt"
```

## Out of Scope

1. Name binding and static checks.
2. Bytecode generation and execution.
