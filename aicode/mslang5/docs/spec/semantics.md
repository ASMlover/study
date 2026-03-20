# Maple Semantics (Draft v0.1)

Status: Implemented baseline (validated in T14 closeout on 2026-03-21).

This document defines baseline static and runtime semantics for Maple.
This document defines the current contract validated by the regression suites and script checks.

## 1. Semantic Domains

### 1.1 Value Kinds

Maple values are:

1. `nil`
2. `bool` (`true`, `false`)
3. `number` (floating-point numeric domain)
4. `string`
5. `callable object` (function, class, bound method)
6. `module object`
7. `instance object`

### 1.2 Truthiness

Truthiness rules:

1. `nil` is falsey.
2. `false` is falsey.
3. All other values are truthy.

## 2. Name Binding and Scope

### 2.1 Lexical Scope

Scope model is lexical and block-based.

1. A block introduces a new scope.
2. Function declarations introduce a new scope for parameters and local declarations.
3. Class bodies introduce declaration contexts for methods.

Name resolution searches from innermost lexical scope outward to global scope.

### 2.2 Declaration and Assignment

1. `var x;` binds `x` to `nil`.
2. `var x = e;` evaluates `e` then binds `x`.
3. Assignment requires pre-existing binding in local enclosing or global scope.
4. Assigning to an unresolved name is a semantic error.

### 2.3 Closures

A function closes over referenced bindings from its defining lexical environment, not from call-site dynamic scope.

Captured bindings are shared by reference semantics:

1. Updates in one closure invocation are observable by other closures capturing the same binding.
2. Captured bindings outlive declaring scope when referenced by live closures.

## 3. Functions and Calls

### 3.1 Function Values

Function declarations and function expressions produce callable values.

### 3.2 Arity

1. Function arity equals parameter count in declaration.
2. Calling with mismatched argument count is a runtime error.

### 3.3 Return Semantics

1. `return e;` exits nearest enclosing function with value of `e`.
2. `return;` exits with `nil`.
3. `return` outside function body is a semantic error.

## 4. Classes, Instances, and Inheritance

### 4.1 Class Declaration

`class C { ... }` declares class `C`.

`class D < C { ... }` declares subclass `D` with superclass `C`.

Constraints:

1. Superclass must resolve to a class value.
2. A class cannot inherit from itself.

### 4.2 Instances and Fields

Calling a class creates a new instance.

1. Field write `obj.name = value` sets an instance field.
2. Field read `obj.name` first checks instance fields.
3. If missing, lookup continues in class method table (with method binding).
4. If unresolved, runtime error: undefined property.

### 4.3 Methods and `this`

Within an instance method call:

1. `this` is bound to the receiver instance.
2. Accessing `this` outside valid method context is a semantic error.

### 4.4 Initializer `init`

If class defines `init`, class call evaluates `init` with provided arguments.

1. `init` arity defines class constructor arity.
2. `init` always returns receiver instance as call result, regardless of explicit return value policy.

### 4.5 `super`

In subclass methods:

1. `super.method(...)` resolves method in direct/ancestor superclass chain.
2. Receiver remains current `this`.
3. Using `super` outside subclass method context is a semantic error.
4. Missing superclass method is runtime error: undefined property.

## 5. Modules and Imports

### 5.1 Module Identity and Loading

`import a.b;` resolves module by dotted name and loads source exactly once per runtime instance.

1. First import initializes and caches module.
2. Subsequent imports return cached module without re-running top-level module code.

### 5.2 Export Surface

Current baseline export model is global-symbol based for module top-level declarations.

`from a.b import x as y;`:

1. resolves module `a.b`,
2. resolves exported symbol `x`,
3. binds it in current scope as `y` (or `x` when alias omitted).

Missing module or symbol is runtime/module error.

### 5.3 Circular Dependencies

If module `M` is imported while already in initializing state, runtime reports circular dependency error.

## 6. Expression Semantics

### 6.1 Arithmetic

1. `+` on two numbers performs numeric addition.
2. `+` on two strings performs concatenation.
3. `-`, `*`, `/` require numeric operands.
4. Invalid operand categories raise runtime type errors.

### 6.2 Property and Call Evaluation Order

Evaluation order is left-to-right:

1. Callee/object expression first,
2. argument/value expressions next in lexical order,
3. operation execution last.

### 6.3 Assignment Targets

Valid assignment targets:

1. identifier l-values
2. instance property l-values (`call "." IDENTIFIER`)

All other targets are semantic errors.

## 7. Errors and Phases

Maple errors are phase-classified:

1. Lex errors: invalid tokens, unterminated literals.
2. Parse errors: grammar violations.
3. Resolve/static semantic errors: context misuse (`return`, `this`, `super`, invalid inheritance).
4. Runtime errors: arity mismatch, undefined variables/properties, invalid operand kinds.
5. Module errors: module not found, export not found, circular import.

Minimum diagnostic contract:

1. phase label
2. source location (line; column planned)
3. human-readable message

## 8. Conformance-Oriented Notes

This document is intended to map one-to-one to conformance tests:

1. every rule above requires at least one positive test and one negative test,
2. diagnostic text matching should prefer code/phase stable fields once available,
3. behavior changes must update this document before implementation changes.
