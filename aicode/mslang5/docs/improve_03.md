# Maple Next-Iteration Design Plan (v0.3)

Status: Proposal  
Date: 2026-03-24  
Scope: Design and planning only. No implementation changes in this document.

## 1. Context and Goal

Maple has completed VM-only convergence for mainline execution and now supports:

1. Bytecode compilation and stack VM execution.
2. Closures, classes/inheritance, `this`/`super`, and module imports.
3. Baseline standard modules (`std.io`, `std.math`, `std.str`, `std.time`, `std.debug`).
4. Unit/integration/conformance/diagnostics suites with broad semantic coverage.

The next iteration goal is to move from "feature complete baseline" to "stable language contract + production-ready runtime architecture."

## 2. Current-State Audit Summary

This plan is based on current repository code and test behavior.

### 2.1 Strengths

1. VM pipeline is the normative route for script execution.
2. Language feature surface is broad enough for clox-style core behavior.
3. Module cache and cycle detection are implemented.
4. Diagnostics and conformance infrastructure already exists and is usable.

### 2.2 Observed Gaps

1. Frontend architecture is monolithic (`compiler.cc` currently combines parse, resolve-like checks, and bytecode emission).
2. Runtime diagnostics still have limited source precision in many runtime/module error paths.
3. GC and runtime ownership semantics are operational but still transitional in object lifetime model.
4. One migration guard test is stale and fails because it points to a removed legacy file.

## 3. Design Principles for v0.3

1. Spec-first governance: language and diagnostics contracts must be updated before behavior changes.
2. Compatibility anchor: `phase + code` remains the primary diagnostics compatibility key.
3. Layered architecture: parser, resolver, lowering, VM runtime, and GC should evolve with clear boundaries.
4. Continuous conformance: every semantic contract must map to conformance cases.
5. Rollback safety: each milestone must keep buildability and testability.

## 4. Iteration Streams

## 4.1 Stream A: Language Contract Consolidation

Goals:

1. Publish `spec v0.2` stability tiers (`stable` vs `provisional`) per rule.
2. Normalize parse/resolve/runtime error-code boundaries where overlap remains.
3. Clarify newline and semicolon coexistence/deprecation policy with explicit timeline.
4. Define module export policy evolution path (current implicit export baseline -> future explicit exports).

Deliverables:

1. Updated `docs/spec/*` with compatibility notes.
2. Updated conformance matrix mapping every normative clause.
3. Diagnostics compatibility notes for any wording changes without code changes.

## 4.2 Stream B: Frontend Architecture Upgrade

Goals:

1. Split current compiler responsibilities into phases:
   - syntax parsing and recovery
   - lexical scope resolution metadata
   - bytecode lowering/emission
2. Improve parser error recovery and synchronization to reduce cascading errors.
3. Preserve language behavior while reducing coupling for future feature work.

Deliverables:

1. Architecture design document for frontend phase boundaries.
2. Migration checklist for incremental refactor with no semantic drift.
3. Test acceptance criteria by phase.

## 4.3 Stream C: Runtime + GC Ownership Hardening

Goals:

1. Define long-term ownership model (`VM-owned heap` target, explicit object graph invariants).
2. Improve runtime diagnostics span precision (line-level as a minimum baseline).
3. Reduce hot-path dynamic type dispatch overhead with a planned object-tag strategy.

Deliverables:

1. Runtime ownership ADR and object lifecycle invariants.
2. GC root and tracing contract document (stack/frame/upvalue/global/module roots).
3. Runtime diagnostics precision acceptance checklist.

## 4.4 Stream D: Engineering and Verification Maturity

Goals:

1. Replace stale migration guard with valid VM-only reintroduction checks.
2. Strengthen conformance + diagnostics golden policy (`phase + code + span` first).
3. Add targeted robustness suites (parser fuzz seeds, bytecode invariants, long-run GC stress).

Deliverables:

1. Updated CI/test policy for migration debt and guardrails.
2. Structured diagnostics matching policy and normalization rules.
3. Test taxonomy document covering deterministic and stress suites.

## 5. Milestone Plan

## M1: Consistency and Guardrail Repair

1. Remove or replace stale legacy freeze guard behavior.
2. Align docs/spec/tests with current VM-only baseline.
3. Lock diagnostics code mapping where currently ambiguous.

Exit Criteria:

1. Test guardrails are green and meaningful.
2. No CI job depends on removed legacy file paths.

## M2: Frontend Refactor Blueprint and Non-Behavioral Split

1. Publish the parser/resolver/lowering split plan.
2. Introduce phase-local acceptance tests and recovery behavior targets.
3. Keep output semantics unchanged.

Exit Criteria:

1. Existing conformance remains green.
2. New architecture boundaries are documented and review-approved.

## M3: Runtime Diagnostics and Ownership Contracts

1. Publish runtime line-precision diagnostics plan and compatibility constraints.
2. Publish GC/runtime ownership contract and migration sequence.
3. Define object-tag migration strategy for runtime dispatch optimization.

Exit Criteria:

1. Diagnostics precision target is specified and testable.
2. Ownership migration risks and rollback points are documented.

## M4: Verification Expansion and Release Gate Definition

1. Expand robustness test design (fuzz/invariant/stress).
2. Define release readiness gates for language/runtime stability.
3. Document KPIs and trend reporting format.

