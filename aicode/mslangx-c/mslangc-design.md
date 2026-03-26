# mslangc Architecture and Design

## 1. Document Purpose

This document defines the language contract and the pure C implementation
boundary for `mslangc`. It is the architecture baseline for implementation,
tests, and later evolution. Execution tasks live under `docs/01-task-*.md`
through `docs/15-task-*.md`; this file defines the stable semantics those
tasks must implement.

This revision fixes several underspecified or inconsistent areas in the prior
baseline:

1. Strings are heap objects, not a separate value category outside the object
   system.
2. Top-level bindings are module-scoped, not process-global shared state.
3. Expression precedence, assignment targets, `for` syntax, and import binding
   rules are explicit.
4. Equality, evaluation order, short-circuit behavior, and indexing semantics
   are explicit.
5. `init` semantics, field-vs-method lookup, and module export/import behavior
   are explicit.

## 2. Design Goals

`mslangc` is a small, dynamically typed, embeddable scripting language. Its
design goals are:

1. Provide a closed core language with variables, functions, closures, classes,
   inheritance, modules, containers, and structured diagnostics.
2. Preserve one execution pipeline:
   `source -> AST -> resolution -> bytecode -> VM`.
3. Keep runtime data structures explicit, auditable, and suitable for a pure C
   codebase.
4. Prefer a small, stable core over feature accumulation.

## 3. Core Language Model

### 3.1 Value Kinds

The language value space is:

1. `nil`
2. `bool`
3. `number`
4. `string`
5. `list`
6. `tuple`
7. `map`
8. `function`
9. `class`
10. `instance`
11. `bound_method`
12. `module`
13. `native_function`

Containers are first-class values, not standard-library sugar.

### 3.2 Numbers and Strings

`number` is a 64-bit IEEE-754 floating-point value.

Source files and runtime strings are UTF-8 byte sequences. In v1:

1. String equality is by byte content.
2. String length is byte length.
3. String indexing is out of scope and must be reported as a runtime error if
   attempted through a generic indexing path.
4. The language does not provide a separate integer type.

### 3.3 Truthiness

The following values are false:

1. `nil`
2. `false`
3. `0` and `0.0`
4. `""`
5. empty `list`
6. empty `tuple`
7. empty `map`

All other values are true. The runtime must implement this through a single
entry point such as `ms_value_is_falsey()`.

### 3.4 Equality and Identity

The language supports `==` and `!=`.

Equality rules in v1 are:

1. `nil` equals only `nil`.
2. `bool` compares by boolean value.
3. `number` compares by IEEE-754 numeric equality.
4. `string` compares by UTF-8 byte content.
5. `list`, `tuple`, `map`, `function`, `class`, `instance`, `bound_method`,
   `module`, and `native_function` compare by identity, not by structural
   content.

Structural container equality is intentionally out of scope for v1.

### 3.5 Scope and Binding

The language uses lexical scope.

1. A block introduces a new local scope.
2. `fn` introduces a parameter scope and local scope.
3. Methods introduce an implicit `self` binding.
4. Subclass methods introduce an implicit `super` binding.
5. Closures capture variables from their definition environment by captured
   cell, not by copied value.
6. `self` and `super` behave as synthetic locals and may therefore be captured
   by nested `fn` expressions inside a method.
7. `var name;` is valid and initializes `name` to `nil`.
8. Top-level declarations live in the current module scope, not in a single
   VM-wide global namespace shared by all modules.

### 3.6 Evaluation Order and Short-Circuiting

Expression evaluation is left-to-right.

1. A call evaluates the callee first, then arguments left-to-right.
2. Property access evaluates the receiver before lookup.
3. Index access evaluates the receiver, then the index expression.
4. Assignment evaluates the storage target receiver and index components
   left-to-right, then the right-hand side, then performs exactly one store.
5. `and` and `or` short-circuit.
6. `and` and `or` return the last evaluated operand, not a coerced boolean.

### 3.7 Callable Protocol

Callable objects are:

1. user functions
2. closures
3. classes as constructors
4. bound methods
5. native functions

Native calls should use one stable ABI:

```c
typedef struct MsCallResult {
  int ok;
  MsValue value;
} MsCallResult;

typedef MsCallResult (*MsNativeFn)(struct MsVM* vm, int argc,
                                   const MsValue* argv);
```

Arity is fixed in v1. Variadics, defaults, and keyword arguments are out of
scope.

## 4. Surface Syntax

### 4.1 Keywords

The reserved keywords are:

1. `var`
2. `fn`
3. `return`
4. `class`
5. `self`
6. `super`
7. `print`
8. `import`
9. `from`
10. `as`
11. `if`
12. `else`
13. `while`
14. `for`
15. `break`
16. `continue`
17. `and`
18. `or`
19. `true`
20. `false`
21. `nil`

