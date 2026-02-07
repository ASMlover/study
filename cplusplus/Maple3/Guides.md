# Maple3 Guides

## 1. Language Implementation Development Guide

### 1.1 Development Objectives

Maple3 aims to evolve from a lightweight scanner/parser project into a complete interpreter with:
1. Frontend: scanner + parser + diagnostics
2. Semantic pass: resolver/scope binding
3. Runtime: value/environment/interpreter
4. CLI: script execution, REPL, test command

### 1.2 Architecture Overview

Current and target architecture details are maintained in `Designs.md`.

Main module responsibilities:
1. `Scanner.*`: source text -> token stream
2. `Token.*`, `TokenTypes.hh`: token model and token type definitions
3. `Parser.*`, `Ast.*`: token stream -> AST
4. `Errors.*`: standardized diagnostics
5. (planned) `Resolver.*`: scope checks and binding
6. (planned) `Value.*`, `Environment.*`, `Interpreter.*`: runtime execution
7. (planned) `Main.cc`/`Cli.*`: CLI command orchestration

### 1.3 Day-to-Day Development Workflow

1. Read `Plans.md` first.
2. Select one task from `Todos.md` and execute via `imp tasks/task_<index>.md`.
3. Implement changes in small, testable increments.
4. Run build and tests before finishing:
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug
```
5. Update documentation in the same round:
- `Plans.md` (progress + next steps)
- `Todos.md` (task status)
- `Designs.md` (architecture changes)
- `Guides.md` (developer/user guidance changes)

### 1.4 Testing Strategy

Maple3 uses dual-track tests:
1. C++ component tests under `tests/` (scanner/parser/resolver/runtime internals)
2. Script regression tests (planned expansion) under:
- `tests/scripts/`
- `tests/expected/`

Rules:
1. Every feature change must add at least one positive testcase.
2. Non-trivial features must add boundary/error testcase.
3. A task is considered done only when all related CTest targets are green.

### 1.5 Coding and File Conventions

1. C++20 codebase, CMake-based build.
2. `*.hh` for headers, `*.cc` for implementations.
3. UTF-8 encoding, LF line endings, trailing whitespace trimmed.
4. Keep includes ordered: standard headers first, then local headers.

## 2. Language Usage Guide

This section describes Maple3 usage status and syntax examples.

### 2.1 Current Project Status

As of now:
1. Scanner and parser test targets are available.
2. Full interpreter runtime/CLI features are under the task roadmap in `Todos.md`.
3. Use CTest targets to validate language frontend behavior.

### 2.2 Build and Run

Configure and build:
```powershell
cmake -S . -B build
cmake --build build --config Debug
```

Run tests:
```powershell
ctest --test-dir build --output-on-failure -C Debug
```

Run executable (current placeholder entry):
```powershell
build\Debug\Maple.exe
```

### 2.3 Syntax Examples (Language Design Direction)

Variable and expression:
```maple
var a = 10;
var b = 20;
print a + b;
```

Condition and loop:
```maple
var n = 3;
if (n > 0) {
  print "positive";
}

while (n > 0) {
  print n;
  n = n - 1;
}
```

Function and closure (planned task scope):
```maple
fun make_counter() {
  var c = 0;
  fun inc() {
    c = c + 1;
    return c;
  }
  return inc;
}

var counter = make_counter();
print counter();
print counter();
```

Class and inheritance (planned task scope):
```maple
class Animal {
  speak() {
    print "animal";
  }
}

class Dog < Animal {
  speak() {
    super.speak();
    print "dog";
  }
}

Dog().speak();
```

Module import (planned task scope):
```maple
import "math.ms";
print add(1, 2);
```

### 2.4 Expected CLI Shape (Planned)

Planned final command shape:
1. `Maple run <file.ms>`
2. `Maple repl`
3. `Maple test <path>`

These commands are part of the roadmap and may not all be available yet until corresponding tasks are completed.
