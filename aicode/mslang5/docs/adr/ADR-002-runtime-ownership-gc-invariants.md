# ADR-002: Runtime Ownership and GC Tracing Invariants

Status: Accepted (design baseline for v0.3 M3; implementation may be incremental)  
Date: 2026-03-24  
Owners: Maple runtime maintainers

## Context

Maple runtime already includes:

1. VM execution as normative path.
2. Tracing GC controller with mark/sweep accounting.
3. Runtime object hierarchy with GC header metadata.

Current ownership model is still transitional:

1. Runtime objects are traced via GC metadata, but many allocations are still retained through VM-side smart-pointer containers.
2. Native function objects use a dedicated ownership container.
3. Modules are managed via shared ownership and are traced as allocation roots by key.

This mixed model works operationally but lacks a single explicit ownership contract and auditable invariants.

## Decision

Adopt a formal ownership and tracing contract with explicit invariants.

1. VM is the sole owner boundary for runtime-managed memory lifecycles.
2. Reachability (through declared root sets and object graph traversal) is the normative liveness criterion.
3. Smart-pointer containers may remain during migration, but must not weaken GC tracing completeness requirements.
4. All runtime and module memory that participates in script semantics must be either:
   - traced via GC object graph / root traversal, or
   - explicitly documented as non-GC-owned runtime infrastructure with independent lifetime.

## Ownership Model

## 1. Ownership Domains

1. VM-owned runtime object domain:
   - all `RuntimeObject` instances participating in language values and execution state.
2. Module domain:
   - module records/caches reachable from module loader and active execution context.
3. Native runtime infrastructure domain:
   - host-side scaffolding not represented as language objects (for example diagnostics helpers).

Rule:

1. Cross-domain references into VM-owned objects require trace visibility from a declared root.

## 2. Allocation Registration Invariant

For every newly allocated `RuntimeObject`:

1. object must be registered exactly once in GC tracking (`register_object`).
2. object byte estimate used for GC accounting must be non-zero and deterministic for the same object shape.
3. object must become trace-reachable from at least one root before the next collection cycle, unless allocation is rolled back.

Violation outcome:

1. treated as memory-safety defect and release blocker.

## 3. Liveness Criterion

An object is live iff reachable from the union of root sets by repeated edge traversal.

1. Reachability graph edges are defined by VM value slots, object fields, upvalue links, class/method tables, module exports, and active execution contexts.
2. Any object not reachable at sweep point is reclaimable.

## GC Root Set Contract

The root set is the disjoint union below; all sets are mandatory unless explicitly empty at runtime.

## 1. Execution Roots

1. VM operand/value stack.
2. Active callframe closures.
3. Open upvalues list.

## 2. Global and Module Roots

1. VM globals table.
2. Current executing module handle (if any).
3. Module loader cache entries and their export tables.

## 3. Transitional Ownership Roots

1. Transitional VM ownership containers holding runtime objects during migration (for example object registries retained by smart pointers) must be traced or otherwise guaranteed equivalent reachability semantics.
2. Such containers are temporary and must be cataloged; adding a new container requires contract update.

## Object Graph Tracing Invariants

Tracer must recursively cover edges by concrete object kind.

1. Closure object:
   - trace function object
   - trace all referenced upvalues
2. Upvalue object:
   - if closed, trace closed value
   - if open, stack root contract must keep slot reachable
3. Class object:
   - trace superclass (if present)
   - trace methods table values
4. Instance object:
   - trace class reference
   - trace fields table values
5. Bound method object:
   - trace receiver value
   - trace underlying closure object
6. Module object/value:
   - trace export table values and module-linked reachable objects

Rule:

1. Introducing a new runtime object kind is incomplete unless tracing edges and tests are added in the same change.

## Sweep and Lifecycle Invariants

1. Sweep must only reclaim unmarked objects/allocation entries.
2. Sweep must unlink reclaimed runtime objects from tracking list exactly once.
3. Mark bits must be reset for survivors before cycle completion.
4. Post-sweep accounting (`bytes_live`, `bytes_reclaimed`, object counters) must satisfy:
   - `bytes_live_after = bytes_live_before - bytes_reclaimed +/- deterministic allocator delta`
   - counters are monotonic within one cycle for processed objects.
5. No reclaimed object pointer may remain in VM live containers after sweep.

## Safety Rules and Forbidden States

Forbidden states:

1. Runtime object reachable from execution state but absent from GC registration.
2. Registered object unreachable due only to missing trace edge for its kind.
3. VM/root container holding raw pointer to reclaimed object.
4. Module cache entry referencing object graph not traversed by tracer.

Required operational checks:

1. Debug builds should keep optional assertions for registration and trace coverage assumptions.
2. Stress tests must force repeated collections under closure/class/module churn.

## Diagnostics and Observability Contract

1. GC metrics snapshot must be queryable through existing runtime stats surface.
2. Collection count, live bytes, and reclaimed bytes must be stable enough for trend-based regression checks.
3. Ownership/tracing invariant violations should be reported with runtime/module diagnostics paths where applicable, without changing `phase + code` contracts unrelated to GC.

## Migration Sequence (v0.3+)

## Phase O1: Contract Enforcement

1. Keep existing mixed ownership model.
2. Add invariant-oriented checks and tracing coverage tests for all current object kinds.
3. Track transitional ownership containers explicitly in docs.

Exit:

1. Root coverage inventory complete.
2. No known untraced edge for existing runtime object kinds.

## Phase O2: Ownership Simplification

1. Reduce reliance on transitional smart-pointer ownership for GC-managed objects.
2. Keep externally visible behavior and diagnostics compatibility unchanged.
3. Validate with stress and conformance suites.

Exit:

1. VM-owned heap semantics are primary lifecycle mechanism.
2. No regression in GC metrics trend under repeated stress runs.

## Phase O3: Contract Hardening

1. Promote key debug assertions to CI-enabled invariant tests where feasible.
2. Freeze ownership/tracing checklist for new runtime object kinds.

Exit:

1. Ownership and tracing checklist is mandatory in runtime PR review criteria.

## Consequences

Positive:

1. Ownership semantics become explicit and auditable.
2. GC regressions can be detected as invariant violations, not only symptom failures.
3. Future object-kind additions have a clear integration checklist.

Tradeoffs:

1. Additional implementation discipline and test overhead.
2. Temporary duality during migration until ownership simplification is complete.

## Verification and Exit Criteria

This ADR is operationally satisfied when:

1. Root set coverage is documented and test-backed.
2. Tracing invariants are implemented for every active runtime object kind.
3. GC stress suites demonstrate stable live/reclaimed trends across repeated runs.
4. No known forbidden state remains open in tracker.

## Traceability

Related planning items:

1. `docs/improve_03.md` -> `P3-06` (Stream C, M3).
2. `docs/design/runtime-diagnostics-precision-v0.3.md` (P3-05).
3. `docs/improve_02.md` GC ownership direction notes.
4. Runtime implementation anchors: `src/runtime/gc.hh`, `src/runtime/gc.cc`, `src/runtime/vm.hh`, `src/runtime/vm.cc`, `src/runtime/object.hh`.
