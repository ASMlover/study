# Maple Module and Namespace Specification (Draft v0.1)

Status: Draft (T18 planning deliverable, docs-only).

This document formalizes module resolution, loading lifecycle, caching, and symbol import behavior.

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

## 5. Export Surface Policy (v0.1 Baseline)

Current baseline export model:

1. top-level module declarations are exported via module export table
2. export visibility controls are not yet introduced

Future extension points:

1. explicit `export` keyword
2. private/internal symbols

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

Existing tests can be migrated from unit/integration module cases to conformance format.
