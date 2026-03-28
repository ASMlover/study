# T18: Classes & Inheritance

**Phase**: 9 - Classes & Inheritance
**Dependencies**: T16 (Functions & Closures), T17 (Garbage Collection)
**Estimated Complexity**: High

## Goal

Implement class declarations, instance creation, field access, methods, constructors (`init`), single inheritance, `this`, and `super`. This extends the compiler, object system, and VM.

## Files to Modify

| File | Changes |
|------|---------|
| `src/object.h` | Fully implement MsClass, MsInstance, MsBoundMethod |
| `src/object.c` | Class/instance/bound method creation and free |
| `src/compiler.c` | Add `compileClassDecl`, `compileGet`, `compileSet`, `compileThis`, `compileSuper` |
| `src/vm.c` | Add OP_CLASS, OP_GET_PROPERTY, OP_SET_PROPERTY, OP_METHOD, OP_INHERIT, OP_INVOKE, OP_SUPER_INVOKE, OP_GET_SUPER |

## TDD Implementation Cycles

### Cycle 1: Object System Extensions (MsClass, MsInstance, MsBoundMethod)

**RED** — Write failing test:
- Create `tests/unit/test_classes.c`
- Write `test_class_object()`: create `MsClass` via `ms_class_new(vm, name)`, verify type is `MS_OBJ_CLASS`, methods table is initialized, `superclass == NULL`
- Write `test_instance_object()`: create `MsInstance` via `ms_instance_new(vm, klass)`, verify type is `MS_OBJ_INSTANCE`, fields table is initialized, `klass` pointer matches
- Write `test_bound_method_object()`: create `MsBoundMethod` via `ms_bound_method_new(vm, receiver, method)`, verify type is `MS_OBJ_BOUND_METHOD`, receiver and method match
- Write `test_type_macros()`: verify `MS_IS_CLASS`, `MS_IS_INSTANCE`, `MS_AS_CLASS`, `MS_AS_INSTANCE` work correctly
- Write `test_class_with_super()`: create class with superclass set, verify `superclass` pointer is correct
- Write `test_object_free_chain()`: create class + instance + bound method, free VM, verify no leak
- Expected failure: linker error — `ms_class_new`, `ms_instance_new`, `ms_bound_method_new` undefined

**Verify RED**: `cmake --build build 2>&1 | grep "undefined reference"` — link errors for class object functions

**GREEN** — Minimal implementation:
- In `src/object.h`: add type check macros:
  - `MS_IS_CLASS(v)`, `MS_IS_INSTANCE(v)` and `MS_AS_CLASS(v)`, `MS_AS_INSTANCE(v)`
- In `src/object.c`:
  - `ms_class_new(vm, name)`: allocate `MsClass`, set `name`, init `methods` table via `ms_table_init()`, set `superclass = NULL`, return
  - `ms_instance_new(vm, klass)`: allocate `MsInstance`, set `klass`, init `fields` table via `ms_table_init()`, return
  - `ms_bound_method_new(vm, receiver, method)`: allocate `MsBoundMethod`, set `receiver` and `method` (closure), return
  - Update `ms_object_free()`:
    - `MS_OBJ_CLASS`: `ms_table_free(&klass->methods)`, free class
    - `MS_OBJ_INSTANCE`: `ms_table_free(&instance->fields)`, free instance
    - `MS_OBJ_BOUND_METHOD`: free bound method (closure is GC-managed separately)
  - Update `ms_gc_blacken_object()`:
    - `MS_OBJ_CLASS`: mark `name`, mark `superclass`, mark all values in `methods` table
    - `MS_OBJ_INSTANCE`: mark `klass`, mark all values in `fields` table
    - `MS_OBJ_BOUND_METHOD`: mark `receiver`, mark `method` (closure)

**Verify GREEN**: `cmake --build build && ./build/test_classes` — object creation and free tests pass

**REFACTOR**: Ensure class and instance table entries are properly marked during GC

