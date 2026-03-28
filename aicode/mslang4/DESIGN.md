# Maple Scripting Language - Design Specification

> **Convention**: All code blocks below omit `#ifndef`/`#define`/`#endif` include guards and `#include` directives for brevity. Every `.h` file uses them in practice.

## 1. Architecture

```
Source (.ms) → Scanner → Parser → Compiler → VM ←→ GC
```

Data flow: `"var x = 42"` → `[VAR, IDENTIFIER("x"), EQUAL, NUMBER(42)]` → `VarDecl{name:"x", init:Literal(42)}` → `[OP_CONSTANT, 0, OP_DEFINE_GLOBAL, 1]` → VM executes.

## 2. Component APIs

### 2.1 Common (`common.h`)

```c
#define MS_STACK_MAX    256
#define MS_FRAMES_MAX   64
#define MS_GC_HEAP_GROW_FACTOR 2
#define MS_MAX_LOCALS   256
#define MS_MAX_UPVALUES 256
#define MS_TABLE_MAX_LOAD 0.75

typedef enum { MS_OK, MS_COMPILE_ERROR, MS_RUNTIME_ERROR } MsResult;

#ifdef MS_DEBUG_LOG_GC
  #define MS_DEBUG_LOG_GC_EXECUTE(code) code
#else
  #define MS_DEBUG_LOG_GC_EXECUTE(code)
#endif

#ifdef MS_DEBUG_STRESS_GC
  #define MS_DEBUG_STRESS_GC_EXECUTE(code) code
#else
  #define MS_DEBUG_STRESS_GC_EXECUTE(code)
#endif

#ifdef MS_DEBUG_TRACE_EXECUTION
  #define MS_DEBUG_TRACE_EXECUTE(code) code
#else
  #define MS_DEBUG_TRACE_EXECUTE(code)
#endif
```

### 2.2 Tokens (`token.h`)

```c
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

typedef struct {
    MsTokenType type;
    const char* start;
    int length;
    int line;
    int column;
} MsToken;
```

### 2.3 Scanner (`scanner.h / .c`)

```c
typedef struct {
    const char* start;
    const char* current;
    int line;
    int column;
} MsScanner;

void ms_scanner_init(MsScanner* scanner, const char* source);
MsToken ms_scanner_scan_token(MsScanner* scanner);
```

Internal helpers (all `static`): `advance()`, `peek()`, `peekNext()`, `match()`, `skipWhitespace()`, `scanString()`, `scanNumber()`, `scanIdentifier()`, `identifierType()`, `checkKeyword()`. Token lexeme is pointer+length into original source (no allocation).

### 2.4 AST (`ast.h`)

