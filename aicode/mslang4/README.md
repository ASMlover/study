# Maple Scripting Language

A modern C++23 implementation of a bytecode-compiled scripting language, inspired by the `clox` interpreter from [Crafting Interpreters](https://craftinginterpreters.com/).

## Overview

Maple (namespace `ms`) is a dynamically-typed, object-oriented scripting language featuring:

- **Bytecode Compilation**: Source code compiled to efficient bytecode
- **Stack-based VM**: High-performance virtual machine execution
- **Garbage Collection**: Automatic memory management with mark-and-sweep GC
- **Module System**: Import/export functionality with multiple syntax forms
- **Cross-Platform**: Windows (MSVC) and Linux (GCC) support
- **Modern C++**: Built with C++23 features and best practices

## Documentation

- **[REQUIREMENTS.md](REQUIREMENTS.md)** - Complete requirements specification
- **[DESIGN.md](DESIGN.md)** - Detailed design and architecture documentation

## Quick Start

### Prerequisites

- **C++23 compliant compiler**:
  - Windows: MSVC 2022 (v143) or later
  - Linux: GCC 12+ or Clang 15+
- **CMake 3.25+**

### Building

```bash
# Clone the repository
git clone <repository-url>
cd mslang4

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

### Running

```bash
# Interactive REPL
./maple

# Execute a script
./maple script.maple
```

## Language Features

### Basic Types

```maple
var number = 42;
var text = "Hello, Maple!";
var flag = true;
var nothing = nil;
```

### Control Flow

```maple
if (condition) {
    // ...
} else {
    // ...
}

while (condition) {
    // ...
}

for (var i = 0; i < 10; i = i + 1) {
    print(i);
}
```

### Functions

```maple
fun greet(name) {
    return "Hello, " + name + "!";
}

print(greet("World"));

// Closures
fun makeCounter() {
    var count = 0;
    fun counter() {
        count = count + 1;
        return count;
    }
    return counter;
}

var counter = makeCounter();
print(counter()); // 1
print(counter()); // 2
```

### Classes

```maple
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        print("Hello, I'm " + this.name);
    }
}

class Employee : Person {
    init(name, age, position) {
        super.init(name, age);
        this.position = position;
    }
}

var person = Person("Alice", 30);
person.greet();
```

### Module System

```maple
// Import entire module
import math;

// Import specific items
from utils import helper, logger;

// Import with alias
from collections import List as ArrayList;
```

## Project Structure

```
mslang4/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── REQUIREMENTS.md         # Requirements specification
├── DESIGN.md               # Design documentation
├── src/                    # Source code
│   ├── main.cc             # Entry point
│   ├── common.hh           # Common definitions
│   ├── logger.hh/cc        # Logger system
│   ├── token.hh            # Token definitions
│   ├── scanner.hh/cc       # Lexer
│   ├── ast.hh              # AST node definitions
│   ├── parser.hh/cc        # Parser
│   ├── compiler.hh/cc      # Compiler
│   ├── chunk.hh/cc         # Bytecode chunk
│   ├── value.hh/cc         # Value representation
│   ├── object.hh/cc        # Object system
│   ├── table.hh/cc         # Hash table
│   ├── memory.hh/cc        # Memory management
│   ├── vm.hh/cc            # Virtual machine
│   ├── module.hh/cc        # Module system
│   ├── builtins.hh/cc      # Built-in functions
│   └── platform.hh/cc      # Platform utilities
├── include/ms/             # Public headers
│   └── maple.hh            # Main API header
├── tests/                  # Test suite
│   ├── basic/              # Basic functionality tests
│   ├── functions/          # Function tests
│   ├── classes/            # OOP tests
│   ├── modules/            # Import system tests
│   └── regression/         # Bug regression tests
└── examples/               # Example programs
```

## Development Status

See [REQUIREMENTS.md](REQUIREMENTS.md) for implementation phases and current status.

## Contributing

This project is currently in development. Contribution guidelines will be added once the core implementation is complete.

## License

[License to be determined]

## Acknowledgments

- Robert Nystrom's [Crafting Interpreters](https://craftinginterpreters.com/) for the foundational design
- The C++ community for excellent language features and standards

## References

- [Crafting Interpreters Book](https://craftinginterpreters.com/)
- [Crafting Interpreters GitHub](https://github.com/munificent/craftinginterpreters)
- [C++23 Reference](https://en.cppreference.com/w/cpp/23)
