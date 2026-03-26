# RC Summary Template (v0.3)

Use this template when reviewing generated reports from `tools/release/generate_release_kpi_report.py`.

## Candidate Metadata

- release: `<v0.3-rcN>`
- commit: `<git-hash>`
- branch: `<branch-name>`
- generated_at_utc: `<ISO-8601>`

## Gate Snapshot

| Gate | Status (`pass/warn/fail`) | Evidence |
|---|---|---|
| G1 |  |  |
| G2 |  |  |
| G3 |  |  |
| G4 |  |  |
| G5 |  |  |
| G6 |  |  |

## KPI Table

| KPI | Value | Threshold | Delta vs previous RC | Owner | Status |
|---|---:|---:|---:|---|---|
| KPI-01 conformance_pass_rate |  | `1.0` |  | language/frontend |  |
| KPI-02 diagnostics_contract_pass_rate |  | `1.0` |  | diagnostics |  |
| KPI-03 runtime_diagnostics_precision_coverage |  | `0.95` |  | runtime_diagnostics |  |
| KPI-04 gc_stability_drift_ratio |  | `bounded` |  | runtime_gc |  |
| KPI-05 guardrail_integrity |  | `0.0 fallback_rate` |  | build_ci |  |
| KPI-06 robustness_critical_open_count |  | `0` |  | qa_release |  |

## Exceptions / Waivers

| Gate | Reason | Owner | Due Date | Approval |
|---|---|---|---|---|
|  |  |  |  |  |

## Decision

- decision: `<promote|hold>`
- rationale:
  1. 
  2. 
