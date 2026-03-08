# Maple Resolver Design (Draft v0.1)

Status: Draft (T16 planning deliverable, docs-only).

This document defines static semantic resolution for Maple.
Resolver runs after parsing and before bytecode generation/runtime execution.

## 1. Goals

1. Bind identifiers to lexical depth deterministically.
2. Detect context-sensitive semantic misuse early.
3. Emit stable resolve-phase diagnostics (`MS3xxx`).
4. Provide compiler metadata for local/upvalue access planning.

## 2. Non-Goals

1. Type inference or static type checking.
2. Runtime value validation (arity at callsite remains runtime unless trivially known).
3. Optimization passes.

## 3. Core Data Structures

### 3.1 Scope Stack

Resolver maintains:

1. `scopes: vector<unordered_map<string, BindingState>>`
2. `currentFunction: none | function | initializer | method`
3. `currentClass: none | class | subclass`

`BindingState` fields:

1. `declared`: name introduced, not yet initialized
2. `defined`: initializer complete / symbol visible
3. `captured`: referenced by nested function
4. `kind`: `var | param | fun | class | this | super`

### 3.2 Resolution Output

For each variable expression/assignment:

1. lexical depth (0 = local scope, 1 = enclosing, etc.)
2. slot index (implementation-chosen)
3. binding kind

This metadata is consumed by compiler backend.

## 4. Resolution Algorithm

## 4.1 Declaration Lifecycle

`declare(name)`:

1. insert name in current scope with `declared=true, defined=false`
2. duplicate name in same scope => resolve error

`define(name)`:

1. mark binding as `defined=true`

Variable read checks:

1. if read in same scope while `declared && !defined`, emit error (self-reference in initializer)

### 4.2 Lexical Lookup

On identifier use:

1. search innermost to outermost local scopes
2. if found, record lexical depth
3. if not found locally, treat as global symbol

Nested function capture:

1. if identifier resolved in enclosing non-global scope, mark `captured=true`
2. propagate capture marker outward for closure planning

### 4.3 Blocks and Functions

Block:

1. `beginScope()`
2. resolve contained declarations/statements
3. `endScope()`

Function resolution:

1. remember previous `currentFunction`
2. set `currentFunction` to function kind
3. new scope for parameters and locals
4. declare+define each parameter
5. resolve body
6. restore previous function context

### 4.4 Class and Inheritance

Class declaration:

1. declare and define class name in current scope
2. if subclass (`class B < A`), resolve superclass symbol and verify non-self-inherit
3. for subclass, open synthetic scope binding `super`
4. open method scope binding `this`
5. resolve each method with appropriate function kind
6. close synthetic scopes in reverse order

Method kinds:

1. `init` => `initializer`
2. others => `method`

## 5. Rule Table (Detection Phase, Code, Message Template)

| Rule ID | Rule | Detection Phase | Error Code | Message Template |
|---|---|---|---|---|
| RES-001 | Duplicate local declaration in same scope | resolve | MS3005 | `variable '{name}' already declared in this scope` |
| RES-002 | Read local variable in its own initializer | resolve | MS3006 | `cannot read local variable '{name}' in its own initializer` |
| RES-003 | `return` outside function | resolve | MS3001 | `cannot return from top-level code` |
| RES-004 | `return <value>` inside initializer (if forbidden policy) | resolve | MS3007 | `cannot return a value from initializer` |
| RES-005 | `this` outside class method context | resolve | MS3002 | `cannot use 'this' outside of a class` |
| RES-006 | `super` outside subclass method context | resolve | MS3003 | `cannot use 'super' outside of a subclass` |
| RES-007 | Class inherits from itself | resolve | MS3004 | `a class cannot inherit from itself` |
| RES-008 | Invalid superclass symbol category | resolve | MS3008 | `superclass must be a class` |

Note:

1. `MS3005~MS3008` are draft additions to baseline catalog; finalize in diagnostics track.

## 6. Integration Contract with Compiler/VM

Compiler requires resolver metadata to choose ops:

1. local depth 0 => local access op
2. depth > 0 and captured => upvalue access op
3. unresolved local => global access op

Class contexts:

1. `this` and `super` are treated as synthetic locals in method resolution scopes.

## 7. Error Handling and Recovery

Resolver should continue after recoverable semantic errors:

1. record diagnostics
2. continue resolving sibling declarations when possible
3. block code generation if resolve-phase errors exist

## 8. Conformance Mapping Hooks

Resolver rules map to conformance IDs:

1. `RES-003` -> `RES-RETURN-OUTSIDE-001`
2. `RES-005` -> `RES-THIS-OUTSIDE-001`
3. `RES-006` -> `RES-SUPER-OUTSIDE-001`
4. `RES-007` -> `RES-SELF-INHERIT-001`

## 9. Open Decisions

1. Should initializer value-return be forbidden at resolve phase (`RES-004`) or normalized at runtime?
2. Should duplicate declaration be warning or error in global scope?
3. Should unresolved identifiers be resolver errors or remain runtime global lookup failures?
