# ADR-001: Maple Execution Model Unification

Status: Accepted (design-level, docs-only, no implementation in this step)
Date: 2026-03-08
Owners: Maple runtime maintainers

## Context

Maple currently has two execution paths:

1. Bytecode VM path (`Vm::Execute`) with a limited instruction set.
2. Script interpreter path (`Vm::ExecuteSource -> ScriptInterpreter`) with richer language behavior.

This dual-path architecture enabled fast feature delivery, but now introduces structural issues:

1. Semantic drift risk: same source may differ by execution path.
2. Test ambiguity: feature tests validate behavior, but not canonical path.
3. Tooling friction: diagnostics and conformance cannot bind to one normative runtime.
4. Optimization blockage: performance work has unclear target.

The language spec track (T14) requires a single normative semantics path.

## Decision

Maple adopts the **Bytecode VM as the sole normative execution engine**.

Decision details:

1. `ScriptInterpreter` becomes transitional and non-normative.
2. New language features must land in parser/compiler -> bytecode -> VM path.
3. Behavioral correctness is defined by conformance tests against the VM path.
4. During migration, parity checks are required where ScriptInterpreter remains in use.

## Alternatives Considered

### A1. Keep ScriptInterpreter as Normative Engine

Pros:

1. Faster to evolve semantics in short term.
2. Lower compiler/bytecode implementation pressure initially.

Cons:

1. Contradicts project architecture goals centered on clox-style VM.
2. Makes bytecode pipeline secondary and harder to justify long-term.
3. Complicates VM-focused optimization and GC integration.

Decision: rejected.

### A2. Keep Dual Normative Paths Long-Term

Pros:

1. Redundancy can catch some mismatches.

Cons:

1. High maintenance cost.
2. Persistent divergence risk.
3. Harder release criteria and compatibility guarantees.

Decision: rejected.

### A3. VM-Normative + Temporary Interpreter Bridge (Chosen)

Pros:

1. Preserves delivery continuity while converging architecture.
2. Enables controlled migration by staged parity checks.
3. Aligns with spec-first and conformance-first strategy.

Cons:

1. Requires temporary duplication and migration management.
2. Demands discipline in feature routing and CI gates.

Decision: accepted.

## Consequences

### Positive

1. Clear single source of truth for language behavior.
2. Better alignment between spec, tests, and runtime.
3. Stronger foundation for diagnostics and optimization.

### Negative

1. Migration workload for features currently realized in ScriptInterpreter.
2. Short-term increase in test and CI complexity.

## Migration Plan

### Phase 0: Freeze and Gate

1. Mark ScriptInterpreter as transitional in docs and plan.
2. Introduce policy: no new feature may be VM-bypassing.
3. Establish parity matrix skeleton for existing features.

### Phase 1: Feature Parity Closure

Scope:

1. closures/upvalues
2. classes/inheritance/this/super
3. module import behaviors
4. diagnostic phase consistency

Actions:

1. For each feature area, create parity cases:
   - expected output
   - expected failure phase
   - expected diagnostic code/message contract (as available)
2. Port semantics into compiler/bytecode/VM path.
3. Keep interpreter path only for temporary comparison and fallback.

### Phase 2: Default Path Switch

1. Make `ExecuteSource` use compiler+VM as default.
2. Interpreter path remains opt-in debug mode (if still needed).
3. CI gate requires VM-path conformance pass.

### Phase 3: Interpreter Retirement (or strict reference mode)

Two allowed outcomes:

1. Retirement:
   - Remove ScriptInterpreter code and tests tied only to it.
2. Strict reference mode:
   - Keep as non-default reference implementation with mandatory parity suite.

Default preferred outcome: retirement, unless reference value is proven high and maintenance cost acceptable.

## Guardrails and Non-Goals

Guardrails:

1. No semantic change without spec update.
2. No parser/VM feature merge without conformance case.
3. No diagnostic format churn without code mapping update.

Non-goals in this ADR:

1. Detailed resolver algorithm (covered by T16).
2. Full diagnostics schema rollout (covered by T19).
3. Micro-optimization strategy (post-unification).

## Verification and Exit Criteria

ADR considered operationally fulfilled when:

1. `ExecuteSource` normative path is compiler+bytecode+VM.
2. Parity matrix exists for all pre-existing dual-path features.
3. CI includes conformance gate against VM path.
4. Plan marks dual-path risk as closed.

## Traceability

Related plan items:

1. T14 Language Specification v0.1
2. T15 Execution Architecture Unification Plan
3. T16 Static Semantics & Resolver Design
4. T20 Conformance & Regression Test System
