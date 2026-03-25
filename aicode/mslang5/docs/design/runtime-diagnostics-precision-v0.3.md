# Maple Runtime Diagnostics Precision Contract and Rollout (v0.3, P3-05)

Status: Draft (design contract; non-behavioral in this task)  
Date: 2026-03-24  
Scope: Runtime and module diagnostics precision baseline, acceptance criteria, and staged rollout plan.

## 1. Objective

Define a stable precision contract for runtime/module diagnostics so error locations are actionable and testable while preserving the existing compatibility anchor (`phase + code`).

## 2. Compatibility and Constraints

1. Normative compatibility key remains `phase + code`.
2. Precision improvements are backward-compatible when `phase + code` stay unchanged.
3. Message text can evolve with release notes.
4. This plan does not renumber existing `MS4xxx/MS5xxx` codes.

## 3. Precision Contract

## 3.1 Minimum Required Fields (v0.3 baseline)

Every runtime/module diagnostic must expose:

1. `phase`
2. `code`
3. `message`
4. `span.file` (optional in REPL, required for file/module execution)
5. `span.line` (required, 1-based)

Optional migration fields:

1. `span.column`
2. `span.length`
3. `notes`

## 3.2 Precision Levels

Define location confidence levels for diagnostics quality tracking:

1. `P0` (fallback): synthetic location only (for example `line=1` when source location is unavailable).
2. `P1` (line-accurate): line points to the user-relevant source line that triggered the failure.
3. `P2` (token-range): line + column (+ optional length) point to the precise token/range.

v0.3 contract target:

1. `P1` is the required baseline for runtime/module user-facing failure paths.
2. `P2` is provisional and may be incrementally added.

## 3.3 Classification of Runtime Error Origins

Runtime diagnostics should be classified by origin to determine span derivation:

1. Instruction-bound user error:
   - Examples: undefined variable/property, arity mismatch, non-callable invocation.
   - Required span source: current instruction line from executing chunk.
2. Module boundary error:
   - Examples: module not found, symbol missing, cycle, initialization failure.
   - Required span source: import site line in caller when available; module root line fallback otherwise.
3. VM-internal invariant error:
   - Examples: invalid operand index, corrupted stack state, invalid frame.
   - Required span source: current instruction line; add diagnostic note indicating internal invariant context.

## 4. Canonical Rules

1. `phase=runtime` must be used for execution failures inside VM evaluation.
2. `phase=module` must be used for module load/resolve/initialize contract failures.
3. No runtime path may emit `phase=parse` or `phase=resolve` diagnostics.
4. If an incoming textual error lacks a line, parser/normalizer must derive line from available execution context before falling back to `line=1`.
5. Fallback `line=1` is allowed only when no source mapping context exists; such cases count as `P0` debt.

## 5. Acceptance Criteria

## 5.1 Global Targets

1. Runtime/module diagnostics line-precision coverage (`P1` or better): >= 95% of user-facing runtime/module failure paths.
2. Deterministic ordering and rendering across platforms remain unchanged.
3. `phase + code` conformance pass rate remains 100%.

## 5.2 Path-Specific Targets

1. Undefined variable/property paths (`MS4001`, `MS4004`) must be `P1`.
2. Arity and callable contract paths (`MS4002`, `MS4005`) must be `P1`.
3. Module errors (`MS5001~MS5004`) must be `P1` at caller import site where available.
4. VM-internal invariant errors (`MS4003` subset) must be at least `P1` with an internal-context note.

## 5.3 Conformance Assertions

Conformance and diagnostics tests must assert:

1. `phase`
2. `code`
3. `span.line` presence and expected line (for deterministic fixtures)
4. optional note presence for internal invariant diagnostics

Golden matching priority:

1. `phase`
2. `code`
3. `span.line`
4. optional `span.column`
5. message substring only for readability

## 6. Rollout Plan

## R0: Baseline Measurement and Inventory

1. Inventory runtime/module error paths by code and origin class.
2. Tag each path as `P0/P1/P2` from current behavior.
3. Publish baseline coverage metric and top `P0` debt list.

Exit:

1. Coverage report checked in with reproducible counting method.

## R1: Line Mapping Foundation

1. Standardize instruction-to-line retrieval in VM and module call sites.
2. Ensure error normalization uses contextual line when textual error omits line.
3. Keep `phase + code` unchanged.

Exit:

1. >= 80% runtime/module failure paths reach `P1`.

## R2: High-Impact Path Hardening

1. Upgrade user-facing high-frequency paths (`MS4001`, `MS4002`, `MS4004`, `MS4005`, `MS5001~MS5004`) to `P1`.
2. Add focused integration fixtures for import-site line attribution.
3. Keep deterministic diagnostics order.

Exit:

1. >= 95% runtime/module failure paths reach `P1`.

## R3: Column/Range Optional Upgrade

1. Introduce `span.column/length` where token metadata is available.
2. Do not regress `P1` coverage while extending to `P2`.
3. Keep column matching optional in conformance until explicitly promoted.

Exit:

1. `P1` target maintained, `P2` coverage trend reported.

## 7. Risk and Rollback

1. Risk: precision updates accidentally alter phase or code.
   Mitigation: enforce strict diagnostics normalization tests on `phase + code`.
2. Risk: module line attribution ambiguity in nested import scenarios.
   Mitigation: prioritize caller import line; attach note when fallback is used.
3. Risk: additional metadata breaks brittle message-based tests.
   Mitigation: keep test anchor on `phase + code + span.line`.

Rollback rule:

1. If precision change causes compatibility regression, revert to previous span derivation while preserving code/phase semantics and log debt in `P0` inventory.

## 8. Deliverable Mapping

P3-05 maps to Stream C / Milestone M3:

1. Runtime diagnostics precision contract is documented.
2. Line-level baseline and measurable thresholds are defined.
3. Staged rollout with explicit exit gates is defined.
