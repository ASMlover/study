# T05: Debug Logger

**Phase**: 1 - Foundation
**Dependencies**: T03 (Memory Subsystem), T04 (Platform Layer)
**Estimated Complexity**: Medium

## Goal

Implement a configurable logging system with colored output, multiple levels, source location tracking, and compile-time filtering. Used throughout the codebase for debugging and diagnostics.

## Files to Create

| File | Purpose |
|------|---------|
| `src/logger.h` | Logger API, level enum, convenience macros |
| `src/logger.c` | Logger implementation |
| `tests/unit/test_logger.c` | Unit tests for logger |

## TDD Implementation Cycles

### Cycle 1: MsLogLevel Enum and Basic Log Function

**RED** — Write failing test:
- Create `tests/unit/test_logger.c` that verifies the logger can output a message:

```c
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_log_basic(void) {
    FILE* tmp = tmpfile();
    if (tmp == NULL) {
        fprintf(stderr, "FAIL: tmpfile() returned NULL\n");
        exit(1);
    }

    ms_logger_set_output(tmp);
    ms_logger_enable_colors(false);
    ms_logger_enable_timestamp(false);
    ms_logger_set_level(MS_LOG_TRACE);

    ms_logger_log(MS_LOG_INFO, "test.c", 42, "test_func", "hello %s", "world");

    fflush(tmp);
    rewind(tmp);

    char buf[512] = {0};
    fread(buf, 1, sizeof(buf) - 1, tmp);
    fclose(tmp);

    if (strstr(buf, "INFO") == NULL) {
        fprintf(stderr, "FAIL: output missing 'INFO', got: %s\n", buf);
        exit(1);
    }
    if (strstr(buf, "test.c:42") == NULL) {
        fprintf(stderr, "FAIL: output missing 'test.c:42', got: %s\n", buf);
        exit(1);
    }
    if (strstr(buf, "test_func") == NULL) {
        fprintf(stderr, "FAIL: output missing 'test_func', got: %s\n", buf);
        exit(1);
    }
    if (strstr(buf, "hello world") == NULL) {
        fprintf(stderr, "FAIL: output missing 'hello world', got: %s\n", buf);
        exit(1);
    }
}

int main(void) {
    test_log_basic();
    printf("test_log_basic passed\n");
    return 0;
}
```

- Add to `tests/CMakeLists.txt`:
```cmake
add_executable(test_logger tests/unit/test_logger.c)
target_include_directories(test_logger PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_logger PRIVATE maple)
add_test(NAME test_logger COMMAND test_logger)
```

- `logger.h` and `logger.c` do not exist yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `logger.h` not found

**GREEN** — Minimal implementation:
- Create `src/logger.h`:

```c
#ifndef MS_LOGGER_H
#define MS_LOGGER_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    MS_LOG_TRACE, MS_LOG_DEBUG, MS_LOG_INFO,
    MS_LOG_WARN, MS_LOG_ERROR, MS_LOG_FATAL, MS_LOG_OFF
} MsLogLevel;

void ms_logger_set_level(MsLogLevel level);
void ms_logger_set_output(FILE* stream);
void ms_logger_enable_colors(bool enable);
void ms_logger_enable_timestamp(bool enable);
void ms_logger_log(MsLogLevel level, const char* file, int line,
                   const char* func, const char* fmt, ...);

#endif
```

- Create `src/logger.c`:

