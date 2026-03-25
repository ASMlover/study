# Maple Parser Recovery and Synchronization Criteria (v0.3, P3-04)

Status: Draft (non-behavioral design and acceptance contract)  
Date: 2026-03-24  
Scope: Parser-phase recovery policy, synchronization rules, and conformance acceptance criteria.

## 1. Purpose

This document defines parser recovery behavior that limits cascading diagnostics while preserving deterministic parse-phase contracts (`phase=parse`, `MS2xxx`).

## 2. Goals

1. Make parser synchronization deterministic and testable.
2. Reduce secondary diagnostics caused by one malformed construct.
3. Preserve current language behavior outside malformed regions.
4. Define acceptance criteria that can be automated in conformance suites.

## 3. Non-Goals

1. No grammar extension in this task.
2. No resolver/runtime diagnostics changes in this task.
3. No mandatory wording freeze for diagnostic message text.

## 4. Recovery Model

Recovery model uses panic-mode synchronization with bounded token skipping.

1. On parse failure in a production, parser emits one primary diagnostic for the failing expectation.
2. Parser enters recovery mode for that production scope.
3. Parser advances tokens until a synchronization boundary is reached.
4. Parser exits recovery mode and resumes at the nearest statement/declaration boundary.

Primary invariants:

1. A malformed statement should produce at most one primary parse diagnostic for that statement unless distinct malformed sub-constructs are independently encountered after synchronization.
2. Recovery must always consume at least one token when failure occurs at the current cursor to avoid infinite loops.
3. Parser termination must be guaranteed (`is_at_end()` eventually true or a synchronized construct is consumed).

## 5. Synchronization Rules

## 5.1 Global Boundaries

A token is a synchronization boundary when any of the following is true:

1. End of file (`EOF`).
2. Statement separators (`;`, newline token when newline mode is enabled).
3. Block close (`}`).

## 5.2 Declaration Anchors

After recovering from a failed declaration parse, parser may stop skipping at tokens that likely start a new declaration:

1. `class`
2. `fun`
3. `var`
4. `import`
5. `from`

## 5.3 Statement Anchors

After recovering inside statement parsing, parser may stop skipping at tokens that likely start a new statement:

1. `for`
2. `if`
3. `while`
4. `print`
5. `return`
6. `{`

## 5.4 Expression Context Rule

If failure happens in expression context (for example missing right parenthesis or operand):

1. Parser first attempts local synchronization to expression terminators: `)`, `]`, `}`, `,`, `;`, newline.
2. If local synchronization fails before reaching `EOF`, parser escalates to statement-level synchronization.

## 6. Bounded Cascade Policy

To prevent diagnostic floods, parser enforces caps:

1. Per malformed statement diagnostic cap: 1 primary diagnostic before synchronization.
2. Per file parse diagnostic cap: configurable hard stop (`max_parse_errors`, default 64).
3. When hard cap is reached, parser reports one terminal parse diagnostic indicating truncation and stops parsing remaining input.

## 7. Diagnostics Contract for Recovery

1. Every recovery-originated diagnostic must keep `phase=parse`.
2. Recovery does not emit resolve/runtime codes.
3. Existing compatibility anchor remains `phase + code`; text can evolve.
4. Span baseline: line number required, column optional.

## 8. Acceptance Criteria

## 8.1 Determinism

1. Running parser twice on identical malformed input yields identical diagnostics order.
2. Diagnostics ordering is source-order stable.

## 8.2 Boundedness

1. Single-token deletion cases (missing `;`, `)`, `}`) do not produce unbounded cascades.
2. Diagnostic count does not exceed `max_parse_errors + 1` (including truncation terminal diagnostic).

## 8.3 Continuation Quality

1. Valid statements after a malformed statement are still parsed when a synchronization anchor exists.
2. Parser reaches EOF without hang on arbitrary byte/token streams accepted by lexer.

## 8.4 Compatibility

1. Conformance asserts `phase=parse` and expected `MS2xxx` code for primary errors.
2. Conformance must not hard-match full diagnostic message text.
3. Existing passing scripts remain unaffected.

## 9. Conformance Expectations and Test Taxonomy

## 9.1 Required Conformance Groups

1. `PARSE-RECOVER-SEMI-*`: missing semicolon/newline boundary recovery.
2. `PARSE-RECOVER-BLOCK-*`: missing/extra brace recovery.
3. `PARSE-RECOVER-PAREN-*`: unbalanced parenthesis recovery.
4. `PARSE-RECOVER-DECL-*`: malformed declaration followed by valid declaration.
5. `PARSE-RECOVER-EXPR-*`: malformed expression followed by valid statement.

## 9.2 Assertions per Group

Each case should assert:

1. expected `phase` (`parse`)
2. expected diagnostic `code` (`MS2xxx`)
3. expected line for primary diagnostic
4. parser continuation behavior (later valid statement is compiled or at least parsed boundary is reached)

## 9.3 Golden Matching Priority

Conformance matching order:

1. `phase`
2. `code`
3. `span.line`
4. optional message substring for readability only

## 10. Rollout Plan

1. Land this document and link from iteration tracker (P3-04).
2. Add/adjust parser conformance fixtures to cover synchronization categories.
3. Enforce deterministic diagnostic ordering in parser unit tests.
4. Keep resolver/lowering behavior unchanged during parser recovery work.

## 11. Deliverable Mapping

P3-04 maps to Stream B / Milestone M2:

1. Explicit synchronization rules are defined.
2. Acceptance criteria and conformance expectations are defined.
3. No language feature or runtime behavior change is introduced by this document.
