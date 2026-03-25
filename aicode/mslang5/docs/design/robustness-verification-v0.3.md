# Maple Robustness Verification Strategy: Fuzz, Invariant, Stress (v0.3, P3-07)

Status: Draft (test strategy and entry criteria; no implementation changes in this task)  
Date: 2026-03-24  
Scope: Define deterministic and non-deterministic robustness suites for Stream D / M4.

## 1. Objective

Define a layered verification strategy that supplements existing unit/integration/conformance/diagnostics suites with:

1. Fuzz testing (input-space exploration).
2. Invariant testing (state/property assertions).
3. Stress testing (long-run and high-churn resilience).

The strategy must preserve compatibility anchors and keep failure triage actionable.

## 2. Design Principles

1. Existing deterministic suites remain release-blocking baseline.
2. Robustness suites are added incrementally with explicit entry gates.
3. Diagnostic compatibility still anchors on `phase + code`.
4. Any non-deterministic suite requires reproducibility controls (seed, run budget, artifact retention).

## 3. Test Taxonomy

## 3.1 Deterministic Suites

1. Unit tests (`tests/unit/*`)
2. Integration tests (`tests/integration/*`)
3. Conformance suites (`tests/conformance/*`)
4. Diagnostics golden suites (`tests/diagnostics/*`)

Use:

1. semantic correctness
2. diagnostics contract checks
3. regression prevention for known behavior

## 3.2 Non-Deterministic / Probabilistic Suites

1. Parser fuzzing (token/script generation + mutation)
2. Runtime invariant fuzzing (VM execution under randomized but bounded programs)
3. Long-run GC/module stress loops with randomized scenario ordering

Use:

1. discover unknown crash/state-corruption paths
2. expose rare lifecycle/edge interactions

## 4. Fuzz Strategy

## 4.1 Parser Fuzz (Primary in v0.3)

Targets:

1. parser recovery/synchronization paths
2. newline/semicolon coexistence boundaries
3. assignment target and nesting edge cases

Input model:

1. seed corpus from existing parse/grammar negative and positive scripts
2. grammar-aware mutation (token insert/delete/replace)
3. structure-preserving mutation for block/paren nesting

Required invariants:

1. parser terminates (no hang)
2. no process crash
3. diagnostics remain parse/lex scoped for syntax faults
4. diagnostic count obeys configured cap behavior

Artifacts per failing seed:

1. failing input script
2. seed id / RNG seed
3. minimized repro input (if reducer available)
4. captured diagnostics and exit category

## 4.2 Runtime Fuzz (Secondary in v0.3)

Targets:

1. callable dispatch edge cases (arity/non-callable/bound method)
2. class/super/closure interactions
3. module import alias/path/cycle combinations

Input constraints:

1. bounded program size and recursion depth
2. bounded loop iteration synthesis
3. bounded import graph size to avoid non-actionable runaway timeouts

Required invariants:

1. VM does not crash or deadlock
2. runtime/module failures produce structured diagnostics (`phase + code`)
3. no forbidden ownership/tracing states are observed (per ADR-002 checks when enabled)

## 5. Invariant Strategy

Invariant tests are deterministic property checks executed with targeted crafted programs and instrumentation snapshots.

## 5.1 Diagnostics Invariants

1. failing runs produce at least one diagnostic
2. primary diagnostic fields are present (`phase`, `code`, `span.line`)
3. diagnostics ordering is deterministic for deterministic fixtures

## 5.2 VM/Runtime Invariants

1. stack discipline holds at instruction boundaries in debug-check mode
2. callframe depth and closure/upvalue invariants remain valid after error exits
3. module cache behavior remains stable across repeated import cycles

## 5.3 GC Invariants

1. every stress round that allocates churn triggers expected collection progression
2. reclaimed/live trends remain within bounded variation across repeated rounds
3. reachable values survive collections (no semantic corruption)

## 6. Stress Strategy

## 6.1 Stress Profiles

1. GC churn profile:
   - repeated object/class/closure churn + forced low GC threshold
2. Module cache profile:
   - repeated import/from-import with alias and cache reuse checks
3. Mixed language profile:
   - control-flow + closure + class + module usage in long-run loops

## 6.2 Execution Budget Profiles

1. PR profile (fast): short stress rounds, deterministic order
2. Nightly profile (deep): longer rounds, multiple seeds, randomized order
3. Pre-release profile: extended budget with failure artifact retention and trend comparison

## 6.3 Acceptance Signals

1. no crash/hang
2. invariant assertions hold
3. diagnostics contract preserved
4. trend metrics do not exceed configured drift thresholds

## 7. Entry Criteria

A robustness suite enters CI only when all criteria below are met.

## 7.1 General Entry Criteria

1. documented scope, owners, and runtime budget
2. reproducibility controls defined (seed handling and rerun command)
3. failure artifacts path defined
4. pass/fail policy defined (blocking vs informational)

## 7.2 Fuzz Suite Entry Criteria

1. has initial seed corpus sourced from real regressions/spec cases
2. enforces timeout and input-size limits
3. records failing seed and minimized repro guidance
4. includes at least one deterministic replay test for known discovered bug class

## 7.3 Invariant Suite Entry Criteria

1. invariant statement is explicit and machine-checkable
2. expected false-positive rate is zero in deterministic mode
3. failure message identifies violated invariant and context

## 7.4 Stress Suite Entry Criteria

1. profile defines round count/time budget
2. key metrics captured (`collections`, `bytes_live`, `bytes_reclaimed`, object counters)
3. drift threshold and baseline source are specified
4. retry policy for flaky infrastructure noise is defined and bounded

## 8. CI Integration Plan

## Phase V1: Baseline Robustness Harness

1. add parser fuzz harness scaffolding and deterministic seed replay mode
2. add invariant checks to existing integration tests where instrumentation already exists
3. keep non-deterministic fuzz runs non-blocking initially

## Phase V2: Gate Promotion

1. promote stable invariant and stress profiles to blocking gates
2. keep deep fuzz/nightly exploratory jobs informational with mandatory triage SLA
3. require bug-class replay cases for every fixed robustness defect

## Phase V3: Release Readiness Coupling

1. pre-release requires green deterministic suites + robustness trend review
2. unresolved critical robustness findings block release candidate cut

## 9. Reporting and Triage Model

Per run, publish:

1. suite id and profile
2. seed(s) used
3. pass/fail + invariant violations
4. crash/hang count
5. diagnostics contract mismatches
6. stress trend deltas against baseline

Severity guidance:

1. S0: crash, memory corruption, or non-termination -> immediate blocker
2. S1: deterministic invariant violation or diagnostics contract break -> blocker
3. S2: non-deterministic reproducible issue -> triage in current milestone
4. S3: flaky/non-reproducible anomaly -> tracked with expiration and owner

## 10. Deliverable Mapping

P3-07 maps to Stream D / Milestone M4:

1. fuzz strategy defined
2. invariant strategy defined
3. stress strategy defined
4. deterministic and non-deterministic suite entry criteria defined