```c
typedef struct MsExpr MsExpr;
typedef struct MsStmt MsStmt;

typedef enum {
    MS_EXPR_ASSIGN, MS_EXPR_BINARY, MS_EXPR_CALL, MS_EXPR_GET,
    MS_EXPR_GROUPING, MS_EXPR_LITERAL, MS_EXPR_LOGICAL, MS_EXPR_SET,
    MS_EXPR_SUPER, MS_EXPR_THIS, MS_EXPR_UNARY, MS_EXPR_VARIABLE,
    MS_EXPR_LIST, MS_EXPR_SUBSCRIPT
} MsExprType;

typedef enum { MS_LITERAL_NIL, MS_LITERAL_BOOL, MS_LITERAL_NUMBER, MS_LITERAL_STRING } MsLiteralType;

typedef struct {
    MsLiteralType type;
    union {
        bool boolean;
        double number;
        struct { const char* start; int length; };
    };
} MsLiteralValue;

struct MsExpr {
    MsExprType type;
    union {
        struct { MsToken name; MsExpr* value; } assign;
        struct { MsExpr* left; MsToken op; MsExpr* right; } binary;
        struct { MsExpr* callee; MsToken paren; MsExpr** args; int argCount; } call;
        struct { MsExpr* object; MsToken name; } get;
        struct { MsExpr* expression; } grouping;
        struct { MsLiteralValue value; MsToken token; } literal;
        struct { MsExpr* left; MsToken op; MsExpr* right; } logical;
        struct { MsExpr* object; MsToken name; MsExpr* value; } set;
        struct { MsToken keyword; MsToken method; } super;
        struct { MsToken keyword; } this_expr;
        struct { MsToken op; MsExpr* right; } unary;
        struct { MsToken name; } variable;
        struct { MsToken bracket; MsExpr** elements; int elementCount; } list;
        struct { MsExpr* object; MsToken bracket; MsExpr* index; } subscript;
    };
};

typedef enum {
    MS_STMT_BLOCK, MS_STMT_CLASS, MS_STMT_EXPRESSION, MS_STMT_FUNCTION,
    MS_STMT_IF, MS_STMT_IMPORT, MS_STMT_RETURN, MS_STMT_VAR,
    MS_STMT_WHILE, MS_STMT_FOR, MS_STMT_BREAK, MS_STMT_CONTINUE
} MsStmtType;

typedef struct {
    MsToken name;
    MsToken alias;  // alias.length == 0 if no alias
} MsImportItem;

struct MsStmt {
    MsStmtType type;
    union {
        struct { MsStmt** statements; int count; } block;
        struct { MsToken name; MsToken superclass; MsStmt** methods; int methodCount; } class_stmt;
        struct { MsExpr* expression; } expression;
        struct { MsToken name; MsToken* params; int paramCount; MsStmt** body; int bodyCount; } function;
        struct { MsToken keyword; MsExpr* condition; MsStmt* thenBranch; MsStmt* elseBranch; } if_stmt;
        struct { MsToken importToken; MsToken moduleName; MsImportItem* items; int itemCount; bool isFromImport; } import;
        struct { MsToken keyword; MsExpr* value; } return_stmt;
        struct { MsToken name; MsExpr* initializer; } var;
        struct { MsToken keyword; MsExpr* condition; MsStmt* body; } while_stmt;
        struct { MsToken keyword; MsStmt* initializer; MsExpr* condition; MsExpr* increment; MsStmt* body; } for_stmt;
        struct { MsToken keyword; } break_stmt;
        struct { MsToken keyword; } continue_stmt;
    };
};

MsExpr* ms_expr_create(MsExprType type);
MsStmt* ms_stmt_create(MsStmtType type);
void ms_expr_free(MsExpr* expr);
void ms_stmt_free(MsStmt* stmt);
void ms_stmt_list_free(MsStmt** stmts, int count);
```

### 2.5 Parser (`parser.h / .c`)

Pratt parser for expressions, recursive descent for statements.

```c
typedef struct {
    MsToken token;
    char message[256];
} MsParseError;

typedef struct {
    MsScanner scanner;
    MsToken current;
    MsToken previous;
    bool hadError;
    bool panicMode;
    MsParseError lastError;
} MsParser;

void ms_parser_init(MsParser* parser, const char* source);
int ms_parser_parse(MsParser* parser, MsStmt*** outStatements);  // returns count, -1 on error
bool ms_parser_had_error(const MsParser* parser);
```

Internal (static in `parser.c`): `advance()`, `consume()`, `check()`, `match()`, `synchronize()`. Expression parsing by precedence: `parseAssignment→parseOr→parseAnd→parseEquality→parseComparison→parseTerm→parseFactor→parseUnary→parseCall→parsePrimary`. Statement parsing: `parseStatement→parseDeclaration`, `parseVarDeclaration`, `parseFunctionDeclaration`, `parseClassDeclaration`, `parseImportStatement`, `parseBlock`, `parseIfStatement`, `parseWhileStatement`, `parseForStatement`, `parseReturnStatement`.

### 2.6 Bytecode (`chunk.h / .c`)

