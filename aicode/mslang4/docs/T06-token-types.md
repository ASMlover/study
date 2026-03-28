# T06: Token Types

**Phase**: 1 - Foundation
**Dependencies**: T02 (Common Definitions)
**Estimated Complexity**: Low

## Goal

Define the complete token type enumeration and token data structure used by the scanner, parser, and compiler. This is a header-only module (no `.c` file needed).

## Files to Create

| File | Purpose |
|------|---------|
| `src/token.h` | Token type enum and token struct |
| `tests/unit/test_token.c` | Unit tests for token types |

## TDD Implementation Cycles

### Cycle 1: MsTokenType Enum — Compiles and Contains All Tokens

**RED** — Write failing test:
- Create `tests/unit/test_token.c` that verifies the enum exists and has the expected token count:

```c
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

static void test_token_count(void) {
    if (MS_TOKEN_EOF + 1 != 49) {
        fprintf(stderr, "FAIL: expected 49 token types, got %d\n", MS_TOKEN_EOF + 1);
        exit(1);
    }
}

int main(void) {
    test_token_count();
    printf("test_token_count passed\n");
    return 0;
}
```

- Add to `tests/CMakeLists.txt`:
```cmake
add_executable(test_token tests/unit/test_token.c)
target_include_directories(test_token PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test_token COMMAND test_token)
```

- `token.h` does not exist yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `token.h` not found

**GREEN** — Minimal implementation:
- Create `src/token.h` with the full `MsTokenType` enum:

```c
#ifndef MS_TOKEN_H
#define MS_TOKEN_H

typedef enum {
    MS_TOKEN_LEFT_PAREN, MS_TOKEN_RIGHT_PAREN,
    MS_TOKEN_LEFT_BRACE, MS_TOKEN_RIGHT_BRACE,
    MS_TOKEN_LEFT_BRACKET, MS_TOKEN_RIGHT_BRACKET,
    MS_TOKEN_COMMA, MS_TOKEN_DOT, MS_TOKEN_NEWLINE, MS_TOKEN_SEMICOLON, MS_TOKEN_COLON,
    MS_TOKEN_BANG, MS_TOKEN_BANG_EQUAL,
    MS_TOKEN_EQUAL, MS_TOKEN_EQUAL_EQUAL,
    MS_TOKEN_GREATER, MS_TOKEN_GREATER_EQUAL,
    MS_TOKEN_LESS, MS_TOKEN_LESS_EQUAL,
    MS_TOKEN_PLUS, MS_TOKEN_MINUS, MS_TOKEN_STAR, MS_TOKEN_SLASH, MS_TOKEN_PERCENT,
    MS_TOKEN_IDENTIFIER, MS_TOKEN_STRING, MS_TOKEN_NUMBER,
    MS_TOKEN_AND, MS_TOKEN_CLASS, MS_TOKEN_ELSE, MS_TOKEN_FALSE,
    MS_TOKEN_FN, MS_TOKEN_FOR, MS_TOKEN_IF, MS_TOKEN_NIL,
    MS_TOKEN_OR, MS_TOKEN_PRINT, MS_TOKEN_RETURN, MS_TOKEN_SUPER,
    MS_TOKEN_THIS, MS_TOKEN_TRUE, MS_TOKEN_VAR, MS_TOKEN_WHILE,
    MS_TOKEN_BREAK, MS_TOKEN_CONTINUE,
    MS_TOKEN_IMPORT, MS_TOKEN_FROM, MS_TOKEN_AS,
    MS_TOKEN_ERROR, MS_TOKEN_EOF
} MsTokenType;

#endif
```

