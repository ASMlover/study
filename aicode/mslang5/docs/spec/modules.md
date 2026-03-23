# Maple Module and Namespace Specification (Spec v0.2)

Status: Implemented baseline with v0.2 stability tiers (updated on 2026-03-24).

This document formalizes module resolution, loading lifecycle, caching, and symbol import behavior.

## 0. Stability Tiers (v0.2)

Tier definitions:

1. `stable`: compatibility-locked in v0.2.
2. `provisional`: may evolve in v0.3 with migration notes.

| Module Contract Area | Tier | Notes |
|---|---|---|
| Dotted-name resolution and import forms | stable | `import` and `from ... import` syntax is locked. |
| Lifecycle states and cycle detection | stable | `MS5003` behavior is compatibility-critical. |
| Cache behavior for initialized/failed modules | stable | `MS5004` failed-cache retry semantics are preserved. |
| Implicit top-level export policy | stable | Current runtime behavior and tests rely on this baseline. |
| Future explicit export/private controls | provisional | Planned evolution path, not part of v0.2 stable surface. |

## 0.1 Compatibility Notes

Tier mapping policy for normative clauses in this document:

1. Default rule: all numbered normative clauses are `stable` unless explicitly marked `provisional`.
2. `provisional` clauses in this version:
   - Section 5 planned extension points (`export` keyword and private/internal visibility controls).
3. Existing implicit top-level export behavior remains the `stable` v0.2 baseline.

## 1. Module Names and Paths

Module names are dotted identifiers:

1. `a`
2. `a.b`
3. `core.math.linear`

Resolution baseline:

1. replace `.` with path separator `/`
2. append `.ms`
3. search in configured module search paths in declared order

Example:

1. `foo.bar` -> `foo/bar.ms`

## 2. Import Forms

### 2.1 Module Import

`import a.b;`

Behavior:

1. resolve module path
2. load/initialize module if not cached
3. bind module object to current scope using last segment (`b`)

### 2.2 Symbol Import

`from a.b import x as y;`

Behavior:

1. resolve/load module `a.b`
2. resolve export symbol `x` in module export table
3. bind to current scope as `y` (or `x` if alias omitted)

## 3. Module Lifecycle State Machine

States:

1. `unseen`
2. `initializing`
3. `initialized`
4. `failed`

Transitions:

1. `unseen -> initializing`: first load start
2. `initializing -> initialized`: successful execution
3. `initializing -> failed`: execution error
4. `failed`: not cached as successful module object for future success path

Circular dependency detection:

1. re-entering module in `initializing` state is module error `MS5003`

## 4. Caching Rules

1. cache key is canonical module name string
2. successful module initialization is cached exactly once per VM instance
3. repeated imports return cached module without re-executing top-level code
4. failed initialization should not produce reusable successful cache entry

## 5. Export Surface Policy (v0.2 Baseline)

Current baseline export model:

1. top-level module declarations are exported via module export table
2. export visibility controls are not yet introduced

Planned extension points (provisional):

1. explicit `export` keyword [provisional]
2. private/internal symbols [provisional]

## 6. Namespace Binding Rules

1. `import a.b;` introduces one local/global name (`b`) unless shadowed by local scope policy
2. `from a.b import x;` introduces symbol `x` directly
3. alias form introduces alias symbol only

Shadowing:

1. follows lexical scope rules from resolver policy
2. duplicate same-scope declarations should be resolver error where resolver is active

## 7. Error Contract

Module error families:

1. module not found => `MS5001`
2. symbol not exported => `MS5002`
3. circular dependency => `MS5003`
4. module initialization failed => `MS5004`

Diagnostics should include:

1. failing module name
2. source import statement line
3. nested cause when module execution fails

Normalization and matching rules are defined in `docs/spec/diagnostics-normalization-v0.2.md`.

## 8. Compatibility Table

| Syntax | Expected Result | Errors |
|---|---|---|
| `import a.b;` | bind module object as `b` | `MS5001`, `MS5003`, `MS5004` |
| `from a.b import x;` | bind exported `x` as `x` | `MS5001`, `MS5002`, `MS5003`, `MS5004` |
| `from a.b import x as y;` | bind exported `x` as `y` | `MS5001`, `MS5002`, `MS5003`, `MS5004` |

## 9. Conformance Mapping

Required case families:

1. successful first import
2. repeated import cache reuse
3. missing module
4. missing symbol
5. circular dependency
6. alias binding correctness

Current conformance/integration suites already cover this baseline.