Exit Criteria:

1. Release gate checklist is complete and actionable.
2. Each KPI has an owner, metric source, and acceptance threshold.

## 6. KPI and Risk Model

## 6.1 Suggested KPIs

1. Conformance pass rate: target 100% on main branch.
2. Diagnostics contract pass rate (`phase + code` strict): target 100%.
3. Runtime diagnostics line-precision coverage: target >= 95% of runtime error paths.
4. GC stability trend: bounded variation of `bytes_live` and `collections` under repeat stress runs.
5. Guardrail integrity: zero stale-file CI checks.

## 6.2 Primary Risks

1. Frontend refactor may accidentally change language behavior.
2. Ownership migration can introduce subtle lifecycle regressions.
3. Diagnostics tightening may break existing message-text-dependent tests.

Mitigation:

1. Keep behavior-preserving phases with explicit invariant checks.
2. Prefer compatibility on `phase + code` during message evolution.
3. Require conformance + diagnostics + stress gates before each milestone closeout.

## 7. Progress Tracking

This section is intended for continuous updates during execution.

| ID | Stream | Milestone | Task | Owner | Status | Start Date | End Date | Evidence/Notes |
|---|---|---|---|---|---|---|---|---|
| P3-01 | A | M1 | Audit and replace stale VM/legacy migration guardrails | Codex | done | 2026-03-24 | 2026-03-24 | Replaced stale legacy-file hash guard with VM-only guard (`cmake/check_vm_only_guard.cmake`), wired in `CMakeLists.txt`, and validated by ctest |
| P3-02 | A | M1 | Publish `spec v0.2` stability tiers and diagnostics code normalization map | Codex | done | 2026-03-24 | 2026-03-24 | Updated `docs/spec/semantics.md`, `docs/spec/errors.md`, `docs/spec/modules.md`, `docs/spec/diagnostics.md`, and added `docs/spec/diagnostics-normalization-v0.2.md` |
| P3-03 | B | M2 | Produce frontend split design (`parser`, `resolver`, `lowering`) | Codex | done | 2026-03-24 | 2026-03-24 | Added `docs/design/frontend-split-v0.3.md` with phase boundaries, contracts, migration steps, and acceptance criteria |
| P3-04 | B | M2 | Define parser recovery acceptance criteria and synchronization rules | Codex | done | 2026-03-24 | 2026-03-24 | Added `docs/design/parser-recovery-v0.3.md` with synchronization boundaries, bounded cascade policy, and conformance expectations |
| P3-05 | C | M3 | Publish runtime diagnostics precision contract and rollout plan | Codex | done | 2026-03-24 | 2026-03-24 | Added `docs/design/runtime-diagnostics-precision-v0.3.md` with precision levels, acceptance thresholds, and phased rollout gates |
| P3-06 | C | M3 | Publish runtime ownership + GC tracing invariants ADR | Codex | done | 2026-03-24 | 2026-03-24 | Added `docs/adr/ADR-002-runtime-ownership-gc-invariants.md` defining ownership domains, root set contract, tracing/sweep invariants, and migration phases |
| P3-07 | D | M4 | Define fuzz/invariant/stress test strategy and entry criteria | Codex | done | 2026-03-24 | 2026-03-24 | Added `docs/design/robustness-verification-v0.3.md` with deterministic/non-deterministic taxonomy, entry criteria, CI promotion phases, and triage model |
| P3-08 | D | M4 | Define release gates and KPI thresholds for v0.3 closeout | Codex | done | 2026-03-24 | 2026-03-24 | Added `docs/design/release-gates-kpi-v0.3.md` with gate model, KPI owner/source/threshold register, waiver policy, and trend reporting format |

## 8. Change Log

1. 2026-03-24: Created initial v0.3 iteration plan with progress tracking table.
2. 2026-03-24: Completed P3-01 by replacing the stale interpreter-freeze guard with an active VM-only anti-regression guard.
3. 2026-03-24: Completed P3-02 by publishing spec v0.2 stability tiers and a diagnostics code normalization map aligned across semantics/errors/modules/diagnostics docs.
4. 2026-03-24: Completed P3-03 by publishing the frontend split architecture design in docs/design/frontend-split-v0.3.md for parser/resolver/lowering boundaries and migration sequencing.
5. 2026-03-24: Completed P3-04 by publishing parser recovery acceptance criteria and synchronization rules in `docs/design/parser-recovery-v0.3.md` with conformance-oriented assertions.
6. 2026-03-24: Completed P3-05 by publishing runtime diagnostics precision contract and rollout sequencing in `docs/design/runtime-diagnostics-precision-v0.3.md` with line-level baseline targets.
7. 2026-03-24: Completed P3-06 by publishing runtime ownership and GC tracing invariants ADR in `docs/adr/ADR-002-runtime-ownership-gc-invariants.md` with root-set and lifecycle constraints.
8. 2026-03-24: Completed P3-07 by publishing fuzz/invariant/stress verification strategy and CI entry criteria in `docs/design/robustness-verification-v0.3.md`.
9. 2026-03-24: Completed P3-08 by publishing release gate and KPI threshold governance in `docs/design/release-gates-kpi-v0.3.md` with trend reporting and waiver policy.