```c
#include "logger.h"
#include "platform.h"
#include <stdarg.h>
#include <string.h>
#include <time.h>

static MsLogLevel current_level = MS_LOG_INFO;
static FILE* output_stream = NULL;
static bool colors_enabled = false;
static bool timestamps_enabled = false;

void ms_logger_set_level(MsLogLevel level) {
    current_level = level;
}

void ms_logger_set_output(FILE* stream) {
    output_stream = stream;
}

void ms_logger_enable_colors(bool enable) {
    colors_enabled = enable;
}

void ms_logger_enable_timestamp(bool enable) {
    timestamps_enabled = enable;
}

static const char* level_tag(MsLogLevel level) {
    switch (level) {
        case MS_LOG_TRACE: return "TRACE";
        case MS_LOG_DEBUG: return "DEBUG";
        case MS_LOG_INFO:  return "INFO";
        case MS_LOG_WARN:  return "WARN";
        case MS_LOG_ERROR: return "ERROR";
        case MS_LOG_FATAL: return "FATAL";
        case MS_LOG_OFF:   return "OFF";
    }
    return "UNKNOWN";
}

void ms_logger_log(MsLogLevel level, const char* file, int line,
                   const char* func, const char* fmt, ...) {
    if (level < current_level) return;

    FILE* out = output_stream ? output_stream : stderr;

    if (timestamps_enabled) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        fprintf(out, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    }

    fprintf(out, "[%s] %s:%d@%s - ", level_tag(level), file, line, func);

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    fprintf(out, "\n");
}
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: compiles and test passes

**REFACTOR**: Logger uses static (file-scoped) globals for state. This is the one exception to the "no globals" rule since the logger is a cross-cutting concern.

### Cycle 2: Level Filtering

**RED** — Write failing test:
- Add a test that verifies level filtering suppresses lower-priority messages:

```c
static void test_level_filtering(void) {
    FILE* tmp = tmpfile();
    ms_logger_set_output(tmp);
    ms_logger_enable_colors(false);
    ms_logger_enable_timestamp(false);
    ms_logger_set_level(MS_LOG_WARN);

    ms_logger_log(MS_LOG_TRACE, "f.c", 1, "fn", "trace msg");
    ms_logger_log(MS_LOG_DEBUG, "f.c", 1, "fn", "debug msg");
    ms_logger_log(MS_LOG_INFO,  "f.c", 1, "fn", "info msg");
    ms_logger_log(MS_LOG_WARN,  "f.c", 1, "fn", "warn msg");
    ms_logger_log(MS_LOG_ERROR, "f.c", 1, "fn", "error msg");

    fflush(tmp);
    rewind(tmp);

    char buf[512] = {0};
    fread(buf, 1, sizeof(buf) - 1, tmp);
    fclose(tmp);

    if (strstr(buf, "trace msg") != NULL) {
        fprintf(stderr, "FAIL: TRACE should be suppressed at WARN level\n");
        exit(1);
    }
    if (strstr(buf, "debug msg") != NULL) {
        fprintf(stderr, "FAIL: DEBUG should be suppressed at WARN level\n");
        exit(1);
    }
    if (strstr(buf, "info msg") != NULL) {
        fprintf(stderr, "FAIL: INFO should be suppressed at WARN level\n");
        exit(1);
    }
    if (strstr(buf, "warn msg") == NULL) {
        fprintf(stderr, "FAIL: WARN should appear\n");
        exit(1);
    }
    if (strstr(buf, "error msg") == NULL) {
        fprintf(stderr, "FAIL: ERROR should appear\n");
        exit(1);
    }
}
```

**Verify RED**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: should already pass — the level filtering logic from Cycle 1 (`if (level < current_level) return;`) handles this.

**GREEN** — No changes needed. The implementation from Cycle 1 already supports level filtering.

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: test passes

**REFACTOR**: No changes needed. The enum ordering (`TRACE=0 < DEBUG=1 < INFO=2 < WARN=3 < ERROR=4 < FATAL=5 < OFF=6`) ensures numeric comparison works for filtering.

### Cycle 3: Convenience Macros

**RED** — Write failing test:
- Add a test that verifies convenience macros expand correctly and produce output:

```c
static void test_convenience_macros(void) {
    FILE* tmp = tmpfile();
    ms_logger_set_output(tmp);
    ms_logger_enable_colors(false);
    ms_logger_enable_timestamp(false);
    ms_logger_set_level(MS_LOG_TRACE);

    ms_logger_trace("trace %d", 1);
    ms_logger_debug("debug %d", 2);
    ms_logger_info("info %d", 3);
    ms_logger_warn("warn %d", 4);
    ms_logger_error("error %d", 5);
    ms_logger_fatal("fatal %d", 6);

    fflush(tmp);
    rewind(tmp);

    char buf[1024] = {0};
    fread(buf, 1, sizeof(buf) - 1, tmp);
    fclose(tmp);

    if (strstr(buf, "TRACE") == NULL || strstr(buf, "trace 1") == NULL) {
        fprintf(stderr, "FAIL: trace macro output missing\n");
        exit(1);
    }
    if (strstr(buf, "DEBUG") == NULL || strstr(buf, "debug 2") == NULL) {
        fprintf(stderr, "FAIL: debug macro output missing\n");
        exit(1);
    }
    if (strstr(buf, "INFO") == NULL || strstr(buf, "info 3") == NULL) {
        fprintf(stderr, "FAIL: info macro output missing\n");
        exit(1);
    }
    if (strstr(buf, "WARN") == NULL || strstr(buf, "warn 4") == NULL) {
        fprintf(stderr, "FAIL: warn macro output missing\n");
        exit(1);
    }
    if (strstr(buf, "ERROR") == NULL || strstr(buf, "error 5") == NULL) {
        fprintf(stderr, "FAIL: error macro output missing\n");
        exit(1);
    }
    if (strstr(buf, "FATAL") == NULL || strstr(buf, "fatal 6") == NULL) {
        fprintf(stderr, "FAIL: fatal macro output missing\n");
        exit(1);
    }
}
```

- `ms_logger_trace`, `ms_logger_debug`, etc. are not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `ms_logger_trace` undeclared (and others)

**GREEN** — Minimal implementation:
- Add convenience macros to `src/logger.h`:

```c
#define ms_logger_trace(...) ms_logger_log(MS_LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_debug(...) ms_logger_log(MS_LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_info(...)  ms_logger_log(MS_LOG_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_warn(...)  ms_logger_log(MS_LOG_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_error(...) ms_logger_log(MS_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_fatal(...) ms_logger_log(MS_LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: compiles and test passes