### 4.2 Statement Termination

Statements may end with:

1. `;`
2. `NEWLINE`
3. implicit termination before `}` or EOF where the grammar allows it

Inside a C-style `for` header, the two separator semicolons are always
mandatory and are not replaced by newline termination.

### 4.3 Literals and Assignment Targets

Container literal syntax is:

1. `[]`, `[a, b, c]` for `list`
2. `()`, `(a,)`, `(a, b)` for `tuple`
3. `{}`, `{key: value}` for `map`

Disambiguation rules:

1. In statement context, `{ ... }` is a block.
2. In expression context, `{ ... }` is a `map` literal.
3. `(expr)` is grouping.
4. `(expr,)` is a one-element tuple.

Valid assignment targets are:

1. an identifier
2. a property access
3. an index access

### 4.4 Operator Precedence and Associativity

The parser must implement the following precedence, from low to high:

1. assignment `=`: right-associative, target restricted to valid lvalues
2. logical `or`: left-associative
3. logical `and`: left-associative
4. equality `== !=`: left-associative
5. comparison `< <= > >=`: left-associative
6. additive `+ -`: left-associative
7. multiplicative `* /`: left-associative
8. unary `! -`: right-associative
9. call, property, index: left-associative
10. primary

### 4.5 Core Grammar Skeleton

```ebnf
program          = { declaration } EOF ;
declaration      = classDecl | fnDecl | varDecl | statement ;

classDecl        = "class" IDENTIFIER [ "<" IDENTIFIER ] "{" { methodDecl } "}" ;
methodDecl       = IDENTIFIER functionBody ;

fnDecl           = "fn" IDENTIFIER functionBody ;
fnExpr           = "fn" functionBody ;
functionBody     = "(" [ parameters ] ")" block ;
parameters       = IDENTIFIER { "," IDENTIFIER } ;

varDecl          = "var" IDENTIFIER [ "=" expression ] terminator ;

statement        = exprStmt | printStmt | returnStmt | breakStmt | continueStmt
                 | block | ifStmt | whileStmt | forStmt | importStmt
                 | fromImportStmt ;

block            = "{" { declaration } "}" ;
ifStmt           = "if" "(" expression ")" statement [ "else" statement ] ;
whileStmt        = "while" "(" expression ")" statement ;
forStmt          = "for" "(" [ forInit ] ";" [ expression ] ";" [ expression ] ")"
                   statement ;
forInit          = varDeclFor | expression ;
varDeclFor       = "var" IDENTIFIER [ "=" expression ] ;

importStmt       = "import" modulePath [ "as" IDENTIFIER ] terminator ;
fromImportStmt   = "from" modulePath "import" IDENTIFIER [ "as" IDENTIFIER ]
                   terminator ;
modulePath       = IDENTIFIER { "." IDENTIFIER } ;

exprStmt         = expression terminator ;
printStmt        = "print" expression terminator ;
returnStmt       = "return" [ expression ] terminator ;
breakStmt        = "break" terminator ;
continueStmt     = "continue" terminator ;
terminator       = ";" | NEWLINE | implicit_terminator ;

expression       = assignment ;
assignment       = assignable "=" assignment | logicOr ;
assignable       = IDENTIFIER | propertyExpr | indexExpr ;
logicOr          = logicAnd { "or" logicAnd } ;
logicAnd         = equality { "and" equality } ;
equality         = comparison { ( "==" | "!=" ) comparison } ;
comparison       = term { ( "<" | "<=" | ">" | ">=" ) term } ;
term             = factor { ( "+" | "-" ) factor } ;
factor           = unary { ( "*" | "/" ) unary } ;
unary            = ( "!" | "-" ) unary | call ;
call             = primary
                   { "(" [ arguments ] ")" | "." IDENTIFIER | "[" expression "]" } ;
arguments        = expression { "," expression } ;

primary          = NUMBER | STRING | "true" | "false" | "nil"
                 | "self" | "super" "." IDENTIFIER
                 | IDENTIFIER | fnExpr
                 | listLiteral | tupleLiteral | mapLiteral
                 | "(" expression ")" ;
```

## 5. Static Semantics

The resolver must enforce these rules:

1. Top-level `return` is illegal.
2. Using `self` outside a method scope or a nested closure inside that method is
   illegal.
3. Using `super` outside a subclass method scope or a nested closure inside that
   method is illegal.
4. `break` may appear only inside a loop body.
5. `continue` may appear only inside a loop body.
6. A class may not inherit from itself.
7. Duplicate declarations in the same scope are illegal, including module scope.
8. A variable may not read its own value inside its initializer.
9. `return <expr>` inside an `init` method is illegal. Bare `return` is allowed
   as an early exit.