### Cycle 2: Class Declaration Compilation

**RED** — Write failing test:
- Add `test_compile_empty_class()`: compile `"class Foo {}"`, verify bytecode: `OP_CLASS("Foo")`, `OP_DEFINE_GLOBAL("Foo")`
- Add `test_compile_class_with_method()`: compile `"class Foo { fn bar() { print 1 } }"`, verify: `OP_CLASS`, `OP_DEFINE_GLOBAL`, `OP_GET_GLOBAL("Foo")`, `OP_CLOSURE(method_fn)`, `OP_METHOD("bar")`
- Add `test_compile_class_with_superclass()`: compile `"class Child < Parent {}"`, verify: `OP_CLASS`, `OP_DEFINE_GLOBAL`, `OP_GET_GLOBAL("Parent")`, `OP_GET_GLOBAL("Child")`, `OP_INHERIT`
- Add `test_compile_init_method()`: compile `"class Foo { fn init() { this.x = 1 } }"`, verify method is compiled as `MS_FUNC_INITIALIZER` type
- Expected failure: compiler error — `compileClassDecl` not implemented

**Verify RED**: `./build/test_classes` — class compilation tests fail

**GREEN** — Minimal implementation:
- In `src/compiler.c`:
  - Replace stub `compileClassDecl()` with full implementation:
    1. Consume `class`, consume identifier (class name)
    2. Emit `OP_CLASS` with name constant
    3. Emit `OP_DEFINE_GLOBAL` with name constant
    4. If `<` token: consume, consume superclass name, emit `OP_GET_GLOBAL(superclass)`, emit `OP_GET_GLOBAL(classname)`, emit `OP_INHERIT`
    5. Consume `{`
    6. `beginScope()`: declare `this` as local (slot 0); if has superclass, declare `super` as local (slot 1)
    7. Loop methods: if `MS_TOKEN_FN`, compile method:
       a. Consume `fn`, consume method name
       b. Create new `MsCompilerState` with type `MS_FUNC_METHOD` (or `MS_FUNC_INITIALIZER` if name is "init")
       c. Compile function body
       d. Emit `OP_CLOSURE`
       e. Emit `OP_METHOD` with method name constant
    8. Consume `}`
    9. `endScope()`
- Ensure method functions capture `this` as an upvalue if accessed in nested functions

**Verify GREEN**: `cmake --build build && ./build/test_classes` — class compilation tests pass

**REFACTOR**: Share function compilation logic between `compileFuncDecl` and method compilation

### Cycle 3: Instance Creation and Field Access

**RED** — Write failing test:
- Add `test_instantiate_class()`: `ms_vm_interpret(vm, "class Foo {}\nvar f = Foo()")` returns `MS_INTERPRET_OK`
- Add `test_instance_type()`: create instance, verify `MS_IS_INSTANCE` returns true
- Add `test_set_field()`: `ms_vm_interpret(vm, "class Foo {}\nvar f = Foo()\nf.x = 42\nprint f.x")` → "42"
- Add `test_get_field()`: set field then retrieve, verify value matches
- Add `test_set_field_wrong_type()`: `"var x = 1\nx.y = 2"` → runtime error "only instances have fields"
- Add `test_get_field_undefined()`: `"class Foo {}\nvar f = Foo()\nprint f.x"` → runtime error "undefined property"
- Expected failure: VM doesn't handle class instantiation or field access

**Verify RED**: `./build/test_classes` — instance and field tests fail

