# AGENTS.md - AI Assistant Configuration

## Project Overview

This is the **Maple Scripting Language** project (namespace `ms`), a modern C++23 implementation of a bytecode-compiled scripting language inspired by the `clox` interpreter from [Crafting Interpreters](https://craftinginterpreters.com/).

## Current AI Model

**Model**: GLM-5 (zai-coding-plan/glm-5)

This project is being developed with the assistance of the GLM-5 AI model, specialized for code generation and software engineering tasks.

## Build Commands

### CMake Configuration
```bash
# Create build directory
mkdir build && cd build

# Configure project
cmake ..

# Build project
cmake --build .
```

### Platform-Specific Notes

**Windows (MSVC)**:
```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

**Linux (GCC)**:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Running the Interpreter
```bash
# Interactive REPL
./maple  # Linux
maple.exe  # Windows

# Execute a script
./maple script.maple
```

## Code Standards

### File Naming
- **Headers**: `.hh` extension
- **Implementation**: `.cc` extension
- **Header Guards**: Use `#pragma once` (NOT `#ifndef`/`#define`/`#endif`)

### Naming Conventions
- **Classes/Structs**: PascalCase (e.g., `VirtualMachine`, `BytecodeChunk`)
- **Functions/Methods**: camelCase (e.g., `compileExpression`, `parseStatement`)
- **Variables**: snake_case (e.g., `current_token`, `scope_depth`)
- **Constants**: UPPER_CASE (e.g., `STACK_MAX`, `FRAMES_MAX`)
- **Member Variables**: trailing underscore (e.g., `objects_`, `bytes_allocated_`)
- **Namespaces**: lowercase (e.g., `ms`, `ms::platform`)

### C++ Style Guidelines
- Use C++23 features where appropriate
- Prefer `auto` for type deduction
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) for ownership
- Use `std::vector` instead of raw arrays
- Use `std::string` and `std::string_view` instead of C strings
- Use `constexpr` for compile-time constants
- Use `[[nodiscard]]` for important return values
- Avoid `using namespace std` in headers
- Prefer stack allocation over heap allocation when possible

### Code Organization
- **Namespace**: All code in `namespace ms` or nested namespaces
- **Separation**: One class per file (header + implementation)
- **Includes**: 
  - Standard library headers first
  - Project headers second
  - Use `<>` for system headers, `""` for project headers

### Example Header Structure
```cpp
#pragma once

#include <string>
#include <vector>

#include "common.hh"

namespace ms {

class Example {
public:
    explicit Example(std::string name);
    ~Example() = default;
    
    void doSomething();
    
private:
    std::string name_;
    int count_{0};
};

} // namespace ms
```

### Example Implementation Structure
```cpp
#include "example.hh"

#include <algorithm>

namespace ms {

Example::Example(std::string name)
    : name_(std::move(name)) {
}

void Example::doSomething() {
    count_++;
}

} // namespace ms
```

## Testing

### Test Organization
```
tests/
├── unit/           # C++ unit tests
│   ├── test_scanner.cc
│   ├── test_parser.cc
│   └── ...
├── basic/          # Basic Maple script tests
├── functions/      # Function tests
├── classes/        # OOP tests
├── modules/        # Import system tests
└── regression/     # Bug regression tests
```

### Running Tests
```bash
# C++ unit tests
cd build
ctest

# Maple script tests
./maple tests/basic/arithmetic.maple
```

## Project-Specific Guidelines

### Memory Management
- Use RAII (Resource Acquisition Is Initialization)
- Prefer smart pointers over manual `new`/`delete`
- The VM uses a custom garbage collector for `Object` types
- Use `MemoryManager` for object allocation

### Error Handling
- Use `std::expected` for fallible operations (C++23)
- Throw exceptions only for truly exceptional cases
- Provide clear, actionable error messages with source locations
- Support error recovery in the parser

### Performance Considerations
- String interning for all strings (stored in `strings_` table)
- Pre-allocate stack to avoid reallocation
- Use `std::string_view` to avoid unnecessary copies
- Inline small, frequently-called functions

### Logging
- Use the `Logger` class for all logging
- Appropriate log levels:
  - `TRACE`: Very detailed execution flow
  - `DEBUG`: Debug information
  - `INFO`: General information
  - `WARN`: Warnings
  - `ERROR`: Errors
  - `FATAL`: Fatal errors
- Colors are automatically applied based on level

## Implementation Phases

Refer to [REQUIREMENTS.md](REQUIREMENTS.md) for the complete implementation plan:

1. **Phase 1**: Foundation (Project structure, Logger, Platform layer)
2. **Phase 2**: Lexer and Parser
3. **Phase 3**: Compiler
4. **Phase 4**: Value System
5. **Phase 5**: Virtual Machine
6. **Phase 6**: Memory Management (GC)
7. **Phase 7**: Functions and Closures
8. **Phase 8**: Classes and Inheritance
9. **Phase 9**: Module System
10. **Phase 10**: Polish and Testing

## Key Components

### Scanner (`scanner.hh/cc`)
- Converts source code to token stream
- Tracks line and column numbers
- Handles lexical errors

### Parser (`parser.hh/cc`)
- Recursive descent parser
- Pratt parser for expressions
- Builds AST from tokens

### Compiler (`compiler.hh/cc`)
- Single-pass compiler
- Generates bytecode from AST
- Manages variable scopes

### VM (`vm.hh/cc`)
- Stack-based virtual machine
- Executes bytecode instructions
- Manages call frames

### Garbage Collector (`memory.hh/cc`)
- Mark-and-sweep algorithm
- Automatic memory management
- Tracks all allocated objects

### Module System (`module.hh/cc`)
- Import/export functionality
- Module caching
- Circular dependency detection

## Documentation References

- **[README.md](README.md)**: Project overview and quick start
- **[REQUIREMENTS.md](REQUIREMENTS.md)**: Complete requirements specification
- **[DESIGN.md](DESIGN.md)**: Detailed design and architecture

## AI Assistant Instructions

When working on this project:

1. **Follow the Code Standards**: Adhere to the naming conventions and style guidelines above
2. **Use Modern C++**: Leverage C++23 features appropriately
3. **Cross-Platform**: Ensure code works on both Windows and Linux
4. **Test Your Code**: Write tests for new functionality
5. **Document Changes**: Update relevant documentation when making changes
6. **Memory Safety**: Be mindful of memory management and ownership
7. **Error Handling**: Provide clear, actionable error messages
8. **Performance**: Consider performance implications of design decisions
9. **Read the Docs**: Consult REQUIREMENTS.md and DESIGN.md for context
10. **Incremental Progress**: Work through the implementation phases systematically

## Current Status

The project is in the **planning and documentation phase**. Implementation will begin with Phase 1 (Foundation).

## Notes

- The implementation uses minimal external dependencies (only C++ standard library)
- The goal is to create a production-ready, well-documented scripting language
- Performance should be competitive with other scripting languages like Lua or Python
- Code quality and maintainability are prioritized over premature optimization