```c
typedef enum {
    MS_OP_CONSTANT, MS_OP_NIL, MS_OP_TRUE, MS_OP_FALSE, MS_OP_POP,
    MS_OP_DEFINE_GLOBAL, MS_OP_GET_GLOBAL, MS_OP_SET_GLOBAL,
    MS_OP_GET_LOCAL, MS_OP_SET_LOCAL, MS_OP_GET_UPVALUE, MS_OP_SET_UPVALUE,
    MS_OP_GET_PROPERTY, MS_OP_SET_PROPERTY, MS_OP_GET_SUPER,
    MS_OP_BUILD_LIST, MS_OP_GET_SUBSCRIPT, MS_OP_SET_SUBSCRIPT,
    MS_OP_EQUAL, MS_OP_NOT_EQUAL, MS_OP_GREATER, MS_OP_GREATER_EQUAL, MS_OP_LESS, MS_OP_LESS_EQUAL,
    MS_OP_ADD, MS_OP_SUBTRACT, MS_OP_MULTIPLY, MS_OP_DIVIDE, MS_OP_MODULO, MS_OP_NEGATE,
    MS_OP_NOT, MS_OP_AND, MS_OP_OR,
    MS_OP_JUMP, MS_OP_JUMP_IF_FALSE, MS_OP_LOOP, MS_OP_BREAK, MS_OP_CONTINUE,
    MS_OP_CALL, MS_OP_INVOKE, MS_OP_SUPER_INVOKE, MS_OP_CLOSURE, MS_OP_CLOSE_UPVALUE,
    MS_OP_CLASS, MS_OP_INHERIT, MS_OP_METHOD,
    MS_OP_IMPORT, MS_OP_IMPORT_FROM,
    MS_OP_RETURN,
    MS_OP_DEBUG_BREAK
} MsOpCode;

typedef struct {
    uint8_t* code;
    int* lines;
    int count;
    int capacity;
    MsValueArray constants;
} MsChunk;

void ms_chunk_init(MsChunk* chunk);
void ms_chunk_free(MsChunk* chunk);
void ms_chunk_write(MsChunk* chunk, uint8_t byte, int line);
int ms_chunk_add_constant(MsChunk* chunk, MsValue value);
void ms_chunk_disassemble(const MsChunk* chunk, const char* name);
int ms_chunk_disassemble_instruction(const MsChunk* chunk, int offset);
```

### 2.7 Values (`value.h / .c`)

```c
typedef struct MsObject MsObject;

typedef enum { MS_VAL_NIL, MS_VAL_BOOL, MS_VAL_NUMBER, MS_VAL_OBJ } MsValueType;

typedef struct {
    MsValueType type;
    union { bool boolean; double number; MsObject* obj; };
} MsValue;

static inline MsValue ms_nil_val(void)    { return (MsValue){ .type = MS_VAL_NIL, .number = 0 }; }
static inline MsValue ms_bool_val(bool v) { return (MsValue){ .type = MS_VAL_BOOL, .boolean = v }; }
static inline MsValue ms_number_val(double v) { return (MsValue){ .type = MS_VAL_NUMBER, .number = v }; }
static inline MsValue ms_obj_val(MsObject* o) { return (MsValue){ .type = MS_VAL_OBJ, .obj = o }; }

static inline bool ms_is_nil(MsValue v)    { return v.type == MS_VAL_NIL; }
static inline bool ms_is_bool(MsValue v)   { return v.type == MS_VAL_BOOL; }
static inline bool ms_is_number(MsValue v) { return v.type == MS_VAL_NUMBER; }
static inline bool ms_is_obj(MsValue v)    { return v.type == MS_VAL_OBJ; }

static inline bool ms_as_bool(MsValue v)     { return v.boolean; }
static inline double ms_as_number(MsValue v) { return v.number; }
static inline MsObject* ms_as_obj(MsValue v) { return v.obj; }

bool ms_values_equal(MsValue a, MsValue b);
bool ms_is_falsey(MsValue value);
void ms_print_value(MsValue value);

typedef struct { MsValue* values; int count; int capacity; } MsValueArray;
void ms_value_array_init(MsValueArray* array);
void ms_value_array_free(MsValueArray* array);
void ms_value_array_write(MsValueArray* array, MsValue value);
```

### 2.8 Objects (`object.h / .c`)

