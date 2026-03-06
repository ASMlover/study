# Maple Scripting Language - Requirements Specification

## 1. Project Overview

### 1.1 Name and Identity
- **Language Name**: Maple
- **Namespace**: `ms` (Maple Script)
- **Implementation Language**: C++23
- **Build System**: CMake
- **Target Platforms**: Windows (MSVC) and Linux (GCC)

### 1.2 Project Goals
Design and implement a production-ready scripting language interpreter inspired by the `clox` implementation from "Crafting Interpreters", enhanced with modern C++ features, a module system, and improved tooling.

### 1.3 Reference Implementation
- **Base Reference**: `clox` from [craftinginterpreters](https://github.com/munificent/craftinginterpreters)
- **Key Features to Retain**:
  - Bytecode compilation
  - Stack-based Virtual Machine
  - Mark-and-sweep Garbage Collection
  - Closures and upvalues
  - Classes and inheritance
  - First-class functions

## 2. Core Requirements

### 2.1 Language Features (Inherited from clox)

#### 2.1.1 Data Types
- **Primitives**:
  - `nil` (null value)
  - Boolean (`true`/`false`)
  - Numbers (double-precision floating point)
  - Strings (heap-allocated, immutable)
- **Compound Types**:
  - Functions (first-class, closures)
  - Classes (with inheritance)
  - Instances (class instances)
  - Lists/Arrays (to be designed)

#### 2.1.2 Control Flow
- `if`/`else` statements
- `while` loops
- `for` loops (C-style)
- `break` and `continue` statements
- `return` statements

#### 2.1.3 Functions
- User-defined functions
- Closures with upvalues
- Variadic functions (optional enhancement)
- Native functions (built-in)
- Recursion support

#### 2.1.4 Object-Oriented Programming
- Class declarations
- Constructor (`init` method)
- Instance methods
- Field access (getters/setters)
- Inheritance (single inheritance)
- `this` keyword
- `super` keyword

#### 2.1.5 Operators
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Logical**: `and`, `or`, `!`
- **Assignment**: `=`
- **String concatenation**: `+`

### 2.2 Enhanced Features (Beyond clox)

#### 2.2.1 Module System (NEW)
##### Import Statement Forms:
```maple
// Import entire module
import math;

// Import specific items
from math import sqrt, pow;

// Import with alias
from math import sqrt as squareRoot;
from utils import helpers as h;

// Combined forms
from collections import List, Map as HashMap;
```

##### Module Semantics:
- Each `.maple` file is a module
- Modules are singletons (loaded once, cached)
- Module scope isolation
- Circular import detection
- Relative and absolute import paths
- Module search path configuration

##### Module System Requirements:
- **File Resolution**: `.maple` extension auto-added
- **Search Paths**: Current directory + configured paths
- **Module Caching**: Prevent double-loading
- **Export Mechanism**: Top-level declarations are exported
- **Error Handling**: Clear error messages for missing modules

#### 2.2.2 Enhanced Debugging Logger (NEW)
##### Logger Levels:
- `TRACE` (Gray) - Very detailed execution flow
- `DEBUG` (Blue) - Debug information
- `INFO` (Green) - General information
- `WARN` (Yellow) - Warnings
- `ERROR` (Red) - Errors
- `FATAL` (Magenta) - Fatal errors

##### Logger Features:
- **Colored Output**: ANSI color codes for terminal
- **Timestamps**: Optional millisecond precision
- **Source Location**: File, line, function name
- **Categories**: Module/component-based filtering
- **Output Destinations**: Console, file, or both
- **Runtime Configuration**: Enable/disable levels programmatically
- **Conditional Logging**: Compile-time level filtering
- **Thread Safety**: Safe for multi-threaded use (future)

##### API Design:
```cpp
ms::Logger::setLevel(ms::LogLevel::DEBUG);
ms::Logger::info("VM initialized");
ms::Logger::debug("Compiling function: {}", functionName);
ms::Logger::error("Undefined variable '{}' at line {}", name, line);
```

#### 2.2.3 Additional Built-in Functions
- `print(value)` - Print to stdout
- `clock()` - Return timestamp in seconds
- `type(value)` - Return type name string
- `len(string|array)` - Return length
- `input([prompt])` - Read from stdin (optional)
- `str(value)` - Convert to string
- `num(value)` - Convert to number

#### 2.2.4 List/Array Support (NEW - Optional)
```maple
var list = [1, 2, 3];
list.append(4);
print(list[0]);  // Index access
list[1] = 10;    // Index assignment
print(list.length);
```

### 2.3 Implementation Requirements

#### 2.3.1 Code Organization
```
mslang4/
├── CMakeLists.txt              # Build configuration
├── README.md                   # Project documentation
├── REQUIREMENTS.md             # This file
├── DESIGN.md                   # Design specification
├── src/                        # Source code
│   ├── main.cc                 # Entry point
│   ├── common.hh               # Common definitions
│   ├── logger.hh/cc            # Logger system
│   ├── token.hh                # Token definitions
│   ├── scanner.hh/cc           # Lexer
│   ├── ast.hh                  # AST node definitions
│   ├── parser.hh/cc            # Parser
│   ├── compiler.hh/cc          # Compiler
│   ├── chunk.hh/cc             # Bytecode chunk
│   ├── value.hh/cc             # Value representation
│   ├── object.hh/cc            # Object system
│   ├── table.hh/cc             # Hash table
│   ├── memory.hh/cc            # Memory management
│   ├── vm.hh/cc                # Virtual machine
│   ├── module.hh/cc            # Module system
│   ├── builtins.hh/cc          # Built-in functions
│   └── platform.hh/cc          # Platform utilities
├── include/ms/                 # Public headers
│   └── maple.hh                # Main API header
├── tests/                      # Test scripts
│   ├── basic/                  # Basic tests
│   ├── functions/              # Function tests
│   ├── classes/                # OOP tests
│   ├── modules/                # Import tests
│   └── regression/             # Bug regression tests
└── examples/                   # Example programs
```

#### 2.3.2 C++23 Features to Use
- **Modules**: Consider using C++20 modules (optional)
- **std::format**: String formatting
- **std::span**: Non-owning views
- **std::expected**: Error handling
- **std::source_location**: Better error reporting
- **std::jthread**: Cooperative interruption (if needed)
- **constexpr**: More compile-time computations
- **auto**: Type deduction
- **Concepts**: Template constraints
- **std::string_view**: Efficient string handling
- **Smart pointers**: RAII-based memory management
- **std::variant**: Type-safe unions
- **std::optional**: Optional values
- **Structured bindings**: Tuple unpacking

#### 2.3.3 Cross-Platform Requirements
- **Compilers**:
  - Windows: MSVC 2022 (v143) or later
  - Linux: GCC 12+ or Clang 15+
- **CMake**: Version 3.25+
- **Platform Abstractions**:
  - File I/O
  - Path manipulation
  - Console colors
  - Time functions
- **No Platform-Specific Code in Core**: Use abstraction layer

#### 2.3.4 Memory Management
- **Custom Allocator**: For VM objects
- **Garbage Collection**: Mark-and-sweep
  - Incremental GC (optional enhancement)
  - Generational GC (future)
- **Memory Tracking**: Debug allocation tracking
- **RAII**: Use C++ destructors for resources

#### 2.3.5 Error Handling
- **Compile-Time Errors**:
  - Syntax errors
  - Semantic errors
  - Type errors (if static typing added)
- **Runtime Errors**:
  - Division by zero
  - Undefined variables
  - Stack overflow
  - Type errors
- **Error Reporting**:
  - Clear messages with source location
  - Line and column numbers
  - Context snippets
  - Error recovery (continue parsing)

#### 2.3.6 Code Style
- **Headers**: `#pragma once` (not `#ifndef` guards)
- **Extensions**: `.hh` for headers, `.cc` for implementation
- **Naming**:
  - Classes: PascalCase
  - Functions: camelCase
  - Variables: snake_case
  - Constants: UPPER_CASE
  - Members: trailing underscore (`name_`)
- **Formatting**: Consistent indentation (2 or 4 spaces)
- **Comments**: Where necessary, prefer self-documenting code
- **No `using namespace std`**: In headers

#### 2.3.7 Testing Requirements
- **Unit Tests**: C++ test suite for components
- **Integration Tests**: Maple script test suite
- **Test Categories**:
  - Lexer tests
  - Parser tests
  - Compiler tests
  - VM execution tests
  - GC tests
  - Module system tests
  - Built-in function tests
  - Error handling tests
  - Performance benchmarks (optional)
- **Test Framework**: Use simple custom framework or Catch2
- **Coverage**: Aim for high code coverage
- **Regression**: All bugs get a test case

#### 2.3.8 Performance Targets
- **Startup Time**: < 50ms for simple scripts
- **Execution Speed**: Competitive with Lua/Python
- **Memory Efficiency**: Minimize overhead
- **GC Pause Time**: < 10ms for typical workloads

### 2.4 Build and Distribution

#### 2.4.1 CMake Configuration
- **Targets**:
  - `maple` - Executable
  - `maple_lib` - Static/shared library (optional)
- **Build Types**: Debug, Release, RelWithDebInfo
- **Compiler Flags**: Proper warnings and standards
- **Dependencies**: Minimal (only standard library)
- **Install Targets**: Optional

#### 2.4.2 Command-Line Interface
```bash
# Run a script
maple script.maple

# Interactive REPL
maple

# Compile to bytecode (optional)
maple -c script.maple -o script.mbc

# Execute bytecode (optional)
maple -x script.mbc

# Set module search path
maple --path /usr/lib/maple script.maple

# Set log level
maple --log-level debug script.maple

# Version info
maple --version

# Help
maple --help
```

### 2.5 Documentation Requirements

#### 2.5.1 User Documentation
- **Language Reference**: Syntax and semantics
- **Standard Library**: Built-in functions
- **Examples**: Common patterns and use cases
- **Tutorial**: Getting started guide

#### 2.5.2 Developer Documentation
- **Architecture**: System design
- **Implementation Notes**: Key algorithms
- **Contributing Guide**: For contributors

## 3. Non-Functional Requirements

### 3.1 Quality Attributes
- **Readability**: Clear, well-documented code
- **Maintainability**: Modular design
- **Extensibility**: Easy to add features
- **Portability**: Cross-platform support
- **Reliability**: Proper error handling

### 3.2 Constraints
- **No External Dependencies**: Use only C++ standard library
- **C++23**: Must compile with C++23 compliant compilers
- **License**: Choose appropriate open-source license
- **Binary Size**: Keep executable size reasonable (< 5MB)

## 4. Implementation Phases

### Phase 1: Foundation
- [ ] Project structure and CMake setup
- [ ] Logger system with colors
- [ ] Common utilities and platform layer
- [ ] Token definitions

### Phase 2: Lexer and Parser
- [ ] Scanner implementation
- [ ] Token stream generation
- [ ] Parser and AST construction
- [ ] Error reporting

### Phase 3: Compiler
- [ ] Bytecode instruction set
- [ ] Code generation
- [ ] Chunk representation
- [ ] Debug information

### Phase 4: Value System
- [ ] Value representation
- [ ] Object system
- [ ] String implementation
- [ ] Table (hash map) implementation

### Phase 5: Virtual Machine
- [ ] VM core
- [ ] Instruction execution
- [ ] Stack management
- [ ] Call frames

### Phase 6: Memory Management
- [ ] Allocation system
- [ ] Garbage collector
- [ ] Memory tracking
- [ ] Stress testing

### Phase 7: Functions and Closures
- [ ] Function objects
- [ ] Upvalues
- [ ] Closures
- [ ] Native functions

### Phase 8: Classes and Inheritance
- [ ] Class objects
- [ ] Instance objects
- [ ] Method binding
- [ ] Inheritance chain

### Phase 9: Module System
- [ ] Module loader
- [ ] Import statements
- [ ] Module caching
- [ ] Path resolution

### Phase 10: Polish and Testing
- [ ] Comprehensive test suite
- [ ] Performance optimization
- [ ] Documentation
- [ ] Examples

## 5. Success Criteria

- [ ] Successfully compiles on Windows (MSVC) and Linux (GCC)
- [ ] Passes all test cases from craftinginterpreters test suite
- [ ] Module system working with all import forms
- [ ] Logger producing colored output at different levels
- [ ] Garbage collector working correctly
- [ ] Classes and inheritance functioning
- [ ] Clear error messages
- [ ] Performance acceptable for real-world scripts
- [ ] Documentation complete and helpful

## 6. Future Enhancements (Out of Scope for v1.0)

- Just-In-Time (JIT) compilation
- Static type system
- Exception handling
- Coroutines/async-await
- Standard library (I/O, networking, etc.)
- Package manager
- Debugger integration
- Language Server Protocol (LSP)
- WebAssembly target
- Multi-threading support
- FFI (Foreign Function Interface)
