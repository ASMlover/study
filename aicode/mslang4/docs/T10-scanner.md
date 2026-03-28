# T10: Scanner (Lexer)

**Phase**: 3 - Scanner
**Dependencies**: T06 (Token Types)
**Estimated Complexity**: High

## Goal

Implement the lexical analyzer that converts source code text into a stream of tokens. Handles whitespace, comments, string/number literals, identifiers, keywords, and all operators.

## Files to Create

| File | Purpose |
|------|---------|
| `src/scanner.h` | Scanner struct and API |
| `src/scanner.c` | Full lexer implementation |

## TDD Implementation Cycles

### Cycle 1: Scanner Init and EOF Token

**RED** — Write failing test:

Create `tests/unit/test_scanner.c`. Write a test function `test_empty_input` that:
- Initializes a scanner with an empty string `""`
- Scans a token and verifies it is `MS_TOKEN_EOF`
- Verifies `line == 1` and `column == 1`

```c
// tests/unit/test_scanner.c (initial skeleton)
#include "scanner.h"
#include "token.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void assert_token(MsToken tok, MsTokenType type, const char* lexeme, int line) {
    assert(tok.type == type);
    assert(tok.length == (int)strlen(lexeme));
    assert(strncmp(tok.start, lexeme, tok.length) == 0);
    assert(tok.line == line);
}

static void test_empty_input(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "");
    MsToken tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_EOF);
    assert(tok.line == 1);
    printf("  test_empty_input PASSED\n");
}

int main(void) {
    printf("Running scanner tests...\n");
    test_empty_input();
    printf("All scanner tests passed.\n");
    return 0;
}
```

**Verify RED**: `gcc -I src -o build/test_scanner tests/unit/test_scanner.c src/scanner.c` → compilation error: `scanner.h: No such file or directory`

**GREEN** — Minimal implementation:

Create `src/scanner.h`:

```c
#ifndef MS_SCANNER_H
#define MS_SCANNER_H

#include "token.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
    int column;
} MsScanner;

void ms_scanner_init(MsScanner* scanner, const char* source);
MsToken ms_scanner_scan_token(MsScanner* scanner);

#endif
```

Create `src/scanner.c` with init and scan_token returning EOF:

```c
#include "scanner.h"
#include "common.h"
#include <string.h>

static MsToken ms_make_token(MsScanner* scanner, MsTokenType type) {
    MsToken tok;
    tok.type = type;
    tok.start = scanner->start;
    tok.length = (int)(scanner->current - scanner->start);
    tok.line = scanner->line;
    tok.column = scanner->column;
    return tok;
}

void ms_scanner_init(MsScanner* scanner, const char* source) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
    scanner->column = 1;
}

MsToken ms_scanner_scan_token(MsScanner* scanner) {
    return ms_make_token(scanner, MS_TOKEN_EOF);
}
```

`ms_scanner_init()`: Set start=current=source, line=1, column=1.

**Verify GREEN**: `gcc -I src -o build/test_scanner tests/unit/test_scanner.c src/scanner.c && ./build/test_scanner` → test passes

**REFACTOR**: None needed.

---

### Cycle 2: Single-Character Tokens

**RED** — Write failing test:

Add `test_single_char_tokens` to `test_scanner.c`:

```c
static void test_single_char_tokens(void) {
    struct { const char* source; MsTokenType type; } cases[] = {
        {"(", MS_TOKEN_LEFT_PAREN},
        {")", MS_TOKEN_RIGHT_PAREN},
        {"{", MS_TOKEN_LEFT_BRACE},
        {"}", MS_TOKEN_RIGHT_BRACE},
        {"[", MS_TOKEN_LEFT_BRACKET},
        {"]", MS_TOKEN_RIGHT_BRACKET},
        {";", MS_TOKEN_SEMICOLON},
        {":", MS_TOKEN_COLON},
        {",", MS_TOKEN_COMMA},
        {".", MS_TOKEN_DOT},
        {"+", MS_TOKEN_PLUS},
        {"-", MS_TOKEN_MINUS},
        {"*", MS_TOKEN_STAR},
        {"/", MS_TOKEN_SLASH},
        {"%", MS_TOKEN_PERCENT},
        {"!", MS_TOKEN_BANG},
        {"=", MS_TOKEN_EQUAL},
        {"<", MS_TOKEN_LESS},
        {">", MS_TOKEN_GREATER},
    };
    int n = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < n; i++) {
        MsScanner scanner;
        ms_scanner_init(&scanner, cases[i].source);
        MsToken tok = ms_scanner_scan_token(&scanner);
        assert(tok.type == cases[i].type);
    }
    printf("  test_single_char_tokens PASSED\n");
}
```