```c
typedef enum {
    MS_OBJ_STRING, MS_OBJ_FUNCTION, MS_OBJ_CLOSURE, MS_OBJ_UPVALUE,
    MS_OBJ_CLASS, MS_OBJ_INSTANCE, MS_OBJ_BOUND_METHOD, MS_OBJ_MODULE,
    MS_OBJ_LIST, MS_OBJ_NATIVE
} MsObjectType;

struct MsObject {
    MsObjectType type;
    MsObject* next;
    bool isMarked;
};

typedef struct { MsObject base; char* chars; int length; uint32_t hash; } MsString;
typedef MsValue (*MsNativeFn)(MsVM* vm, int argCount, MsValue* args);
typedef struct { MsObject base; MsNativeFn function; MsString* name; int arity; } MsNative;
typedef struct { MsObject base; int arity; int upvalueCount; MsChunk chunk; MsString* name; } MsFunction;
typedef struct MsUpvalue { MsObject base; MsValue* location; MsValue closed; struct MsUpvalue* next; } MsUpvalue;
typedef struct { MsObject base; MsFunction* function; MsUpvalue** upvalues; int upvalueCount; } MsClosure;
typedef struct { MsObject base; MsString* name; struct MsClass* superclass; MsTable methods; } MsClass;
typedef struct { MsObject base; MsClass* klass; MsTable fields; } MsInstance;
typedef struct { MsObject base; MsValue receiver; MsClosure* method; } MsBoundMethod;
typedef struct { MsObject base; MsString* name; char* path; MsTable exports; bool isLoaded; } MsModule;
typedef struct { MsObject base; MsValue* elements; int count; int capacity; } MsList;

#define MS_IS_STRING(v)  (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_STRING)
#define MS_IS_FUNCTION(v)(ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_FUNCTION)
#define MS_IS_CLOSURE(v) (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_CLOSURE)
#define MS_IS_CLASS(v)   (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_CLASS)
#define MS_IS_INSTANCE(v)(ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_INSTANCE)
#define MS_IS_MODULE(v)  (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_MODULE)
#define MS_IS_LIST(v)    (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_LIST)
#define MS_IS_NATIVE(v)  (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_NATIVE)

#define MS_AS_STRING(v)  ((MsString*)ms_as_obj(v))
#define MS_AS_FUNCTION(v)((MsFunction*)ms_as_obj(v))
#define MS_AS_CLOSURE(v) ((MsClosure*)ms_as_obj(v))
#define MS_AS_CLASS(v)   ((MsClass*)ms_as_obj(v))
#define MS_AS_INSTANCE(v)((MsInstance*)ms_as_obj(v))
#define MS_AS_MODULE(v)  ((MsModule*)ms_as_obj(v))
#define MS_AS_LIST(v)    ((MsList*)ms_as_obj(v))
#define MS_AS_NATIVE(v)  ((MsNative*)ms_as_obj(v))

MsString* ms_string_copy(const char* chars, int length);
MsString* ms_string_take(char* chars, int length);
uint32_t ms_string_hash(const char* key, int length);
MsString* ms_string_concat(MsString* a, MsString* b);
MsFunction* ms_function_new(MsString* name);
MsNative* ms_native_new(MsNativeFn fn, MsString* name, int arity);
MsClosure* ms_closure_new(MsFunction* function);
MsUpvalue* ms_upvalue_new(MsValue* slot);
MsClass* ms_class_new(MsString* name);
MsInstance* ms_instance_new(MsClass* klass);
MsBoundMethod* ms_bound_method_new(MsValue receiver, MsClosure* method);
MsModule* ms_module_new(MsString* name);
MsList* ms_list_new(void);
void ms_list_append(MsList* list, MsValue value);
MsValue ms_list_get(MsList* list, int index);
void ms_list_set(MsList* list, int index, MsValue value);
int ms_list_length(MsList* list);
void ms_object_free(MsObject* obj);
void ms_object_print(MsValue value);
```

### 2.9 Hash Table (`table.h / .c`)