**REFACTOR**: No changes needed.

### Cycle 4: Colored Output

**RED** — Write failing test:
- Add a test that verifies colored output includes ANSI codes when enabled:

```c
static void test_colored_output(void) {
    FILE* tmp = tmpfile();
    ms_logger_set_output(tmp);
    ms_logger_enable_timestamp(false);
    ms_logger_set_level(MS_LOG_TRACE);

    ms_logger_enable_colors(true);
    ms_logger_log(MS_LOG_ERROR, "f.c", 1, "fn", "red error");

    fflush(tmp);
    rewind(tmp);

    char buf[512] = {0};
    fread(buf, 1, sizeof(buf) - 1, tmp);

    if (strstr(buf, "\033[") == NULL) {
        fprintf(stderr, "FAIL: colored output should contain ANSI escape codes\n");
        exit(1);
    }
    if (strstr(buf, "red error") == NULL) {
        fprintf(stderr, "FAIL: colored output should still contain message\n");
        exit(1);
    }

    fclose(tmp);
    ms_logger_enable_colors(false);
}
```

- The current implementation does not emit ANSI codes — test will fail because `strstr(buf, "\033[")` returns NULL.

**Verify RED**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: test fails — no ANSI escape codes in output

**GREEN** — Minimal implementation:
- Add color support to `src/logger.c`. Add a helper and update `ms_logger_log`:

```c
static const char* level_ansi(MsLogLevel level) {
    switch (level) {
        case MS_LOG_TRACE: return "\033[90m";
        case MS_LOG_DEBUG: return "\033[34m";
        case MS_LOG_INFO:  return "\033[32m";
        case MS_LOG_WARN:  return "\033[33m";
        case MS_LOG_ERROR: return "\033[31m";
        case MS_LOG_FATAL: return "\033[35m";
        case MS_LOG_OFF:   return "";
    }
    return "";
}
```