**Verify RED**: Build and run — tests fail because `ms_scanner_scan_token` always returns EOF.

**GREEN** — Minimal implementation:

Update `src/scanner.c` to dispatch on first character:

```c
static char ms_scanner_advance(MsScanner* scanner) {
    char c = *scanner->current;
    scanner->current++;
    scanner->column++;
    return c;
}

static char ms_scanner_peek(MsScanner* scanner) {
    return *scanner->current;
}

static MsToken ms_scanner_scan_token(MsScanner* scanner) {
    char c = ms_scanner_advance(scanner);
    switch (c) {
        case '\0': return ms_make_token(scanner, MS_TOKEN_EOF);
        case '(': return ms_make_token(scanner, MS_TOKEN_LEFT_PAREN);
        case ')': return ms_make_token(scanner, MS_TOKEN_RIGHT_PAREN);
        case '{': return ms_make_token(scanner, MS_TOKEN_LEFT_BRACE);
        case '}': return ms_make_token(scanner, MS_TOKEN_RIGHT_BRACE);
        case '[': return ms_make_token(scanner, MS_TOKEN_LEFT_BRACKET);
        case ']': return ms_make_token(scanner, MS_TOKEN_RIGHT_BRACKET);
        case ';': return ms_make_token(scanner, MS_TOKEN_SEMICOLON);
        case ':': return ms_make_token(scanner, MS_TOKEN_COLON);
        case ',': return ms_make_token(scanner, MS_TOKEN_COMMA);
        case '.': return ms_make_token(scanner, MS_TOKEN_DOT);
        case '+': return ms_make_token(scanner, MS_TOKEN_PLUS);
        case '-': return ms_make_token(scanner, MS_TOKEN_MINUS);
        case '*': return ms_make_token(scanner, MS_TOKEN_STAR);
        case '/': return ms_make_token(scanner, MS_TOKEN_SLASH);
        case '%': return ms_make_token(scanner, MS_TOKEN_PERCENT);
        case '!': return ms_make_token(scanner, MS_TOKEN_BANG);
        case '=': return ms_make_token(scanner, MS_TOKEN_EQUAL);
        case '<': return ms_make_token(scanner, MS_TOKEN_LESS);
        case '>': return ms_make_token(scanner, MS_TOKEN_GREATER);
        default:  return ms_make_token(scanner, MS_TOKEN_ERROR);
    }
}
```

Note: Adjust `ms_make_token` to use `scanner->start` before advance, or restructure so start points to the beginning of the token. Refactor: set `scanner->start = scanner->current` before advancing.

**Verify GREEN**: Build and run — both tests pass.

**REFACTOR**: Ensure `ms_make_token` uses the correct `start` pointer. Update `ms_scanner_scan_token` to set `scanner->start = scanner->current` at the beginning before advancing.

---

### Cycle 3: Two-Character Tokens (==, !=, <=, >=)

**RED** — Write failing test:

Add `test_two_char_tokens` to `test_scanner.c`:

```c
static void test_two_char_tokens(void) {
    struct { const char* source; MsTokenType type; } cases[] = {
        {"==", MS_TOKEN_EQUAL_EQUAL},
        {"!=", MS_TOKEN_BANG_EQUAL},
        {"<=", MS_TOKEN_LESS_EQUAL},
        {">=", MS_TOKEN_GREATER_EQUAL},
    };
    int n = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < n; i++) {
        MsScanner scanner;
        ms_scanner_init(&scanner, cases[i].source);
        MsToken tok = ms_scanner_scan_token(&scanner);
        assert(tok.type == cases[i].type);
    }
    printf("  test_two_char_tokens PASSED\n");
}
```

**Verify RED**: Build and run — `==` scans as EQUAL followed by EQUAL, not as EQUAL_EQUAL. Tests fail.

**GREEN** — Minimal implementation:

Add `match` helper and update the switch cases:

```c
static bool ms_scanner_match(MsScanner* scanner, char expected) {
    if (*scanner->current == '\0') return false;
    if (*scanner->current != expected) return false;
    scanner->current++;
    scanner->column++;
    return true;
}
```

Update the switch in `ms_scanner_scan_token` for `!`, `=`, `<`, `>`:

```c
        case '!': return ms_make_token(scanner,
            ms_scanner_match(scanner, '=') ? MS_TOKEN_BANG_EQUAL : MS_TOKEN_BANG);
        case '=': return ms_make_token(scanner,
            ms_scanner_match(scanner, '=') ? MS_TOKEN_EQUAL_EQUAL : MS_TOKEN_EQUAL);
        case '<': return ms_make_token(scanner,
            ms_scanner_match(scanner, '=') ? MS_TOKEN_LESS_EQUAL : MS_TOKEN_LESS);
        case '>': return ms_make_token(scanner,
            ms_scanner_match(scanner, '=') ? MS_TOKEN_GREATER_EQUAL : MS_TOKEN_GREATER);
```

**Verify GREEN**: Build and run — all three tests pass.

**REFACTOR**: None needed.

---

### Cycle 4: Number Literals

**RED** — Write failing test:

Add `test_number_literals` to `test_scanner.c`:

```c
static void test_number_literals(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "123");
    MsToken tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_NUMBER);
    assert(tok.length == 3);
    assert(strncmp(tok.start, "123", 3) == 0);

    ms_scanner_init(&scanner, "3.14");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_NUMBER);
    assert(tok.length == 4);
    assert(strncmp(tok.start, "3.14", 4) == 0);

    printf("  test_number_literals PASSED\n");
}
```

**Verify RED**: Build and run — `1` is not recognized, returns ERROR. Tests fail.

**GREEN** — Minimal implementation:

Add `scanNumber` helper to `src/scanner.c`:

```c
static bool ms_is_digit(char c) { return c >= '0' && c <= '9'; }

static MsToken ms_scan_number(MsScanner* scanner) {
    while (ms_is_digit(ms_scanner_peek(scanner))) ms_scanner_advance(scanner);
    if (ms_scanner_peek(scanner) == '.' && ms_is_digit(*(scanner->current + 1))) {
        ms_scanner_advance(scanner);
        while (ms_is_digit(ms_scanner_peek(scanner))) ms_scanner_advance(scanner);
    }
    return ms_make_token(scanner, MS_TOKEN_NUMBER);
}
```

Add to the switch: `default: if (ms_is_digit(c)) return ms_scan_number(scanner);`

**Verify GREEN**: Build and run — all four tests pass.

**REFACTOR**: None needed.

---

### Cycle 5: String Literals with Escapes

**RED** — Write failing test:

Add `test_string_literals` to `test_scanner.c`:

```c
static void test_string_literals(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "\"hello\"");
    MsToken tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_STRING);
    assert(tok.length == 7); /* includes quotes in lexeme */

    /* Escaped quote */
    ms_scanner_init(&scanner, "\"a\\\"b\"");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_STRING);

    /* Escape sequences: \\ \n \t \" */
    ms_scanner_init(&scanner, "\"\\n\\t\\\\\"");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_STRING);

    printf("  test_string_literals PASSED\n");
}
```

**Verify RED**: Build and run — `"` is not recognized as string start. Tests fail.

**GREEN** — Minimal implementation:

Add `scanString` helper to `src/scanner.c`:

```c
static MsToken ms_scan_string(MsScanner* scanner) {
    while (ms_scanner_peek(scanner) != '"' && !ms_scanner_at_end(scanner)) {
        if (ms_scanner_peek(scanner) == '\n') {
            scanner->line++;
            scanner->column = 0;
        }
        if (ms_scanner_peek(scanner) == '\\') ms_scanner_advance(scanner);
        ms_scanner_advance(scanner);
    }
    if (ms_scanner_at_end(scanner)) return ms_error_token(scanner, "Unterminated string.");
    ms_scanner_advance(scanner); /* closing " */
    return ms_make_token(scanner, MS_TOKEN_STRING);
}
```

Add to the switch: `case '"': return ms_scan_string(scanner);`

Handle `\n`, `\t`, `\\`, `\"` escapes. Return `MS_TOKEN_STRING` with lexeme pointing into source (including quotes).

**Verify GREEN**: Build and run — all five tests pass.

**REFACTOR**: None needed.

---

### Cycle 6: Identifiers

**RED** — Write failing test:

Add `test_identifiers` to `test_scanner.c`:

```c
static void test_identifiers(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "myVar");
    MsToken tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_IDENTIFIER);
    assert(tok.length == 5);
    assert(strncmp(tok.start, "myVar", 5) == 0);

    ms_scanner_init(&scanner, "_foo_bar2");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_IDENTIFIER);
    assert(tok.length == 9);

    printf("  test_identifiers PASSED\n");
}
```