```c
typedef struct { MsString* key; MsValue value; } MsTableEntry;
typedef struct { MsTableEntry* entries; int count; int capacity; } MsTable;

void ms_table_init(MsTable* table);
void ms_table_free(MsTable* table);
bool ms_table_set(MsTable* table, MsString* key, MsValue value);
bool ms_table_get(MsTable* table, MsString* key, MsValue* outValue);
bool ms_table_remove(MsTable* table, MsString* key);
void ms_table_add_all(MsTable* from, MsTable* to);
MsString* ms_table_find_string(MsTable* table, const char* chars, int length, uint32_t hash);
void ms_table_remove_white(MsTable* table);
void ms_table_mark(MsTable* table);
```

### 2.10 Compiler (`compiler.h / .c`)

Single-pass compiler: walks AST and emits bytecode.

```c
typedef enum { MS_FUNC_SCRIPT, MS_FUNC_FUNCTION, MS_FUNC_METHOD, MS_FUNC_INITIALIZER } MsFunctionType;

typedef struct MsLocal { MsToken name; int depth; bool isCaptured; } MsLocal;
typedef struct MsCompilerUpvalue { int index; bool isLocal; } MsCompilerUpvalue;

typedef struct MsCompilerState {
    struct MsCompilerState* enclosing;
    MsFunction* function;
    MsFunctionType type;
    MsLocal locals[MS_MAX_LOCALS];
    int localCount;
    int scopeDepth;
    MsCompilerUpvalue upvalues[MS_MAX_UPVALUES];
    int upvalueCount;
    int* breakJumps; int breakJumpCount; int breakJumpCapacity;
    int* continueJumps; int continueJumpCount; int continueJumpCapacity;
    int loopStart;
} MsCompilerState;

typedef struct { MsParser parser; MsCompilerState* current; } MsCompiler;

void ms_compiler_init(MsCompiler* compiler);
void ms_compiler_free(MsCompiler* compiler);
MsFunction* ms_compiler_compile(MsCompiler* compiler, const char* source);
void ms_compiler_mark_roots(MsCompiler* compiler);
```

Internal (static): emit: `emitByte`, `emitBytes`, `emitLoop`, `emitJump`, `emitReturn`, `emitConstant`, `patchJump`. Constants: `makeConstant`, `identifierConstant`. Variables: `declareVariable`, `defineVariable`, `markInitialized`, `resolveLocal`, `addUpvalue`, `resolveUpvalue`. Scope: `beginScope`, `endScope`. Compile expressions: `compileExpr`, `compileLiteral`, `compileVariable`, `compileAssign`, `compileBinary`, `compileUnary`, `compileLogical`, `compileCall`, `compileGet`, `compileSet`, `compileThis`, `compileSuper`, `compileList`, `compileSubscript`. Compile statements: `compileStmt`, `compileBlock`, `compileVarDecl`, `compileFuncDecl`, `compileClassDecl`, `compileIfStmt`, `compileWhileStmt`, `compileForStmt`, `compileReturnStmt`, `compileImportStmt`, `compileBreakStmt`, `compileContinueStmt`, `compileExprStmt`.

### 2.11 Virtual Machine (`vm.h / .c`)

```c
typedef struct { MsClosure* closure; uint8_t* ip; MsValue* slots; } MsCallFrame;

typedef struct {
    MsValue stack[MS_STACK_MAX];
    MsValue* stackTop;
    MsCallFrame frames[MS_FRAMES_MAX];
    int frameCount;
    MsTable globals;
    MsTable strings;
    MsTable modules;
    char** modulePaths; int modulePathCount; int modulePathCapacity;
    MsObject* objects;
    size_t bytesAllocated;
    size_t nextGC;
    MsObject** grayStack; int grayCount; int grayCapacity;
    MsCompiler* compiler;
    bool initialized;
} MsVM;

typedef enum { MS_INTERPRET_OK, MS_INTERPRET_COMPILE_ERROR, MS_INTERPRET_RUNTIME_ERROR } MsInterpretResult;

void ms_vm_init(MsVM* vm);
void ms_vm_free(MsVM* vm);
MsInterpretResult ms_vm_interpret(MsVM* vm, const char* source);
MsInterpretResult ms_vm_import_module(MsVM* vm, const char* moduleName);
MsInterpretResult ms_vm_import_from(MsVM* vm, const char* moduleName, const char* itemName, const char* alias);
void ms_vm_add_module_path(MsVM* vm, const char* path);
```