- Update `ms_logger_log` to emit color codes:

```c
void ms_logger_log(MsLogLevel level, const char* file, int line,
                   const char* func, const char* fmt, ...) {
    if (level < current_level) return;

    FILE* out = output_stream ? output_stream : stderr;

    if (colors_enabled) {
        fprintf(out, "%s", level_ansi(level));
    }

    if (timestamps_enabled) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        fprintf(out, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    }

    fprintf(out, "[%s] %s:%d@%s - ", level_tag(level), file, line, func);

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    fprintf(out, "\n");

    if (colors_enabled) {
        fprintf(out, "\033[0m");
    }
}
```

ANSI color codes for levels: TRACE=90(gray), DEBUG=34(blue), INFO=32(green), WARN=33(yellow), ERROR=31(red), FATAL=35(magenta).

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: compiles and test passes

**REFACTOR**: On Windows, `ms_platform_enable_console_colors()` must be called before color output works in terminal. For file output (tmpfile), ANSI codes are written raw.

### Cycle 5: Compile-Time Log Level Filtering

**RED** — Write failing test:
- This cycle verifies the compile-time `MS_LOG_LEVEL_*` macros exist and have correct values. Add a test:

```c
static void test_compile_time_level(void) {
    if (MS_LOG_LEVEL < 0 || MS_LOG_LEVEL > 6) {
        fprintf(stderr, "FAIL: MS_LOG_LEVEL out of range: %d\n", MS_LOG_LEVEL);
        exit(1);
    }
}
```

- `MS_LOG_LEVEL` is not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MS_LOG_LEVEL` undeclared

**GREEN** — Minimal implementation:
- Add compile-time level filtering to `src/logger.h`:

```c
#ifdef MS_LOG_LEVEL_TRACE
  #define MS_LOG_LEVEL 0
#elif defined(MS_LOG_LEVEL_DEBUG)
  #define MS_LOG_LEVEL 1
#elif defined(MS_LOG_LEVEL_INFO)
  #define MS_LOG_LEVEL 2
#elif defined(MS_LOG_LEVEL_WARN)
  #define MS_LOG_LEVEL 3
#elif defined(MS_LOG_LEVEL_ERROR)
  #define MS_LOG_LEVEL 4
#else
  #define MS_LOG_LEVEL 2
#endif
```

Default level is INFO (2). When `MS_LOG_LEVEL_TRACE` is defined via compiler flag, `MS_LOG_LEVEL` becomes 0, etc.

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_logger
```
Expected: compiles and test passes (default MS_LOG_LEVEL is 2)

**REFACTOR**: No changes needed. The compile-time filter can be used in future for `#if MS_LOG_LEVEL <= MS_LOG_DEBUG` guards around expensive debug code.

## Acceptance Criteria

- [ ] Logger compiles and links with platform layer
- [ ] `ms_logger_info("test")` outputs `[INFO] test` to stdout
- [ ] Setting level to `MS_LOG_WARN` suppresses TRACE/DEBUG/INFO messages
- [ ] Colored output works on Windows and POSIX terminals
- [ ] Timestamps can be toggled on/off
- [ ] Output can be redirected to a FILE* stream
- [ ] Compile-time `MS_LOG_LEVEL_*` filtering works
- [ ] All convenience macros expand correctly

## Notes

- Logger uses file-scoped static globals for state. This is the one accepted exception to the "no globals" rule.
- Log format: `[LEVEL] file:line@func - message\n` (with optional timestamp prefix `[HH:MM:SS]`).
- Uses `<stdarg.h>` for variadic args, `vsnprintf`/`vfprintf` for formatting.
- Timestamps via `time()` + `localtime()`. Millisecond precision can be added later.