**Verify RED**: Build and run — identifiers not recognized, return ERROR.

**GREEN** — Minimal implementation:

Add helpers to `src/scanner.c`:

```c
static bool ms_is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool ms_is_alphanumeric(char c) {
    return ms_is_alpha(c) || ms_is_digit(c);
}

static MsToken ms_scan_identifier(MsScanner* scanner) {
    while (ms_is_alphanumeric(ms_scanner_peek(scanner))) ms_scanner_advance(scanner);
    return ms_make_token(scanner, MS_TOKEN_IDENTIFIER);
}
```

Update the switch default:
```c
    default:
        if (ms_is_digit(c)) return ms_scan_number(scanner);
        if (ms_is_alpha(c)) return ms_scan_identifier(scanner);
        return ms_error_token(scanner, "Unexpected character.");
```

Collect alphanumeric + underscore chars. Return `MS_TOKEN_IDENTIFIER`.

**Verify GREEN**: Build and run — all six tests pass.

**REFACTOR**: None needed.

---

### Cycle 7: Keywords

**RED** — Write failing test:

Add `test_keywords` to `test_scanner.c`:

```c
static void test_keywords(void) {
    struct { const char* word; MsTokenType type; } keywords[] = {
        {"and", MS_TOKEN_AND}, {"class", MS_TOKEN_CLASS}, {"else", MS_TOKEN_ELSE},
        {"false", MS_TOKEN_FALSE}, {"fn", MS_TOKEN_FN}, {"for", MS_TOKEN_FOR},
        {"if", MS_TOKEN_IF}, {"nil", MS_TOKEN_NIL}, {"or", MS_TOKEN_OR},
        {"print", MS_TOKEN_PRINT}, {"return", MS_TOKEN_RETURN}, {"super", MS_TOKEN_SUPER},
        {"this", MS_TOKEN_THIS}, {"true", MS_TOKEN_TRUE}, {"var", MS_TOKEN_VAR},
        {"while", MS_TOKEN_WHILE}, {"break", MS_TOKEN_BREAK}, {"continue", MS_TOKEN_CONTINUE},
        {"import", MS_TOKEN_IMPORT}, {"from", MS_TOKEN_FROM}, {"as", MS_TOKEN_AS},
    };
    int n = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < n; i++) {
        MsScanner scanner;
        ms_scanner_init(&scanner, keywords[i].word);
        MsToken tok = ms_scanner_scan_token(&scanner);
        assert(tok.type == keywords[i].type);
    }
    printf("  test_keywords PASSED\n");
}
```

**Verify RED**: Build and run — all keywords scan as IDENTIFIER, not their keyword type. Tests fail.

**GREEN** — Minimal implementation:

Add `identifierType` and `checkKeyword` helpers to `src/scanner.c`:

```c
static MsTokenType ms_check_keyword(MsScanner* scanner, int start, int length,
                                     const char* rest, MsTokenType type) {
    if (scanner->current - scanner->start == start + length &&
        memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }
    return MS_TOKEN_IDENTIFIER;
}

static MsTokenType ms_identifier_type(MsScanner* scanner) {
    switch (scanner->start[0]) {
        case 'a': {
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'n': return ms_check_keyword(scanner, 2, 1, "d", MS_TOKEN_AND);
                    case 's': return ms_check_keyword(scanner, 2, 0, "", MS_TOKEN_AS);
                }
            }
            break;
        }
        case 'b': return ms_check_keyword(scanner, 1, 4, "reak", MS_TOKEN_BREAK);
        case 'c': {
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'l': return ms_check_keyword(scanner, 2, 3, "ass", MS_TOKEN_CLASS);
                    case 'o': return ms_check_keyword(scanner, 2, 6, "ntinue", MS_TOKEN_CONTINUE);
                }
            }
            break;
        }
        case 'e': return ms_check_keyword(scanner, 1, 3, "lse", MS_TOKEN_ELSE);
        case 'f': {
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'a': return ms_check_keyword(scanner, 2, 3, "lse", MS_TOKEN_FALSE);
                    case 'n': return ms_check_keyword(scanner, 2, 0, "", MS_TOKEN_FN);
                    case 'o': return ms_check_keyword(scanner, 2, 1, "r", MS_TOKEN_FOR);
                    case 'r': return ms_check_keyword(scanner, 2, 2, "om", MS_TOKEN_FROM);
                }
            }
            break;
        }
        case 'i': {
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'f': return ms_check_keyword(scanner, 2, 0, "", MS_TOKEN_IF);
                    case 'm': return ms_check_keyword(scanner, 2, 4, "port", MS_TOKEN_IMPORT);
                }
            }
            break;
        }
        case 'n': return ms_check_keyword(scanner, 1, 2, "il", MS_TOKEN_NIL);
        case 'o': return ms_check_keyword(scanner, 1, 1, "r", MS_TOKEN_OR);
        case 'p': return ms_check_keyword(scanner, 1, 4, "rint", MS_TOKEN_PRINT);
        case 'r': return ms_check_keyword(scanner, 1, 5, "eturn", MS_TOKEN_RETURN);
        case 's': return ms_check_keyword(scanner, 1, 4, "uper", MS_TOKEN_SUPER);
        case 't': {
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'h': return ms_check_keyword(scanner, 2, 2, "is", MS_TOKEN_THIS);
                    case 'r': return ms_check_keyword(scanner, 2, 2, "ue", MS_TOKEN_TRUE);
                }
            }
            break;
        }
        case 'v': return ms_check_keyword(scanner, 1, 2, "ar", MS_TOKEN_VAR);
        case 'w': return ms_check_keyword(scanner, 1, 4, "hile", MS_TOKEN_WHILE);
    }
    return MS_TOKEN_IDENTIFIER;
}
```

