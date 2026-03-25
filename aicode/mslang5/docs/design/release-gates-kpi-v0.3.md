# Maple v0.3 Release Gates and KPI Thresholds (P3-08)

Status: Draft (release governance definition; no implementation changes in this task)  
Date: 2026-03-24  
Scope: Define release gates, KPI thresholds, owners, metric sources, and trend reporting format for v0.3 closeout.

## 1. Objective

Establish an actionable, auditable release-readiness contract for v0.3 so release candidates are gated by deterministic quality signals rather than ad hoc judgment.

## 2. Scope and Compatibility

1. Applies to v0.3 closeout decisions and release candidate (`RC`) promotion.
2. Does not change language/runtime behavior directly.
3. Keeps diagnostics compatibility anchor on `phase + code`.

## 3. Gate Model

Release decision is based on six gates:

1. G1: Build and baseline deterministic test health.
2. G2: Conformance and diagnostics contract lock.
3. G3: Runtime diagnostics precision threshold.
4. G4: Runtime ownership/GC stability threshold.
5. G5: Robustness suites and critical issue policy.
6. G6: Documentation and traceability completeness.

Gate states:

1. `green`: pass, no blocking action.
2. `yellow`: temporary exception allowed only with approved waiver and due date.
3. `red`: blocking; RC cut is forbidden.

## 4. Release Gates

## G1: Build and Deterministic Baseline

Pass conditions:

1. Configure/build succeeds on required platforms/toolchains.
2. Deterministic suites are green:
   - unit
   - integration
   - conformance
   - diagnostics golden
3. No stale migration guard or removed-path dependency in CI checks.

Fail examples:

1. any deterministic suite regression on main release branch
2. broken CMake/test target wiring

## G2: Conformance and Diagnostics Contract

Pass conditions:

1. Conformance pass rate is 100%.
2. Diagnostics contract pass rate (`phase + code` strict) is 100%.
3. No unauthorized diagnostics code remap outside normalization rules.

Fail examples:

1. conformance mismatch for normative clause
2. diagnostics phase/code drift without migration note

## G3: Runtime Diagnostics Precision

Pass conditions:

1. Runtime/module diagnostics line precision coverage (`P1` or better) >= 95%.
2. High-impact runtime/module paths (`MS4001/2/4/5`, `MS5001~MS5004`) satisfy path-level precision criteria.
3. Ordering and rendering remain deterministic.

Fail examples:

1. coverage drops below threshold
2. fallback `line=1` usage increases in user-facing paths without approved exception

## G4: Runtime Ownership and GC Stability

Pass conditions:

1. Ownership/tracing invariants hold for active runtime object kinds.
2. GC stress metrics remain within configured drift envelope across repeated rounds.
3. No open critical forbidden-state bug (dangling reference, untraced edge, double reclaim class).

Fail examples:

1. invariant violation reproducible in CI profile
2. significant metric drift indicating retention/reclaim regression

## G5: Robustness and Critical Findings

Pass conditions:

1. Required robustness profiles (per `robustness-verification-v0.3`) execute at defined cadence.
2. No open S0/S1 robustness issue.
3. S2 items have owner, repro seed/artifact, and scheduled fix milestone.

Fail examples:

1. unresolved crash/hang/memory-corruption finding
2. missing reproducibility artifact for recurring non-deterministic failure

## G6: Documentation and Traceability

Pass conditions:

1. Spec/design/ADR docs reflect released behavior and contracts.
2. Progress tracker and changelog are up-to-date for all M1-M4 closeout items.
3. KPI report and release gate snapshot are attached to RC decision record.

Fail examples:

1. missing ownership/diagnostics/robustness policy references in release notes
2. stale task status contradicting implemented state

## 5. KPI Register (Owner / Source / Threshold)

## KPI-01 Conformance Pass Rate

1. Owner: Language/Frontend maintainer.
2. Source: `ctest -R maple_tests_conformance` result summary.
3. Threshold: 100% pass on release branch and RC candidate commit.
4. Gate binding: G2.

## KPI-02 Diagnostics Contract Pass Rate

1. Owner: Diagnostics contract owner.
2. Source: diagnostics golden + conformance diagnostics assertions.
3. Threshold: 100% (`phase + code` strict).
4. Gate binding: G2.

## KPI-03 Runtime Diagnostics Line Precision Coverage

1. Owner: Runtime diagnostics owner.
2. Source: precision inventory and path-level precision report.
3. Threshold: >= 95% of runtime/module user-facing failure paths at `P1` or better.
4. Gate binding: G3.

## KPI-04 GC Stability Trend

1. Owner: Runtime/GC owner.
2. Source: integration/unit GC stress snapshots (`collections`, `bytes_live`, `bytes_reclaimed`, object counters).
3. Threshold:
   - collections remain non-zero under churn profile
   - reclaimed bytes remain non-zero under churn profile
   - repeat-run drift bounded by agreed envelope in baseline report
4. Gate binding: G4.

## KPI-05 Guardrail Integrity

1. Owner: Build/CI owner.
2. Source: migration/guardrail checks and CI config validation.
3. Threshold: zero stale-file or removed-path dependency.
4. Gate binding: G1.

## KPI-06 Robustness Critical Backlog

1. Owner: QA/Release owner.
2. Source: robustness triage board and nightly reports.
3. Threshold:
   - S0 open count = 0
   - S1 open count = 0
4. Gate binding: G5.

## 6. Waiver Policy

Yellow waiver is allowed only when all rules are met:

1. exact scope and risk statement recorded
2. owner assigned
3. remediation due date set (must be before GA unless explicitly approved)
4. no waiver allowed for S0, for conformance < 100%, or diagnostics contract < 100%

## 7. Trend Reporting Format

Produce one release KPI report per RC cut.

Required sections:

1. Candidate metadata:
   - commit hash
   - date/time
   - branch
2. Gate snapshot table (`G1..G6` with state and evidence link)
3. KPI table:
   - current value
   - threshold
   - delta vs previous RC
   - owner
4. Exceptions/waivers table
5. Decision summary (`promote RC` / `hold RC`)

Recommended machine-readable companion (`json`):

```json
{
  "release": "v0.3-rcN",
  "commit": "<hash>",
  "gates": {
    "G1": "green",
    "G2": "green",
    "G3": "yellow",
    "G4": "green",
    "G5": "green",
    "G6": "green"
  },
  "kpis": [
    {
      "id": "KPI-01",
      "name": "conformance_pass_rate",
      "value": 1.0,
      "threshold": 1.0,
      "owner": "language"
    }
  ],
  "decision": "hold",
  "waivers": [
    {
      "gate": "G3",
      "reason": "line precision inventory incomplete for low-frequency path",
      "owner": "runtime",
      "due_date": "2026-04-07"
    }
  ]
}
```

## 8. RC Promotion Checklist

Before promoting an RC to GA:

1. all gates are green, or only approved yellow waivers remain
2. KPI thresholds are met or waived per policy
3. no open S0/S1 issue
4. release notes include diagnostics compatibility and known limitations
5. tracker status for P3-series is consistent with final evidence

## 9. Deliverable Mapping

P3-08 maps to Stream D / Milestone M4:

1. release gates are defined
2. KPI owner/source/threshold model is defined
3. trend reporting format is defined
4. RC promotion checklist is defined