Internal (static): Stack: `push`, `pop`, `peek`, `resetStack`. Execution: `run` (switch-based dispatch). Call: `callValue`, `call`, `invokeFromClass`, `invoke`, `bindMethod`. Upvalue: `captureUpvalue`, `closeUpvalues`. Error: `runtimeError`. GC: `collectGarbage`, `markObject`, `markValue`, `markRoots`, `traceReferences`, `sweep`, `blackenObject`. Natives: `defineNative`, `defineNatives`. Module: `resolveModulePath`, `loadModule`. Debug: `printStack`, `disassembleInstruction`.

### 2.12 Module System (`module.h / .c`)

```c
typedef struct { char* moduleName; char* message; int line; } MsModuleError;

MsResult ms_module_load(MsVM* vm, const char* moduleName, MsModule** outModule);
MsModule* ms_module_get_loaded(MsVM* vm, const char* moduleName);
void ms_module_add_search_path(MsVM* vm, const char* path);
char* ms_module_resolve_path(MsVM* vm, const char* moduleName);
```

Internal (static): `readFile`, `fileExists`, `detectCircularDependency`.

### 2.13 Logger (`logger.h / .c`)

```c
typedef enum { MS_LOG_TRACE, MS_LOG_DEBUG, MS_LOG_INFO, MS_LOG_WARN, MS_LOG_ERROR, MS_LOG_FATAL, MS_LOG_OFF } MsLogLevel;

void ms_logger_set_level(MsLogLevel level);
void ms_logger_set_output(FILE* stream);
void ms_logger_enable_colors(bool enable);
void ms_logger_enable_timestamp(bool enable);
void ms_logger_log(MsLogLevel level, const char* file, int line, const char* func, const char* fmt, ...);

#define ms_logger_trace(...) ms_logger_log(MS_LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_debug(...) ms_logger_log(MS_LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_info(...)  ms_logger_log(MS_LOG_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_warn(...)  ms_logger_log(MS_LOG_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_error(...) ms_logger_log(MS_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ms_logger_fatal(...) ms_logger_log(MS_LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

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

### 2.14 Memory (`memory.h / .c`)

```c
void* ms_reallocate(void* pointer, size_t oldSize, size_t newSize);

#define MS_ALLOCATE(type, count) \
    (type*)ms_reallocate(NULL, 0, sizeof(type) * (count))
#define MS_FREE(type, pointer, count) \
    ms_reallocate(pointer, sizeof(type) * (count), 0)
#define MS_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)
#define MS_GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)ms_reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))
#define MS_FREE_ARRAY(type, pointer, oldCount) \
    ms_reallocate(pointer, sizeof(type) * (oldCount), 0)

void ms_gc_collect(MsVM* vm);
void ms_gc_mark_object(MsVM* vm, MsObject* obj);
void ms_gc_mark_value(MsVM* vm, MsValue value);
void ms_gc_mark_roots(MsVM* vm);
void ms_gc_trace_references(MsVM* vm);
void ms_gc_sweep(MsVM* vm);
void ms_gc_blacken_object(MsVM* vm, MsObject* obj);

#define MS_NEW_OBJ(vm, type, objType) \
    (type*)ms_alloc_object(vm, sizeof(type), objType)
MsObject* ms_alloc_object(MsVM* vm, size_t size, MsObjectType type);
void ms_free_objects(MsVM* vm);
```

### 2.15 Platform (`platform.h / .c`)

```c
bool ms_platform_file_exists(const char* path);
char* ms_platform_read_file(const char* path);
bool ms_platform_write_file(const char* path, const char* content);
char* ms_platform_get_cwd(void);
char* ms_platform_join_path(const char* a, const char* b);
char* ms_platform_get_executable_path(void);
void ms_platform_enable_console_colors(void);
bool ms_platform_supports_colors(void);
void ms_platform_set_console_color(const char* ansi_code);
void ms_platform_reset_console_color(void);
double ms_platform_get_time_seconds(void);
char* ms_platform_get_env(const char* name);