Design notes:
- Single-character tokens first, then compound operators, then literals, then keywords, then special tokens at end
- Enum values start at 0 and are sequential (C default)
- `MS_TOKEN_ERROR` carries error messages as the lexeme
- `MS_TOKEN_EOF` marks end of input

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_token
```
Expected: compiles and test passes

**REFACTOR**: No changes needed.

### Cycle 2: MsToken Struct — All Fields Accessible

**RED** — Write failing test:
- Add a test that creates an `MsToken` instance and verifies all fields:

```c
static void test_token_struct(void) {
    MsToken tok;
    tok.type = MS_TOKEN_NUMBER;
    tok.start = "42";
    tok.length = 2;
    tok.line = 1;
    tok.column = 5;

    if (tok.type != MS_TOKEN_NUMBER) {
        fprintf(stderr, "FAIL: type field not set correctly\n");
        exit(1);
    }
    if (tok.start[0] != '4' || tok.start[1] != '2') {
        fprintf(stderr, "FAIL: start field not set correctly\n");
        exit(1);
    }
    if (tok.length != 2) {
        fprintf(stderr, "FAIL: length field not set correctly\n");
        exit(1);
    }
    if (tok.line != 1) {
        fprintf(stderr, "FAIL: line field not set correctly\n");
        exit(1);
    }
    if (tok.column != 5) {
        fprintf(stderr, "FAIL: column field not set correctly\n");
        exit(1);
    }
}
```

- `MsToken` struct is not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MsToken` undeclared (the struct doesn't exist yet)

**GREEN** — Minimal implementation:
- Add `MsToken` struct to `src/token.h`:

```c
typedef struct {
    MsTokenType type;
    const char* start;
    int length;
    int line;
    int column;
} MsToken;
```

Design notes:
- `MsToken` stores a pointer + length into the original source string (no allocation)
- `line` and `column` track position for error messages
- `start` is a non-owning pointer — points into source code buffer

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_token
```
Expected: compiles and test passes

**REFACTOR**: No changes needed.

### Cycle 3: Enum Value Ordering and Sizeof Checks

**RED** — Write failing test:
- Add tests that verify enum ordering assumptions and struct size:

```c
static void test_enum_ordering(void) {
    if (MS_TOKEN_LEFT_PAREN != 0) {
        fprintf(stderr, "FAIL: MS_TOKEN_LEFT_PAREN should be 0\n");
        exit(1);
    }
    if (MS_TOKEN_BANG_EQUAL <= MS_TOKEN_BANG) {
        fprintf(stderr, "FAIL: compound operators should come after single-char\n");
        exit(1);
    }
    if (MS_TOKEN_ERROR >= MS_TOKEN_EOF) {
        fprintf(stderr, "FAIL: EOF should be last token\n");
        exit(1);
    }
    if (MS_TOKEN_LEFT_BRACE != MS_TOKEN_RIGHT_PAREN + 1) {
        fprintf(stderr, "FAIL: tokens should be sequential\n");
        exit(1);
    }
}

static void test_token_size(void) {
    if (sizeof(MsToken) > 32) {
        fprintf(stderr, "FAIL: MsToken is unexpectedly large: %zu bytes\n", sizeof(MsToken));
        exit(1);
    }
    if (sizeof(MsToken) < sizeof(MsTokenType) + sizeof(const char*) + sizeof(int) * 3) {
        fprintf(stderr, "FAIL: MsToken is too small, fields may be missing\n");
        exit(1);
    }
}
```

**Verify RED**: 
```
cmake --build build
ctest --test-dir build -R test_token
```
Expected: should already pass — the enum from Cycle 1 already has correct ordering.

**GREEN** — No changes needed. The implementation from Cycles 1-2 already satisfies these checks.

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_token
```
Expected: all tests pass. `sizeof(MsToken)` is typically 24 bytes on 64-bit (1 enum + 1 pointer + 3 ints, with padding).

**REFACTOR**: Verify `sizeof(MsToken)` is 24 bytes on 64-bit systems (enum=4, pointer=8, int*3=12, with padding to 24). This is reasonable and no packing pragmas are needed.

## Acceptance Criteria

- [ ] `token.h` compiles when included from a `.c` file
- [ ] `MsTokenType` enum contains all 49 token types from the design
- [ ] `MsToken` struct has fields: `type`, `start`, `length`, `line`, `column`
- [ ] `sizeof(MsToken)` is reasonable (typically 24-32 bytes on 64-bit)
- [ ] Enum values start at 0 and are sequential (C default)

## Notes

- This is a header-only module — no `.c` file needed.
- Token types follow a deliberate ordering: single-char → compound operators → literals → keywords → special (`ERROR`, `EOF`).
- `MsToken.start` is a non-owning pointer into the source string. The scanner creates tokens without allocating memory for the lexeme.