The resolver must track at least:

1. scope stack
2. function context stack
3. loop context stack
4. class context stack
5. per-function capture metadata for locals, upvalues, `self`, and `super`

## 6. Compiler Architecture

The frontend is strictly split into four phases:

1. `Lexer`: tokenization
2. `Parser`: AST construction and syntax diagnostics
3. `Resolver`: binding, capture analysis, and static semantic checks
4. `Lowering`: bytecode emission from AST plus resolution data

### 6.1 AST and Compile-Time Memory

AST storage uses an arena:

1. one `MsArena` per compilation
2. AST nodes, token slices, and parse/resolve helper structures allocated from
   the arena
3. one bulk free at compilation end

Each AST node must retain at least:

1. `kind`
2. `node_id`
3. `line`
4. `column`
5. `end_column`

### 6.2 Resolution Data

The resolver must not write complex cross-links directly into AST nodes. It
should produce a separate `ResolutionTable` that records:

1. local slot index
2. lexical depth
3. upvalue metadata
4. legality and binding metadata for `self` and `super`
5. loop context metadata needed by lowering
6. initializer and method context metadata

### 6.3 Lowering Invariants

Lowering must preserve the semantic rules above.

1. `and` and `or` must lower via short-circuit jumps while preserving operand
   result values.
2. `break` and `continue` must use explicit patch lists per loop.
3. `for`-loop `continue` must jump to the increment path, not the loop body
   entry.
4. Global bytecode operations target the current module namespace, not a
   process-wide shared variable table.
5. Bytecode emission must preserve the specified left-to-right evaluation order.

## 7. Runtime Architecture

### 7.1 Value Representation

Runtime values use tagged unions:

```c
typedef enum MsValueType {
  MS_VAL_NIL,
  MS_VAL_BOOL,
  MS_VAL_NUMBER,
  MS_VAL_OBJECT
} MsValueType;

typedef struct MsValue {
  MsValueType type;
  union {
    int boolean;
    double number;
    struct MsObject* object;
  } as;
} MsValue;
```

Strings, containers, functions, classes, modules, and other heap-backed values
all use `MS_VAL_OBJECT`.

### 7.2 Object Header

All heap objects share a common header:

```c
typedef enum MsObjectType {
  MS_OBJ_STRING,
  MS_OBJ_FUNCTION,
  MS_OBJ_CLOSURE,
  MS_OBJ_UPVALUE,
  MS_OBJ_CLASS,
  MS_OBJ_INSTANCE,
  MS_OBJ_BOUND_METHOD,
  MS_OBJ_NATIVE_FN,
  MS_OBJ_LIST,
  MS_OBJ_TUPLE,
  MS_OBJ_MAP,
  MS_OBJ_MODULE
} MsObjectType;

typedef struct MsObject {
  MsObjectType type;
  unsigned char marked;
  struct MsObject* next;
} MsObject;
```

### 7.3 Bytecode VM

Execution uses a stack VM. `MsVM` must contain at least:

1. value stack
2. call frame array
3. open upvalue list
4. interned string table
5. native function registry or builtin table
6. module cache
7. current module pointer or equivalent frame-level module context
8. GC state
9. diagnostic buffer

The core opcode families are:

1. constants, arithmetic, comparison, equality, logical operations
2. `GET_LOCAL / SET_LOCAL`
3. `GET_UPVALUE / SET_UPVALUE / CLOSE_UPVALUE`
4. `GET_GLOBAL / DEFINE_GLOBAL / SET_GLOBAL`
5. `CALL / INVOKE / SUPER_INVOKE / RETURN`
6. `CLASS / INHERIT / METHOD / GET_PROPERTY / SET_PROPERTY / GET_SUPER`
7. `IMPORT_MODULE / IMPORT_SYMBOL`
8. `BUILD_LIST / BUILD_TUPLE / BUILD_MAP`
9. `INDEX_GET / INDEX_SET`
10. `JUMP / JUMP_IF_FALSE / LOOP`

Here, `GLOBAL` means the current module's top-level namespace.

## 8. Objects, Containers, and Access Semantics

### 8.1 Class System

The class system stays intentionally small:

1. A class is callable.
2. An instance owns a mutable field table.
3. A class owns a method table and an optional `superclass`.
4. Method invocation binds `self` automatically.
5. Property lookup checks instance fields first, then the class method table,
   then the superclass chain.
6. Property assignment writes instance fields only; it does not mutate class
   method tables.
7. `init` is the initializer. Calling a class always yields the new instance.
8. If `init` exists, it is invoked automatically after allocation.

### 8.2 Container Semantics

The minimum container semantics are:

1. `list`: ordered, mutable, supports indexed read and write
2. `tuple`: ordered, immutable, supports indexed read only
3. `map`: mutable key-value container; v1 requires string keys