#ifdef _WIN32
  #define MS_PLATFORM_WINDOWS 1
#else
  #define MS_PLATFORM_WINDOWS 0
#endif
#ifdef __linux__
  #define MS_PLATFORM_LINUX 1
#else
  #define MS_PLATFORM_LINUX 0
#endif
#ifdef __APPLE__
  #define MS_PLATFORM_MACOS 1
#else
  #define MS_PLATFORM_MACOS 0
#endif

#if MS_PLATFORM_WINDOWS
  #define MS_PATH_SEPARATOR '\\'
  #define MS_PATH_SEPARATOR_STR "\\"
#else
  #define MS_PATH_SEPARATOR '/'
  #define MS_PATH_SEPARATOR_STR "/"
#endif
```

### 2.16 Builtins (`builtins.h / .c`)

```c
void ms_builtins_define_all(MsVM* vm);
MsValue ms_builtin_print(MsVM* vm, int argCount, MsValue* args);
MsValue ms_builtin_clock(MsVM* vm, int argCount, MsValue* args);
MsValue ms_builtin_type(MsVM* vm, int argCount, MsValue* args);
MsValue ms_builtin_len(MsVM* vm, int argCount, MsValue* args);
MsValue ms_builtin_input(MsVM* vm, int argCount, MsValue* args);
MsValue ms_builtin_str(MsVM* vm, int argCount, MsValue* args);
MsValue ms_builtin_num(MsVM* vm, int argCount, MsValue* args);
```

## 3. Import System

**Syntax**: `import <module>` | `from <module> import <item> (as <alias>)? (, <item> (as <alias>)?)*`

**Resolution**:
1. Cache lookup in `vm->modules` → return if found
2. Resolve path: try `"./<module>.ms"`, then each `vm->modulePaths` entry
3. Circular dependency check via per-VM loading stack
4. Load file → compile via `ms_compiler_compile()` → execute module body → capture top-level declarations as exports
5. Cache module, bind imported symbols to current scope

**Bytecodes**:
- `MS_OP_IMPORT <constant>` — push module object (contains exports table)
- `MS_OP_IMPORT_FROM <module_const> <item_const> <alias_const?>` — import specific item, optionally aliased

## 4. Error Handling

```c
typedef struct {
    char message[256];
    MsToken token;
    char sourceLine[512];
} MsCompileError;

typedef struct {
    char message[256];
    MsCallFrame* frames[MS_FRAMES_MAX];
    int frameCount;
} MsRuntimeError;
```

**Recovery**: Lexer → skip to next token, return `MS_TOKEN_ERROR`. Parser → synchronize to next boundary (newline, `}`, class, fn, var, for, if, while, print, return). Compiler → stop after first error. VM → unwind stack, print stack trace.

## 5. Testing

### 5.1 Test Files

```
tests/unit/:    test_scanner.c, test_parser.c, test_compiler.c, test_vm.c, test_gc.c, test_table.c, test_logger.c
tests/basic/:   arithmetic.ms, strings.ms, variables.ms, control_flow.ms
tests/functions/: basic.ms, closures.ms, recursion.ms
tests/classes/:  basic.ms, inheritance.ms, methods.ms
tests/modules/:  math.ms, utils.ms, test_import.ms, test_from_import.ms
tests/errors/:   syntax_errors.ms, runtime_errors.ms, import_errors.ms
```

### 5.2 Test Framework

```c
typedef struct { const char* name; void (*func)(void); bool passed; char error[256]; } MsTest;
typedef struct { MsTest* tests; int count; int capacity; int passed; int failed; } MsTestSuite;

void ms_test_suite_init(MsTestSuite* suite);
void ms_test_suite_free(MsTestSuite* suite);
void ms_test_suite_add(MsTestSuite* suite, const char* name, void (*func)(void));
int ms_test_suite_run(MsTestSuite* suite);
void ms_test_suite_report(const MsTestSuite* suite);