Update `ms_scan_identifier` to call `ms_identifier_type`:

```c
static MsToken ms_scan_identifier(MsScanner* scanner) {
    while (ms_is_alphanumeric(ms_scanner_peek(scanner))) ms_scanner_advance(scanner);
    return ms_make_token(scanner, ms_identifier_type(scanner));
}
```

Use `checkKeyword` trie approach for efficient keyword matching. Keywords: and, class, else, false, fn, for, if, nil, or, print, return, super, this, true, var, while, break, continue, import, from, as.

**Verify GREEN**: Build and run — all seven tests pass.

**REFACTOR**: None needed.

---

### Cycle 8: Whitespace and Comments

**RED** — Write failing test:

Add `test_whitespace_comments` to `test_scanner.c`:

```c
static void test_whitespace_comments(void) {
    /* Line comment */
    MsScanner scanner;
    ms_scanner_init(&scanner, "// comment\n42");
    MsToken tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_NUMBER);
    assert(tok.line == 2);

    /* Block comment */
    ms_scanner_init(&scanner, "/* block */ 42");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_NUMBER);

    /* Whitespace skipping */
    ms_scanner_init(&scanner, "   \t\r\n  42");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_NUMBER);
    assert(tok.line == 2);

    printf("  test_whitespace_comments PASSED\n");
}
```

**Verify RED**: Build and run — `//` scans as SLASH SLASH (not a comment). Tests fail.

**GREEN** — Minimal implementation:

Add `skipWhitespace` to `src/scanner.c`:

```c
static void ms_skip_whitespace(MsScanner* scanner) {
    for (;;) {
        char c = ms_scanner_peek(scanner);
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                ms_scanner_advance(scanner);
                break;
            case '\n':
                scanner->line++;
                scanner->column = 0;
                ms_scanner_advance(scanner);
                break;
            case '/':
                if (*(scanner->current + 1) == '/') {
                    while (ms_scanner_peek(scanner) != '\n' && !ms_scanner_at_end(scanner))
                        ms_scanner_advance(scanner);
                } else if (*(scanner->current + 1) == '*') {
                    ms_scanner_advance(scanner);
                    ms_scanner_advance(scanner);
                    while (!ms_scanner_at_end(scanner)) {
                        if (ms_scanner_peek(scanner) == '\n') {
                            scanner->line++;
                            scanner->column = 0;
                        }
                        if (ms_scanner_peek(scanner) == '*' && *(scanner->current + 1) == '/') {
                            ms_scanner_advance(scanner);
                            ms_scanner_advance(scanner);
                            break;
                        }
                        ms_scanner_advance(scanner);
                    }
                } else {
                    return;
                }
                break;
            default: return;
        }
    }
}
```

Update `ms_scanner_scan_token` to call `ms_skip_whitespace` first and update `scanner->start`:

```c
MsToken ms_scanner_scan_token(MsScanner* scanner) {
    ms_skip_whitespace(scanner);
    scanner->start = scanner->current;
    if (*scanner->current == '\0') return ms_make_token(scanner, MS_TOKEN_EOF);
    /* ... rest of dispatch ... */
}
```

Handle `//` line comments (skip to end of line) and `/* */` block comments. Handle newlines (increment line, reset column).

