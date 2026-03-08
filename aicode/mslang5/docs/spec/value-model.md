# Maple Value/Object Model (Draft v0.1)

Status: Draft (T17 planning deliverable, docs-only).

This document defines value algebra, equality rules, object identity, and callable protocol.

## 1. Value Algebra

Maple runtime values:

1. `nil`
2. `bool`
3. `number`
4. `string`
5. `module`
6. object references:
   - function
   - class
   - instance
   - bound method
   - future runtime object kinds

## 2. Numeric Model

Baseline numeric semantics:

1. numeric domain is IEEE-754 double precision
2. no implicit string-to-number coercion
3. arithmetic operators (`-`, `*`, `/`) require numeric operands
4. `+` supports:
   - number + number => numeric addition
   - string + string => string concatenation
   - all other combinations => runtime type error (`MS4003`)

Division behavior:

1. division by zero follows host floating-point behavior in v0.1 unless changed by future spec
2. diagnostic tightening may be introduced in later revision

## 3. Equality and Identity

### 3.1 Equality (`==`, `!=`) Target Semantics

Draft rule set:

1. `nil == nil` is true
2. booleans compare by value
3. numbers compare numerically
4. strings compare by content
5. object references compare by identity
6. cross-kind equality is false (except future explicit coercion rules, currently none)

### 3.2 Identity

Identity is reference-based for objects/modules:

1. same runtime object instance => identical
2. separate instances with equal field values are not identical by default

## 4. String Semantics

1. strings are immutable values from language perspective
2. concatenation creates logical new value
3. implementation may intern strings as optimization without altering observable semantics

## 5. Callable Contract

Callable categories:

1. function
2. class constructor call
3. bound method

Unified callable protocol:

1. `arity() -> integer`
2. `call(args...) -> value | runtime_error`

Arity rule:

1. argument count must equal callable arity
2. mismatch => `MS4002`

Non-callable invocation:

1. invoking non-callable value => `MS4005`

## 6. Method Binding Contract

Property read that resolves to method returns callable bound to receiver:

1. `obj.m` captures receiver as `this`
2. `obj.m()` and `var f = obj.m; f()` use same receiver
3. rebinding via different receiver requires explicit property fetch from that receiver

## 7. Class/Instance Object Semantics

Class value:

1. callable
2. has method table and optional superclass link

Instance value:

1. has field table
2. property read order:
   - own fields
   - class methods (including inherited chain)
3. unresolved property => `MS4004`

Initializer:

1. class call executes `init` when present
2. initializer result of class call is receiver instance

## 8. Module Value Semantics

Module value:

1. reference object containing export table
2. imported module identity is cache-based (same module name => same module object per VM runtime)

## 9. Error Conditions Summary

1. invalid operand kinds for arithmetic => `MS4003`
2. arity mismatch => `MS4002`
3. non-callable invocation => `MS4005`
4. undefined property => `MS4004`
5. undefined variable => `MS4001`

## 10. Open Questions for v0.2

1. numeric edge-case policy (`NaN`, infinities, signed zero) standardization depth
2. whether module equality is identity-only or by canonical name
3. whether future containers (`list/map`) are value-equality or reference-equality by default