**GREEN** — Minimal implementation:
- In `src/vm.c`:
  - `OP_CLASS`: create `MsClass` with name constant, push onto stack
  - `OP_GET_GLOBAL` dispatch for class: when global value is a class, it's callable (handled via `callValue`)
  - `callValue()` update: if callee is a class → create new `MsInstance`, push as receiver, if class has `init` method, call it; otherwise just push instance
  - `OP_GET_PROPERTY`:
    1. Pop instance (verify it's an instance, else runtime error)
    2. Read name constant
    3. Look up in `instance->fields` table; if found, push value
    4. If not in fields, look up in `instance->klass->methods`; if found, create `MsBoundMethod`, push
    5. If neither, runtime error "undefined property"
  - `OP_SET_PROPERTY`:
    1. Pop instance (verify it's an instance)
    2. Read name constant
    3. Pop value, set in `instance->fields` table
    4. Push value back (assignment result)
- In `src/compiler.c`:
  - `compileGet()`: parse `.name` after expression, emit `OP_GET_PROPERTY` with name constant
  - `compileSet()`: parse `.name = expr`, emit `OP_SET_PROPERTY` with name constant

**Verify GREEN**: `cmake --build build && ./build/test_classes` — instance and field tests pass

**REFACTOR**: Consolidate property lookup (fields then methods) into a helper function

### Cycle 4: Method Definitions and Invocation

**RED** — Write failing test:
- Add `test_method_call()`: `ms_vm_interpret(vm, "class Foo { fn bar() { print \"hello\" } }\nvar f = Foo()\nf.bar()")` → "hello"
- Add `test_method_with_args()`: `ms_vm_interpret(vm, "class Adder { fn add(a, b) { return a + b } }\nvar a = Adder()\nprint a.add(3, 4)")` → "7"
- Add `test_method_access_field()`: `ms_vm_interpret(vm, "class Foo { fn greet() { print this.name } fn init(n) { this.name = n } }\nvar f = Foo(\"world\")\nf.greet()")` → "world"
- Add `test_method_not_found()`: `ms_vm_interpret(vm, "class Foo {}\nFoo().bar()")` → runtime error "undefined method"
- Add `test_invoke_opcode()`: compile `"f.greet()"` and verify it can emit `OP_INVOKE` (direct method call optimization) instead of get property + call
- Expected failure: method calls not working

**Verify RED**: `./build/test_classes` — method tests fail

**GREEN** — Minimal implementation:
- In `src/vm.c`:
  - `OP_METHOD`: pop method closure and class from stack, store in `class->methods` table with name key
  - `OP_INVOKE` (optimization):
    1. Read name constant and arg count
    2. Pop receiver, verify it's an instance
    3. Look up method in `instance->klass->methods`
    4. If found, call directly (create bound method and call frame)
    5. If not found in methods, check fields (fields are not callable, so runtime error)
    6. If not found, runtime error "undefined method"
  - `invokeFromClass(klass, name, argCount)`: find method in class methods table, bind to receiver, create call frame
  - `bindMethod(vm, klass, name)`: look up method, create `MsBoundMethod` with current stack top as receiver, push bound method
- In `src/compiler.c`:
  - In `parseCall()` level: after parsing primary expression, if `.` followed by identifier followed by `(`, emit `OP_INVOKE` instead of `OP_GET_PROPERTY` + `OP_CALL`

**Verify GREEN**: `cmake --build build && ./build/test_classes` — method tests pass

**REFACTOR**: Ensure `OP_INVOKE` and `OP_GET_PROPERTY` + `OP_CALL` produce same behavior

### Cycle 5: Constructors (init)

**RED** — Write failing test:
- Add `test_init_constructor()`: `ms_vm_interpret(vm, "class Foo { fn init() { this.x = 1 } }\nvar f = Foo()\nprint f.x")` → "1"
- Add `test_init_with_args()`: `ms_vm_interpret(vm, "class Foo { fn init(x) { this.x = x } }\nvar f = Foo(42)\nprint f.x")` → "42"
- Add `test_init_returns_self()`: verify `Foo()` returns the instance even after init runs (init's return value is ignored, instance is returned)
- Add `test_init_implicit()`: class without init can still be instantiated: `"class Foo {}\nvar f = Foo()\nprint f"` → no error, instance printed
- Add `test_init_not_callable_directly()`: `"class Foo { fn init() {} }\nvar f = Foo()\nf.init()"` → may call init again (language design choice) or work normally
- Expected failure: constructor not properly invoked

**Verify RED**: `./build/test_classes` — constructor tests fail

**GREEN** — Minimal implementation:
- In `callValue()` for class type:
  1. Create new `MsInstance`
  2. Push instance onto stack (as receiver)
  3. Look up `init` method in class's methods table
  4. If found, call it (creates new call frame with instance as receiver, `this` = instance)
  5. If not found, just push instance (no constructor)
  6. After init returns, the instance remains on stack (init's return value is discarded in favor of the instance)
- Ensure `OP_RETURN` for `MS_FUNC_INITIALIZER` type returns `this` (the receiver) instead of explicit return value
- In compiler: `compileClassDecl` marks method named "init" as `MS_FUNC_INITIALIZER` type

**Verify GREEN**: `cmake --build build && ./build/test_classes` — constructor tests pass

**REFACTOR**: Verify init return handling — initializer methods should always return `this`

### Cycle 6: Inheritance and Super

**RED** — Write failing test:
- Add `test_basic_inheritance()`: `ms_vm_interpret(vm, "class Base { fn greet() { print \"hi\" } }\nclass Child < Base {}\nChild().greet()")` → "hi"
- Add `test_method_override()`: `ms_vm_interpret(vm, "class Base { fn greet() { print \"base\" } }\nclass Child < Base { fn greet() { print \"child\" } }\nChild().greet()")` → "child"
- Add `test_super_call()`: `ms_vm_interpret(vm, "class Base { fn greet() { print \"base\" } }\nclass Child < Base { fn greet() { super.greet() } }\nChild().greet()")` → "base"
- Add `test_super_with_args()`: super method called with arguments, verify args passed correctly
- Add `test_super_in_init()`: `"class Base { fn init() { this.x = 1 } }\nclass Child < Base { fn init() { super.init() } }\nvar c = Child()\nprint c.x"` → "1"
- Add `test_super_no_superclass()`: `"class Foo { fn bar() { super.bar() } }\nFoo().bar()"` → runtime error "no superclass"
- Add `test_super_chain()`: three-level inheritance, super.greet() calls correct level
- Add `test_get_super()`: `"class Base { fn greet() { return \"hi\" } }\nclass Child < Base { fn greet() { var m = super.greet\nprint m() } }\nChild().greet()")` → "hi"
- Expected failure: inheritance and super not implemented

**Verify RED**: `./build/test_classes` — inheritance and super tests fail

**GREEN** — Minimal implementation:
- In `src/vm.c`:
  - `OP_INHERIT`:
    1. Pop superclass and subclass from stack
    2. Verify superclass is a class
    3. Copy all methods from `superclass->methods` into `subclass->methods` (only methods not already defined in subclass)
    4. Set `subclass->superclass = superclass`
  - `OP_GET_SUPER`:
    1. Read name constant
    2. Pop superclass from stack (it's a local variable)
    3. Look up method in `superclass->methods`
    4. Create bound method with current receiver, push
  - `OP_SUPER_INVOKE`:
    1. Read name constant and arg count
    2. Pop superclass from stack
    3. Look up method in `superclass->methods`
    4. Call via `invokeFromClass(superclass, name, argCount)`
- In `src/compiler.c`:
  - `compileThis()`: emit `OP_GET_LOCAL` for `this` variable (always slot 0 in method scope)
  - `compileSuper()`: emit `OP_GET_LOCAL` for `super` variable (slot 1 when superclass exists), then:
    - If followed by `.method()`: emit `OP_SUPER_INVOKE` with method name and arg count
    - If followed by `.method` (no call): emit `OP_GET_SUPER` with method name

**Verify GREEN**: `cmake --build build && ./build/test_classes` — inheritance and super tests pass

**REFACTOR**: Verify `OP_INHERIT` is emitted at compile time so methods are available at runtime

### Cycle 7: this Binding and Integration Tests

**RED** — Write failing test:
- Add `test_this_in_method()`: `ms_vm_interpret(vm, "class Foo { fn who() { return this } fn name() { return \"Foo\" } }\nvar f = Foo()\nprint f.who().name()")` → "Foo"
- Add `test_this_in_nested_function()`: `"class Foo { fn make() { fn inner() { return this }\nreturn inner() }\n}\nvar f = Foo()\nprint f.make()")` — verify `this` captured in closure (may need upvalue support for this)
- Add `test_this_not_outside_class()`: `"print this"` → compile error "cannot use 'this' outside of a class"
- Add `test_super_not_without_superclass()`: `"class Foo { fn bar() { super.bar() } }\nFoo().bar()"` → runtime error
- Add `test_field_on_non_instance()`: `"var x = 1\nx.y = 2"` → runtime error
- Write integration tests:
  - `tests/classes/basic.ms`: class creation, methods, fields
  - `tests/classes/inheritance.ms`: inheritance chain, super calls, method override
  - `tests/classes/methods.ms`: method types, constructors, this binding
- Run all tests under stress GC (`MS_DEBUG_STRESS_GC`) to verify GC correctness with class objects
- Expected failure: edge cases in this/super binding, integration test failures

**Verify RED**: `./build/test_classes` — this binding and integration tests fail

**GREEN** — Minimal implementation:
- Fix `this` binding:
  - `this` is a local variable (slot 0) in method scope
  - When method is called via bound method, receiver is pushed and becomes `this`
  - `this` can be captured as an upvalue in nested closures within methods
- Error handling:
  - Compile error: `this` used outside class method → report "cannot use 'this' outside of a class"
  - Runtime error: field access on non-instance → "only instances have properties"
  - Runtime error: `super` used in class without superclass → "no superclass"
- Run integration test scripts through full pipeline, compare against expected output
- Run under stress GC: `cmake -B build -DMS_DEBUG_STRESS_GC=ON && cmake --build build && ./build/test_classes`

**Verify GREEN**: `cmake --build build && ./build/test_classes` — all tests pass including integration scripts and stress GC

**REFACTOR**: Final review of all class-related opcodes, ensure consistent error messages

## Acceptance Criteria

- [ ] Basic class: `class Foo { fn bar() { print "hello" } }\nvar f = Foo()\nf.bar()` → "hello"
- [ ] Constructor: `class Foo { fn init() { this.x = 1 } }\nvar f = Foo()\nprint f.x` → "1"
- [ ] Fields: `class Foo {}\nvar f = Foo()\nf.x = 42\nprint f.x` → "42"
- [ ] Inheritance: `class Base { fn greet() { print "hi" } }\nclass Child < Base {}\nChild().greet()` → "hi"
- [ ] Super calls: `class Base { fn greet() { print "base" } }\nclass Child < Base { fn greet() { super.greet() } }\nChild().greet()` → "base"
- [ ] Method override: subclass method replaces inherited method
- [ ] `this` binds correctly in methods
- [ ] Field access on non-instance → runtime error
- [ ] No memory leaks

## Notes

- Object types: `MsClass` (name, methods table, superclass pointer), `MsInstance` (klass pointer, fields table), `MsBoundMethod` (receiver value, closure pointer)
- Opcodes: `OP_CLASS`, `OP_GET_PROPERTY`, `OP_SET_PROPERTY`, `OP_METHOD`, `OP_INHERIT`, `OP_INVOKE`, `OP_SUPER_INVOKE`, `OP_GET_SUPER`
- `OP_INHERIT` copies superclass methods into subclass at runtime when class declaration is executed
- `OP_INVOKE` is an optimization: combines property get + call in a single opcode, avoiding intermediate bound method allocation
- Initializer methods (`init`) always return `this` regardless of explicit return value
- `this` and `super` are implemented as local variables (slots 0 and 1) in method scope, making them available for upvalue capture