**Verify GREEN**: Build and run — all eight tests pass.

**REFACTOR**: None needed.

---

### Cycle 9: Multi-Token Programs and Line/Column Tracking

**RED** — Write failing test:

Add `test_multi_token` and `test_line_column` to `test_scanner.c`:

```c
static MsToken scan_next(MsScanner* scanner) {
    return ms_scanner_scan_token(scanner);
}

static void test_multi_token(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "var x = 42");
    MsToken tok;

    tok = scan_next(&scanner);
    assert(tok.type == MS_TOKEN_VAR);
    tok = scan_next(&scanner);
    assert(tok.type == MS_TOKEN_IDENTIFIER);
    assert(strncmp(tok.start, "x", 1) == 0);
    tok = scan_next(&scanner);
    assert(tok.type == MS_TOKEN_EQUAL);
    tok = scan_next(&scanner);
    assert(tok.type == MS_TOKEN_NUMBER);
    assert(strncmp(tok.start, "42", 2) == 0);
    tok = scan_next(&scanner);
    assert(tok.type == MS_TOKEN_EOF);

    printf("  test_multi_token PASSED\n");
}

static void test_line_column(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "1\n+\n2");
    MsToken tok;

    tok = scan_next(&scanner);
    assert(tok.line == 1);
    tok = scan_next(&scanner);
    assert(tok.line == 2);
    assert(tok.type == MS_TOKEN_PLUS);
    tok = scan_next(&scanner);
    assert(tok.line == 3);

    printf("  test_line_column PASSED\n");
}
```

**Verify RED**: Build and run — should pass if previous cycles are correct. This is a verification cycle for multi-token scanning and line tracking.

**Verify GREEN**: Build and run — all ten tests pass.

**REFACTOR**: Clean up helper functions, ensure `ms_scanner_at_end` helper exists:

```c
static bool ms_scanner_at_end(MsScanner* scanner) {
    return *scanner->current == '\0';
}
```

---

### Cycle 10: Error Tokens

**RED** — Write failing test:

Add `test_error_tokens` to `test_scanner.c`:

```c
static void test_error_tokens(void) {
    MsScanner scanner;
    ms_scanner_init(&scanner, "@");
    MsToken tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_ERROR);

    ms_scanner_init(&scanner, "#");
    tok = ms_scanner_scan_token(&scanner);
    assert(tok.type == MS_TOKEN_ERROR);

    printf("  test_error_tokens PASSED\n");
}
```

**Verify RED**: Build and run — `@` should already return ERROR from the default case. This verifies error handling.

**Verify GREEN**: Build and run — all eleven tests pass.

**REFACTOR**: Ensure `ms_error_token` helper exists:

```c
static MsToken ms_error_token(MsScanner* scanner, const char* message) {
    MsToken tok;
    tok.type = MS_TOKEN_ERROR;
    tok.start = message;
    tok.length = (int)strlen(message);
    tok.line = scanner->line;
    tok.column = scanner->column;
    return tok;
}
```

## Acceptance Criteria

- [ ] `ms_scanner_init` + `ms_scanner_scan_token` until EOF for `"123"` produces: NUMBER(123), EOF
- [ ] `"var x = 42"` produces: VAR, IDENTIFIER("x"), EQUAL, NUMBER(42), EOF
- [ ] `"1 + 2 * 3"` produces: NUMBER(1), PLUS, NUMBER(2), STAR, NUMBER(3), EOF
- [ ] `"\"hello\""` produces: STRING("hello"), EOF
- [ ] All keywords are recognized (and, class, else, false, fn, for, if, nil, or, print, return, super, this, true, var, while, break, continue, import, from, as)
- [ ] `==`, `!=`, `<=`, `>=` produce two-char tokens
- [ ] `// comment\n42` skips comment, produces NUMBER(42)
- [ ] `/* block */ 42` skips block comment
- [ ] Unrecognized character produces MS_TOKEN_ERROR
- [ ] Multi-line tracking: line numbers increment on newlines
- [ ] Column numbers track position on each line

## Notes

- Token creation: Set `start` to start of lexeme, `length` = current-start, `line` and `column` from scanner state.
- All internal helpers (`advance`, `peek`, `peekNext`, `match`, `skipWhitespace`, `scanString`, `scanNumber`, `scanIdentifier`, `identifierType`, `checkKeyword`) are `static` in `scanner.c`.
- The scanner does not allocate memory — all lexemes point into the source string.
- Column tracking is approximate for multi-byte characters; exact column tracking is not required.