extern MsTestSuite* ms_current_suite;
#define TEST(name) static void test_##name(void)
#define RUN_TEST(suite, name) ms_test_suite_add(suite, #name, test_##name)
#define ASSERT_TRUE(cond) do { if (!(cond)) { /* snprintf error + return */ } } while(0)
#define ASSERT_EQ(expected, actual) do { if ((expected) != (actual)) { /* snprintf error + return */ } } while(0)
#define ASSERT_STR_EQ(expected, actual) do { if (strcmp((expected),(actual)) != 0) { /* snprintf error + return */ } } while(0)
```

## 6. Build Configuration

```cmake
cmake_minimum_required(VERSION 3.10)
project(Maple LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(maple ${SOURCES})
target_include_directories(maple PRIVATE src)
add_library(maple_lib STATIC ${SOURCES})
target_include_directories(maple_lib PUBLIC src)

target_compile_definitions(maple PRIVATE
    $<$<CONFIG:Debug>:MS_DEBUG_TRACE_EXECUTION>
    $<$<CONFIG:Debug>:MS_DEBUG_LOG_GC>)

option(BUILD_TESTS "Build test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
install(TARGETS maple RUNTIME DESTINATION bin)
```

## 7. Memory Patterns

### 7.1 Dynamic Arrays

All dynamic arrays follow: `init(data=NULL, count=0, capacity=0)` → `write` grows with `MS_GROW_CAPACITY` + `MS_GROW_ARRAY` → `free` uses `MS_FREE_ARRAY` then re-inits.

### 7.2 Object Allocation

All VM-managed objects allocated via `ms_alloc_object(vm, size, type)` which calls `ms_reallocate`, links into `vm->objects` list, tracks `vm->bytesAllocated`.

### 7.3 GC Lifecycle

1. Threshold: `if (bytesAllocated > nextGC) ms_gc_collect()`
2. Mark: stack → call frames → globals → open upvalues → compiler roots → trace gray stack
3. Sweep: walk object list, free unmarked, reset marks on survivors, update nextGC

## 8. Performance

- **String interning** in `vm->strings` table
- **Switch dispatch** (computed goto optional for GCC/Clang via labels-as-values)
- **Fixed stack**: pre-allocated `MsValue[MS_STACK_MAX]`
- **Values**: 16-byte tagged union inline (type + union)
- **Objects**: heap via `ms_reallocate`, intrusive linked list

Computed goto (optional):
```c
#ifdef __GNUC__
  #define MS_DISPATCH() goto *dispatch_table[*ip++]
  #define MS_OPCODE(op) &&label_##op
  static void* dispatch_table[] = { &&label_MS_OP_CONSTANT, &&label_MS_OP_NIL, ... };
#else
  #define MS_DISPATCH() switch (*ip++)
#endif
```

## 9. C vs C++ Pattern Mapping

| C++ | C |
|---|---|
| `namespace ms {}` | `ms_` function prefix |
| class/struct with methods | `struct` + separate functions |
| `std::vector<T>` | Dynamic array (`T*` + count + capacity) |
| `std::string` | `char*` + `int length` |
| `std::string_view` | `const char*` + `int length` |
| `std::variant<A,B,C>` | Tagged union (enum type + union) |
| `std::expected<T,E>` | Return `MsResult` + output param |
| `std::optional<T>` | Return bool + output param |
| virtual/polymorphism | Function pointers / tag dispatch |
| `std::unique_ptr` | Manual init/free pairs |
| `std::unordered_map` | Custom `MsTable` |
| `std::format()` | `snprintf()` |
| `std::source_location` | `__FILE__`, `__LINE__`, `__func__` |
| `#pragma once` | `#ifndef`/`#define`/`#endif` guards |
| RAII destructors | Explicit `ms_xxx_free()` |
| `template<typename T>` | Macros (`MS_GROW_ARRAY`, etc.) |
| `static_assert(cond)` | `_Static_assert(cond, msg)` |
| Exceptions | Error codes + `setjmp`/`longjmp` (optional) |

## 10. Future Extensions

JIT compilation, generational GC, coroutines (setjmp/longjmp or ucontext), pattern matching, optional static typing, standard library, debugger, profiler, LSP, package manager.
