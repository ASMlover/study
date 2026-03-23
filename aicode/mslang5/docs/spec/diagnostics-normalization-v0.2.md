# Maple Diagnostics Normalization Map (v0.2)

Status: v0.2 normative mapping for conformance and diagnostics matching (updated on 2026-03-24).

This document defines normalization rules for diagnostics compatibility.
The primary compatibility key is always `phase + code`.

## 1. Scope

This map applies to:

1. diagnostics produced by compiler/runtime/module paths
2. conformance annotations (`@diag.phase`, `@diag.code`)
3. diagnostics golden matching in CI

## 2. Normalization Principles

1. Prefer canonical `phase + code` per condition.
2. If an older emission exists, normalize to canonical code when policy allows.
3. Keep message text as non-normative; matching must not depend on exact wording.
4. Preserve source line as required; column/length are tolerated as optional during migration.

## 3. Canonical Mapping Table

| Condition Family | Canonical Phase | Canonical Code | Transitional Alias | Tier | Notes |
|---|---|---|---|---|---|
| Unterminated string literal | `lex` | `MS1002` | none | provisional | Planned full conformance lock in v0.3 diagnostics track. |
| Expected expression / generic parse failure | `parse` | `MS2001` | none | stable | Generic parse failure bucket. |
| Invalid assignment target | `parse` | `MS2003` | `MS2001` | provisional | Current parser may still emit `MS2001`; tests may normalize to `MS2003` where rule-specific matching is required. |
| Return outside function | `resolve` | `MS3001` | none | stable | Resolver contract is locked. |
| `this` outside class method context | `resolve` | `MS3002` | none | stable | Resolver contract is locked. |
| `super` outside subclass method context | `resolve` | `MS3003` | none | stable | Resolver contract is locked. |
| Self-inheritance class declaration | `resolve` | `MS3004` | none | stable | Resolver contract is locked. |
| Undefined variable lookup | `runtime` | `MS4001` | none | stable | Includes unresolved global/module-scope access at runtime. |
| Arity mismatch | `runtime` | `MS4002` | none | stable | Applies to function/class/native call paths. |
| Runtime type contract violation | `runtime` | `MS4003` | none | stable | Includes invalid operands and non-instance property receiver paths. |
| Undefined property | `runtime` | `MS4004` | none | stable | Applies to property/method lookup failures. |
| Non-callable invocation | `runtime` | `MS4005` | none | stable | Invocation target is not callable. |
| Module not found | `module` | `MS5001` | none | stable | Resolution/load path not found. |
| Symbol not found in module export table | `module` | `MS5002` | none | stable | `from ... import` missing symbol path. |
| Circular module dependency | `module` | `MS5003` | none | stable | Includes re-entering `initializing` module state. |
| Module initialization failure | `module` | `MS5004` | none | stable | Generic module execution/read/init failure excluding canonical cycle detection. |

## 4. Module-Specific Precedence Rule

When module initialization fails with nested causes:

1. if the nested cause is or normalizes to `MS5003`, normalize final classification to `module + MS5003`
2. otherwise normalize to `module + MS5004`

This keeps cycle diagnosis deterministic.

## 5. Matcher Contract for Tests

Conformance and diagnostics tests should match in this order:

1. `phase` (required)
2. `code` (required)
3. `span.line` (required)
4. `span.column` (optional during migration)
5. `message` is non-blocking and should not be required by default

## 6. Change Control

1. Adding new rows is backward-compatible.
2. Repointing a stable row to a different code is breaking.
3. Any promotion from `provisional` to `stable` must update `docs/spec/errors.md` and conformance matrix notes.