Indexing rules are:

1. `list` and `tuple` indices must be numeric, integral, and in range
   `[0, length)`.
2. Negative indices are not supported in v1.
3. `map[key]` requires `key` to be a string.
4. Reading a missing `map` key is a runtime error.
5. Writing `map[key] = value` inserts or replaces the entry.
6. Writing through a `tuple` index is a runtime error.

Common helpers should include:

```c
int ms_value_length(MsValue value, int* out_length);
int ms_value_is_falsey(MsValue value);
```

`ms_value_length()` should succeed for `string`, `list`, `tuple`, and `map`,
and report failure for other value kinds.

## 9. Module System

Module names use dotted paths such as `a`, `a.b`, and `std.math`.

Path mapping rules are:

1. replace `.` with the host path separator
2. append `.ms`
3. search in configured search-path order
4. cache by canonical resolved path, not only by raw import text

Each module executes in its own top-level namespace. Importing a module must not
merge that namespace into another module implicitly.

Import binding rules are:

1. `import a.b` loads module `a.b` and binds its module object to local name `b`
   unless an explicit `as` alias is provided.
2. `import a.b as mod` binds the module object to `mod`.
3. `from a.b import name` binds exported symbol `name` into the current scope.
4. `from a.b import name as alias` binds the exported symbol to `alias`.
5. `from ... import ...` binds the exported value snapshot after successful
   module initialization; imports are not live bindings in v1.
6. Imported names are not re-exported unless the module defines its own top-level
   `var`, `fn`, or `class` with that name.

Module state is:

1. `UNSEEN`
2. `INITIALIZING`
3. `INITIALIZED`
4. `FAILED`

Module errors are:

1. `MS5001`: module file not found
2. `MS5002`: exported symbol not found
3. `MS5003`: cyclic dependency
4. `MS5004`: module initialization failed

In v1, top-level `var`, `fn`, and `class` declarations are exported by default.
If explicit `export` syntax is added later, it must be a versioned evolution,
not a silent change to this default behavior.

## 10. Diagnostics

Diagnostics must be structured:

```c
typedef struct MsSpan {
  const char* file;
  int line;
  int column;
  int length;
} MsSpan;

typedef struct MsDiagnostic {
  const char* phase;
  const char* code;
  const char* message;
  MsSpan span;
} MsDiagnostic;
```

Phases are:

1. `lex`
2. `parse`
3. `resolve`
4. `runtime`
5. `module`

Error families remain:

1. `MS1xxx` for lexing
2. `MS2xxx` for parsing
3. `MS3xxx` for static semantics
4. `MS4xxx` for runtime
5. `MS5xxx` for modules

Module failures caused by imported code should preserve the originating failure
diagnostic and may add a wrapping import-site `MS5004` diagnostic.

## 11. GC and Memory Management

The runtime uses mark-sweep GC. The root set must include at least:

1. value stack
2. closures reachable from call frames
3. open upvalues
4. current module and any frame-associated module references
5. module cache
6. interned string table
7. builtin/native registry
8. temporary runtime roots created during compilation or module loading

Memory domains must stay separate:

1. compile-time arena
2. runtime heap

All language objects are GC-managed. AST nodes, tokens, and compile-time
diagnostic scratch structures stay outside the GC heap.

## 12. Test Baseline

Testing is layered:

1. unit tests: lexer, parser, resolver, lowering, runtime tables, GC
2. integration tests: functions, closures, classes, modules, containers
3. conformance tests: syntax, semantics, diagnostics, module contracts
4. golden tests: lock `phase + code + line`
5. stress tests: object churn, closure capture, module-cache liveness

Required high-value cases include:

1. empty string and empty-container truthiness
2. precedence, associativity, and left-to-right evaluation order
3. short-circuit behavior and operand-value results of `and` and `or`
4. `fn` and `self` parsing and diagnostics
5. `break` and `continue` legality and lowering behavior
6. container literals, indexing, mutation, and tuple write failures
7. field-before-method lookup and `super.method()` dispatch
8. `init` early return and illegal `return expr`
9. module namespace isolation, cache behavior, alias binding, and export lookup

## 13. Conclusion

`mslangc` implementation must preserve these stable constraints:

1. The surface language uses `fn`, `self`, `break`, and `continue`.
2. Containers are first-class values and participate in truthiness.
3. Strings, containers, functions, classes, and modules share one heap object
   model under `MS_VAL_OBJECT`.
4. Top-level bindings are module-scoped, not process-global shared state.
5. The compiler remains split into `lexer`, `parser`, `resolver`, and
   `lowering`.
6. The runtime uses a stack VM, structured diagnostics, and mark-sweep GC.
7. Pure C is the only implementation target.
