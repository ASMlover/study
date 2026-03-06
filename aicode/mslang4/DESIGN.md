# Maple Scripting Language - Design Specification

## 1. Architecture Overview

### 1.1 High-Level Architecture

```
Source Code (.maple)
       │
       ▼
   ┌─────────┐
   │ Scanner │  → Token Stream
   └─────────┘
       │
       ▼
   ┌─────────┐
   │ Parser  │  → Abstract Syntax Tree (AST)
   └─────────┘
       │
       ▼
   ┌───────────┐
   │ Compiler  │  → Bytecode Chunks
   └───────────┘
       │
       ▼
   ┌──────────────┐
   │ Virtual      │  ←→ Garbage Collector
   │ Machine (VM) │
   └──────────────┘
       │
       ▼
   Execution Result
```

### 1.2 Component Diagram

```
┌────────────────────────────────────────────────────────────┐
│                         Maple Runtime                       │
├────────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │  Logger  │  │ Platform │  │  Memory  │  │  Module  │  │
│  │  System  │  │  Layer   │  │ Manager  │  │  Loader  │  │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │
├────────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │ Scanner  │  │  Parser  │  │Compiler  │  │   VM     │  │
│  │          │→ │          │→ │          │→ │          │  │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │
├────────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │  Value   │  │  Object  │  │  Table   │  │  Chunk   │  │
│  │  System  │  │  System  │  │(HashMap) │  │(Bytecode)│  │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │
└────────────────────────────────────────────────────────────┘
```

### 1.3 Data Flow

```
1. Lexical Analysis:
   "var x = 42;" → [VAR, IDENTIFIER("x"), EQUAL, NUMBER(42), SEMICOLON]

2. Parsing:
   Token Stream → VarDecl { name: "x", initializer: Literal(42) }

3. Compilation:
   AST → Bytecode: [OP_CONSTANT, 0, OP_DEFINE_GLOBAL, 1]

4. Execution:
   VM interprets bytecode → Memory state updated
```

## 2. Detailed Component Design

### 2.1 Token and Lexer System

#### 2.1.1 Token Types (`token.hh`)

```cpp
namespace ms {

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,  // For arrays
    COMMA, DOT, SEMICOLON, COLON,
    
    // Operators
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    
    // Literals
    IDENTIFIER, STRING, NUMBER,
    
    // Keywords
    AND, CLASS, ELSE, FALSE, FUN, FOR,
    IF, NIL, OR, PRINT, RETURN, SUPER,
    THIS, TRUE, VAR, WHILE,
    BREAK, CONTINUE,  // Loop control
    
    // Module system
    IMPORT, FROM, AS,
    
    // Special
    ERROR, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    std::variant<std::monostate, double, std::string_view> literal;
    size_t line;
    size_t column;
    std::source_location location;  // C++23 feature
};

} // namespace ms
```

#### 2.1.2 Scanner Design (`scanner.hh/cc`)

**Responsibilities**:
- Convert source code to token stream
- Track line and column numbers
- Handle string interpolation (optional)
- Report lexical errors

**Key Methods**:
```cpp
class Scanner {
public:
    explicit Scanner(std::string_view source);
    
    Token scanToken();
    bool isAtEnd() const;
    size_t getCurrentLine() const;
    
private:
    std::string_view source_;
    size_t start_;
    size_t current_;
    size_t line_;
    size_t column_;
    
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    
    Token makeToken(TokenType type);
    Token errorToken(std::string_view message);
    
    void skipWhitespace();
    Token scanString();
    Token scanNumber();
    Token scanIdentifier();
    TokenType identifierType();
    TokenType checkKeyword(size_t start, size_t length, 
                          std::string_view rest, TokenType type);
};
```

### 2.2 Abstract Syntax Tree (`ast.hh`)

#### 2.2.1 AST Node Design

Using modern C++23 with `std::variant` for type-safe AST nodes:

```cpp
namespace ms {

// Forward declarations
struct Expr;
struct Stmt;

// Expressions
struct AssignExpr {
    Token name;
    std::unique_ptr<Expr> value;
};

struct BinaryExpr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
};

struct CallExpr {
    std::unique_ptr<Expr> callee;
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;
};

struct GetExpr {
    std::unique_ptr<Expr> object;
    Token name;
};

struct GroupingExpr {
    std::unique_ptr<Expr> expression;
};

struct LiteralExpr {
    Token literal;
    std::variant<std::monostate, double, std::string> value;
};

struct LogicalExpr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
};

struct SetExpr {
    std::unique_ptr<Expr> object;
    Token name;
    std::unique_ptr<Expr> value;
};

struct SuperExpr {
    Token keyword;
    Token method;
};

struct ThisExpr {
    Token keyword;
};

struct UnaryExpr {
    Token op;
    std::unique_ptr<Expr> right;
};

struct VariableExpr {
    Token name;
};

struct ListExpr {  // For array literals
    Token bracket;
    std::vector<std::unique_ptr<Expr>> elements;
};

struct SubscriptExpr {  // For array indexing
    std::unique_ptr<Expr> object;
    Token bracket;
    std::unique_ptr<Expr> index;
};

using Expr = std::variant<
    AssignExpr,
    BinaryExpr,
    CallExpr,
    GetExpr,
    GroupingExpr,
    LiteralExpr,
    LogicalExpr,
    SetExpr,
    SuperExpr,
    ThisExpr,
    UnaryExpr,
    VariableExpr,
    ListExpr,
    SubscriptExpr
>;

// Statements
struct BlockStmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct ClassStmt {
    Token name;
    std::optional<Token> superclass;
    std::vector<std::unique_ptr<Stmt>> methods;
};

struct ExpressionStmt {
    std::unique_ptr<Expr> expression;
};

struct FunctionStmt {
    Token name;
    std::vector<Token> params;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct IfStmt {
    Token ifToken;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
};

struct ImportStmt {
    Token importToken;
    Token moduleName;
    std::vector<std::pair<Token, std::optional<Token>>> items;  // (name, alias)
    bool isFromImport;  // true for "from X import Y"
};

struct ReturnStmt {
    Token keyword;
    std::unique_ptr<Expr> value;
};

struct VarStmt {
    Token name;
    std::unique_ptr<Expr> initializer;
};

struct WhileStmt {
    Token whileToken;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
};

struct ForStmt {
    Token forToken;
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;
};

struct BreakStmt {
    Token keyword;
};

struct ContinueStmt {
    Token keyword;
};

using Stmt = std::variant<
    BlockStmt,
    ClassStmt,
    ExpressionStmt,
    FunctionStmt,
    IfStmt,
    ImportStmt,
    ReturnStmt,
    VarStmt,
    WhileStmt,
    ForStmt,
    BreakStmt,
    ContinueStmt
>;

} // namespace ms
```

### 2.3 Parser Design (`parser.hh/cc`)

#### 2.3.1 Recursive Descent Parser

**Design Pattern**: Pratt parser for expressions, recursive descent for statements

```cpp
namespace ms {

class Parser {
public:
    explicit Parser(std::string_view source);
    
    std::expected<std::vector<std::unique_ptr<Stmt>>, 
                  ParseError> parse();
    
private:
    Scanner scanner_;
    Token current_;
    Token previous_;
    bool hadError_;
    bool panicMode_;
    
    // Error handling
    struct ParseError {
        Token token;
        std::string message;
    };
    
    ParseError error(Token token, std::string_view message);
    void synchronize();
    
    // Token consumption
    Token advance();
    Token consume(TokenType type, std::string_view message);
    bool check(TokenType type) const;
    bool match(TokenType type);
    
    // Expression parsing (Pratt parser)
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parseCall();
    std::unique_ptr<Expr> parsePrimary();
    
    // Statement parsing
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseDeclaration();
    std::unique_ptr<Stmt> parseVarDeclaration();
    std::unique_ptr<Stmt> parseFunctionDeclaration();
    std::unique_ptr<Stmt> parseClassDeclaration();
    std::unique_ptr<Stmt> parseImportStatement();
    std::unique_ptr<Stmt> parseBlock();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseForStatement();
    std::unique_ptr<Stmt> parseReturnStatement();
    
    // Helper methods
    std::vector<std::unique_ptr<Stmt>> parseBlockStatements();
    std::vector<Token> parseFunctionParameters();
    std::vector<std::unique_ptr<Expr>> parseCallArguments();
};

} // namespace ms
```

### 2.4 Bytecode Design (`chunk.hh/cc`)

#### 2.4.1 Instruction Set

```cpp
namespace ms {

enum class OpCode : uint8_t {
    // Stack operations
    OP_CONSTANT,        // Load constant
    OP_NIL,            // Push nil
    OP_TRUE,           // Push true
    OP_FALSE,          // Push false
    OP_POP,            // Pop top of stack
    
    // Variable operations
    OP_DEFINE_GLOBAL,  // Define global variable
    OP_GET_GLOBAL,     // Get global variable
    OP_SET_GLOBAL,     // Set global variable
    OP_GET_LOCAL,      // Get local variable
    OP_SET_LOCAL,      // Set local variable
    OP_GET_UPVALUE,    // Get upvalue
    OP_SET_UPVALUE,    // Set upvalue
    
    // Property access
    OP_GET_PROPERTY,   // Get object property
    OP_SET_PROPERTY,   // Set object property
    OP_GET_SUPER,      // Get superclass method
    
    // Array operations
    OP_BUILD_LIST,     // Build list from stack elements
    OP_GET_SUBSCRIPT,  // Get list element by index
    OP_SET_SUBSCRIPT,  // Set list element by index
    
    // Comparison
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    
    // Arithmetic
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NEGATE,
    
    // Logic
    OP_NOT,
    OP_AND,            // Short-circuit and
    OP_OR,             // Short-circuit or
    
    // Control flow
    OP_JUMP,           // Unconditional jump
    OP_JUMP_IF_FALSE,  // Conditional jump
    OP_LOOP,           // Jump backward for loops
    OP_BREAK,          // Break from loop (internal)
    OP_CONTINUE,       // Continue loop (internal)
    
    // Functions
    OP_CALL,           // Function call
    OP_INVOKE,         // Method call (optimized)
    OP_SUPER_INVOKE,   // Super method call
    OP_CLOSURE,        // Create closure
    OP_CLOSE_UPVALUE,  // Close upvalue
    
    // Classes
    OP_CLASS,          // Create class
    OP_INHERIT,        // Inherit from superclass
    OP_METHOD,         // Define method
    
    // Modules
    OP_IMPORT,         // Import module
    OP_IMPORT_FROM,    // Import specific items from module
    
    // Return
    OP_RETURN,
    
    // Debug
    OP_DEBUG_BREAK,    // Debugger breakpoint (optional)
};

struct BytecodeChunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<size_t> lines;  // Line numbers for debugging
    
    void write(uint8_t byte, size_t line);
    void writeOp(OpCode op, size_t line);
    size_t addConstant(Value value);
    size_t getCount() const;
    OpCode getOp(size_t offset) const;
    uint8_t getByte(size_t offset) const;
    size_t getLine(size_t offset) const;
};

} // namespace ms
```

### 2.5 Value Representation (`value.hh/cc`)

#### 2.5.1 Tagged Union Design

Using NaN boxing for efficient value representation (or simple tagged union for clarity):

```cpp
namespace ms {

// Forward declarations
class Object;

// Value type enumeration
enum class ValueType {
    NIL,
    BOOL,
    NUMBER,
    OBJECT
};

// Value representation
struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Object* object;
    } as;
    
    // Type queries
    bool isNil() const { return type == ValueType::NIL; }
    bool isBool() const { return type == ValueType::BOOL; }
    bool isNumber() const { return type == ValueType::NUMBER; }
    bool isObject() const { return type == ValueType::OBJECT; }
    
    // Type-safe accessors
    bool asBool() const;
    double asNumber() const;
    Object* asObject() const;
    
    // Factory methods
    static Value nilVal();
    static Value boolVal(bool value);
    static Value numberVal(double value);
    static Value objectVal(Object* obj);
    
    // Comparison
    bool operator==(const Value& other) const;
    bool isFalsey() const;
};

// Value array for constants
class ValueArray {
public:
    void write(Value value);
    size_t getCount() const { return values_.size(); }
    Value operator[](size_t index) const { return values_[index]; }
    
private:
    std::vector<Value> values_;
};

} // namespace ms
```

### 2.6 Object System (`object.hh/cc`)

#### 2.6.1 Object Types and Structure

```cpp
namespace ms {

enum class ObjectType {
    STRING,
    FUNCTION,
    CLOSURE,
    UPVALUE,
    CLASS,
    INSTANCE,
    BOUND_METHOD,
    MODULE,
    LIST
};

// Base object class
class Object {
public:
    ObjectType type;
    Object* next;  // Intrusive linked list for GC
    bool isMarked; // GC mark bit
    
    virtual ~Object() = default;
    virtual std::string toString() const = 0;
    virtual void traceReferences() = 0;  // For GC
    
protected:
    explicit Object(ObjectType type);
};

// String object
class StringObject : public Object {
public:
    std::string chars;
    uint32_t hash;  // Cached hash for table use
    
    static StringObject* create(std::string_view str);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit StringObject(std::string_view str);
};

// Function object
class FunctionObject : public Object {
public:
    int arity;
    int upvalueCount;
    BytecodeChunk chunk;
    StringObject* name;
    bool isMethod;  // Distinguish methods from functions
    
    static FunctionObject* create(StringObject* name);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit FunctionObject(StringObject* name);
};

// Upvalue object
class UpvalueObject : public Object {
public:
    Value* location;  // Pointer to stack slot
    Value closed;     // Captured value when closed
    UpvalueObject* next;
    
    static UpvalueObject* create(Value* slot);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit UpvalueObject(Value* slot);
};

// Closure object
class ClosureObject : public Object {
public:
    FunctionObject* function;
    std::vector<UpvalueObject*> upvalues;
    
    static ClosureObject* create(FunctionObject* function);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit ClosureObject(FunctionObject* function);
};

// Class object
class ClassObject : public Object {
public:
    StringObject* name;
    ClassObject* superclass;
    Table methods;
    
    static ClassObject* create(StringObject* name);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit ClassObject(StringObject* name);
};

// Instance object
class InstanceObject : public Object {
public:
    ClassObject* klass;
    Table fields;
    
    static InstanceObject* create(ClassObject* klass);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit InstanceObject(ClassObject* klass);
};

// Bound method object
class BoundMethodObject : public Object {
public:
    Value receiver;
    ClosureObject* method;
    
    static BoundMethodObject* create(Value receiver, ClosureObject* method);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    BoundMethodObject(Value receiver, ClosureObject* method);
};

// Module object
class ModuleObject : public Object {
public:
    StringObject* name;
    std::string path;
    Table exports;  // Exported symbols
    bool isLoaded;
    
    static ModuleObject* create(StringObject* name);
    std::string toString() const override;
    void traceReferences() override;
    
private:
    explicit ModuleObject(StringObject* name);
};

// List object (for arrays)
class ListObject : public Object {
public:
    std::vector<Value> elements;
    
    static ListObject* create();
    static ListObject* create(std::vector<Value>&& elements);
    std::string toString() const override;
    void traceReferences() override;
    
    // List methods
    void append(Value value);
    Value get(size_t index);
    void set(size_t index, Value value);
    size_t length() const { return elements.size(); }
    
private:
    ListObject() : Object(ObjectType::LIST) {}
};

} // namespace ms
```

### 2.7 Hash Table (`table.hh/cc`)

#### 2.7.1 Open Addressing Hash Map

```cpp
namespace ms {

struct TableEntry {
    StringObject* key;
    Value value;
    
    TableEntry() : key(nullptr), value(Value::nilVal()) {}
};

class Table {
public:
    Table();
    
    bool set(StringObject* key, Value value);
    bool get(StringObject* key, Value* value) const;
    bool remove(StringObject* key);
    void addAll(Table* from);
    StringObject* findString(std::string_view chars) const;
    
    size_t getCount() const { return count_; }
    size_t getCapacity() const { return entries_.size(); }
    
    // Iteration
    class Iterator {
        // ... iterator implementation
    };
    Iterator begin();
    Iterator end();
    
    // GC support
    void removeWhite();
    void mark();
    
private:
    std::vector<TableEntry> entries_;
    size_t count_;
    
    Entry* findEntry(StringObject* key);
    void adjustCapacity(size_t capacity);
};

} // namespace ms
```

### 2.8 Compiler (`compiler.hh/cc`)

#### 2.8.1 Single-Pass Compiler

```cpp
namespace ms {

class Compiler {
public:
    Compiler();
    
    std::expected<FunctionObject*, CompileError> 
    compile(std::vector<std::unique_ptr<Stmt>> statements);
    
private:
    // Compilation state
    struct Local {
        Token name;
        size_t depth;
        bool isCaptured;
    };
    
    struct Upvalue {
        size_t index;
        bool isLocal;
    };
    
    enum class FunctionType {
        FUNCTION,
        METHOD,
        INITIALIZER,
        SCRIPT
    };
    
    struct CompilerState {
        CompilerState* enclosing;
        FunctionObject* function;
        FunctionType type;
        
        std::vector<Local> locals;
        std::vector<Upvalue> upvalues;
        int scopeDepth;
        
        std::vector<size_t> breakJumps;     // For break statements
        std::vector<size_t> continueJumps;  // For continue statements
        size_t loopStart;                   // Loop start for continue
    };
    
    CompilerState* current_;
    Parser parser_;
    
    // Code emission
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitLoop(size_t loopStart);
    size_t emitJump(uint8_t instruction);
    void emitReturn();
    void emitConstant(Value value);
    void patchJump(size_t offset);
    void emitJumpIfFalse(size_t offset);
    
    // Constant management
    size_t makeConstant(Value value);
    size_t identifierConstant(Token name);
    
    // Variable handling
    void declareVariable();
    void defineVariable(size_t global);
    void markInitialized();
    int resolveLocal(CompilerState* state, Token name);
    int addUpvalue(CompilerState* state, size_t index, bool isLocal);
    int resolveUpvalue(CompilerState* state, Token name);
    
    // Scope handling
    void beginScope();
    void endScope();
    
    // Expression compilation
    void compileExpression(const Expr& expr);
    void compileLiteral(const LiteralExpr& expr);
    void compileVariable(const VariableExpr& expr);
    void compileAssignment(const AssignExpr& expr);
    void compileBinary(const BinaryExpr& expr);
    void compileUnary(const UnaryExpr& expr);
    void compileLogical(const LogicalExpr& expr);
    void compileCall(const CallExpr& expr);
    void compileGet(const GetExpr& expr);
    void compileSet(const SetExpr& expr);
    void compileThis(const ThisExpr& expr);
    void compileSuper(const SuperExpr& expr);
    void compileList(const ListExpr& expr);
    void compileSubscript(const SubscriptExpr& expr);
    
    // Statement compilation
    void compileStatement(const Stmt& stmt);
    void compileBlock(const BlockStmt& stmt);
    void compileVarDeclaration(const VarStmt& stmt);
    void compileFunctionDeclaration(const FunctionStmt& stmt);
    void compileClassDeclaration(const ClassStmt& stmt);
    void compileIfStatement(const IfStmt& stmt);
    void compileWhileStatement(const WhileStmt& stmt);
    void compileForStatement(const ForStmt& stmt);
    void compileReturnStatement(const ReturnStmt& stmt);
    void compileImportStatement(const ImportStmt& stmt);
    void compileBreakStatement(const BreakStmt& stmt);
    void compileContinueStatement(const ContinueStmt& stmt);
    void compileExpressionStatement(const ExpressionStmt& stmt);
    
    // Function compilation
    FunctionObject* compileFunction(const FunctionStmt& stmt, 
                                    FunctionType type);
};

} // namespace ms
```

### 2.9 Virtual Machine (`vm.hh/cc`)

#### 2.9.1 Stack-Based VM

```cpp
namespace ms {

class VM {
public:
    VM();
    ~VM();
    
    InterpretResult interpret(const std::string& source);
    InterpretResult interpret(FunctionObject* function);
    
    // Module support
    bool importModule(const std::string& moduleName);
    bool importFromModule(const std::string& moduleName,
                         const std::string& itemName,
                         const std::string& alias = "");
    
private:
    // Execution state
    static constexpr size_t STACK_MAX = 256;
    static constexpr size_t FRAMES_MAX = 64;
    static constexpr size_t GC_HEAP_GROW_FACTOR = 2;
    
    struct CallFrame {
        ClosureObject* closure;
        uint8_t* ip;          // Instruction pointer
        Value* slots;         // Stack slots for this frame
    };
    
    // VM state
    std::array<Value, STACK_MAX> stack_;
    Value* stackTop_;
    std::array<CallFrame, FRAMES_MAX> frames_;
    size_t frameCount_;
    
    // Global state
    Table globals_;
    Table strings_;          // String interning table
    
    // Module system
    std::unordered_map<std::string, ModuleObject*> modules_;
    std::vector<std::string> modulePaths_;
    
    // Memory management
    Object* objects_;        // Linked list of all objects
    size_t bytesAllocated_;
    size_t nextGC_;
    size_t grayCount_;
    size_t grayCapacity_;
    std::vector<Object*> grayStack_;
    
    // Built-in functions
    void defineNatives();
    
    // Stack operations
    void push(Value value);
    Value pop();
    Value peek(int distance);
    void resetStack();
    
    // Execution
    InterpretResult run();
    bool callValue(Value callee, int argCount);
    bool call(ClosureObject* closure, int argCount);
    bool invokeFromClass(ClassObject* klass, 
                        StringObject* name, 
                        int argCount);
    bool invoke(StringObject* name, int argCount);
    bool bindMethod(ClassObject* klass, StringObject* name);
    
    // Upvalue handling
    UpvalueObject* captureUpvalue(Value* local);
    void closeUpvalues(Value* last);
    
    // Error handling
    void runtimeError(const std::string& message);
    
    // Garbage collection
    void collectGarbage();
    void markObject(Object* object);
    void markValue(Value value);
    void markRoots();
    void traceReferences();
    void sweep();
    void blackenObject(Object* object);
    
    // Module helpers
    std::string resolveModulePath(const std::string& moduleName);
    bool loadModule(const std::string& path, ModuleObject* module);
    
    // Debug support
    void printStack();
    void disassembleInstruction(uint8_t* code, size_t offset);
};

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

} // namespace ms
```

### 2.10 Module System (`module.hh/cc`)

#### 2.10.1 Module Loader Design

```cpp
namespace ms {

class ModuleLoader {
public:
    ModuleLoader();
    
    std::expected<ModuleObject*, ModuleError> 
    loadModule(const std::string& moduleName, VM& vm);
    
    void addSearchPath(const std::string& path);
    void setSearchPaths(const std::vector<std::string>& paths);
    
    ModuleObject* getLoadedModule(const std::string& moduleName);
    bool isModuleLoaded(const std::string& moduleName) const;
    
private:
    std::vector<std::string> searchPaths_;
    std::unordered_map<std::string, ModuleObject*> loadedModules_;
    
    std::string resolvePath(const std::string& moduleName);
    bool fileExists(const std::string& path);
    std::string readFile(const std::string& path);
    
    bool detectCircularDependency(const std::string& moduleName);
};

struct ModuleError {
    std::string moduleName;
    std::string message;
    std::source_location location;
};

} // namespace ms
```

### 2.11 Logger System (`logger.hh/cc`)

#### 2.11.1 Colored Logger Implementation

```cpp
namespace ms {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    OFF
};

class Logger {
public:
    static void setLevel(LogLevel level);
    static void setOutput(std::ostream* stream);
    static void enableColors(bool enable);
    static void enableTimestamp(bool enable);
    static void enableSourceLocation(bool enable);
    
    // Logging functions
    template<typename... Args>
    static void trace(std::format_string<Args...> fmt, Args&&... args);
    
    template<typename... Args>
    static void debug(std::format_string<Args...> fmt, Args&&... args);
    
    template<typename... Args>
    static void info(std::format_string<Args...> fmt, Args&&... args);
    
    template<typename... Args>
    static void warn(std::format_string<Args...> fmt, Args&&... args);
    
    template<typename... Args>
    static void error(std::format_string<Args...> fmt, Args&&... args);
    
    template<typename... Args>
    static void fatal(std::format_string<Args...> fmt, Args&&... args);
    
    // With source location (C++23 feature)
    template<typename... Args>
    static void trace(std::format_string<Args...> fmt,
                     std::source_location loc = std::source_location::current(),
                     Args&&... args);
    
private:
    static LogLevel currentLevel_;
    static std::ostream* output_;
    static bool colorsEnabled_;
    static bool timestampEnabled_;
    static bool sourceLocationEnabled_;
    
    static void log(LogLevel level, const std::string& message,
                   std::source_location loc = std::source_location::current());
    
    static std::string levelToString(LogLevel level);
    static std::string levelToColor(LogLevel level);
    static std::string getTimestamp();
    static void resetColor();
};

// ANSI color codes
namespace Color {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BLACK   = "\033[30m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* BLUE    = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* WHITE   = "\033[37m";
    constexpr const char* GRAY    = "\033[90m";
    
    // Bright colors
    constexpr const char* BRIGHT_RED     = "\033[91m";
    constexpr const char* BRIGHT_GREEN   = "\033[92m";
    constexpr const char* BRIGHT_YELLOW  = "\033[93m";
    constexpr const char* BRIGHT_BLUE    = "\033[94m";
    constexpr const char* BRIGHT_MAGENTA = "\033[95m";
}

} // namespace ms

// Macro helpers for conditional compilation
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
  #define MS_LOG_LEVEL 5  // Default to INFO
#endif
```

### 2.12 Memory Management (`memory.hh/cc`)

#### 2.12.1 Garbage Collector

```cpp
namespace ms {

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();
    
    // Allocation
    void* allocate(size_t size);
    void* reallocate(void* pointer, size_t oldSize, size_t newSize);
    void deallocate(void* pointer, size_t size);
    
    // Object management
    template<typename T, typename... Args>
    T* createObject(Args&&... args);
    
    void addObject(Object* object);
    void freeObject(Object* object);
    void freeObjects();
    
    // Garbage collection
    void collectGarbage();
    void setGCThreshold(size_t bytes);
    void enableGC(bool enable);
    
    // Statistics
    size_t getBytesAllocated() const { return bytesAllocated_; }
    size_t getObjectCount() const { return objectCount_; }
    size_t getGCCollectionCount() const { return gcCount_; }
    
private:
    Object* objects_;
    size_t bytesAllocated_;
    size_t objectCount_;
    size_t nextGC_;
    size_t gcCount_;
    bool gcEnabled_;
    
    // GC state
    std::vector<Object*> grayStack_;
    
    // Tracking allocations (debug mode)
#ifdef MS_DEBUG_MEMORY
    std::unordered_map<void*, AllocationInfo> allocations_;
#endif
};

// RAII wrapper for GC roots
class GCRoot {
public:
    explicit GCRoot(Value* value);
    ~GCRoot();
    
    GCRoot(const GCRoot&) = delete;
    GCRoot& operator=(const GCRoot&) = delete;
    
private:
    Value* value_;
};

} // namespace ms
```

### 2.13 Platform Abstraction (`platform.hh/cc`)

```cpp
namespace ms::platform {

// File operations
bool fileExists(const std::string& path);
std::string readFile(const std::string& path);
bool writeFile(const std::string& path, const std::string& content);
std::string getCurrentDirectory();
std::string joinPath(const std::string& a, const std::string& b);
std::string getExecutablePath();

// Console operations
void enableConsoleColors();
bool supportsConsoleColors();
void setConsoleColor(const std::string& color);
void resetConsoleColor();

// Time operations
double getTimeInSeconds();  // High-resolution timer
std::string formatTimestamp(std::chrono::system_clock::time_point time);

// Environment
std::string getEnvironmentVariable(const std::string& name);
std::vector<std::string> getPathSeparator();

// Platform detection
inline constexpr bool isWindows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

inline constexpr bool isLinux() {
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

inline constexpr bool isMacOS() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

} // namespace ms::platform
```

## 3. Import System Design

### 3.1 Import Statement Parsing

```
import <module> ;
from <module> import <item> (as <alias>)? (, <item> (as <alias>)?)* ;
```

### 3.2 Module Resolution Algorithm

```
1. Check if module already loaded (cache)
   - If yes, return cached module
   - If no, continue

2. Resolve module path:
   a. Try current directory: "./<module>.maple"
   b. Try search paths: for path in searchPaths:
      - "<path>/<module>.maple"
   c. If not found, error

3. Check for circular dependencies
   - Maintain loading stack
   - If module in loading stack, error

4. Load and compile module:
   a. Read file
   b. Compile to bytecode
   c. Create ModuleObject
   d. Execute module code
   e. Capture exports (top-level declarations)

5. Cache module for future imports

6. Bind imported symbols to current scope
```

### 3.3 Bytecode for Imports

```
OP_IMPORT <module_name_constant>
  → Pushes module object onto stack
  → Module object contains all exports

OP_IMPORT_FROM <module_constant> <item_constant> <alias_constant?>
  → Imports specific item from module
  → If alias provided, binds to alias name
  → Otherwise binds to original name
```

## 4. Error Handling Strategy

### 4.1 Compile-Time Errors

```cpp
struct CompileError {
    std::string message;
    Token token;
    std::string sourceLine;
    std::vector<std::string> suggestions;
    
    std::string format() const;
};
```

### 4.2 Runtime Errors

```cpp
struct RuntimeError {
    std::string message;
    std::vector<StackFrame> stackTrace;
    std::source_location location;
    
    std::string format() const;
};
```

### 4.3 Error Recovery

- **Lexer**: Skip to next token
- **Parser**: Synchronization (skip to next statement)
- **Compiler**: Stop after first error
- **VM**: Unwind stack, report error

## 5. Testing Strategy

### 5.1 Unit Tests

```
tests/
├── unit/
│   ├── test_scanner.cc
│   ├── test_parser.cc
│   ├── test_compiler.cc
│   ├── test_vm.cc
│   ├── test_gc.cc
│   ├── test_table.cc
│   └── test_logger.cc
```

### 5.2 Integration Tests (Maple Scripts)

```
tests/
├── basic/
│   ├── arithmetic.maple
│   ├── strings.maple
│   ├── variables.maple
│   └── control_flow.maple
├── functions/
│   ├── basic.maple
│   ├── closures.maple
│   └── recursion.maple
├── classes/
│   ├── basic.maple
│   ├── inheritance.maple
│   └── methods.maple
├── modules/
│   ├── math.maple
│   ├── utils.maple
│   ├── test_import.maple
│   └── test_from_import.maple
└── errors/
    ├── syntax_errors.maple
    ├── runtime_errors.maple
    └── import_errors.maple
```

### 5.3 Test Framework

Simple test framework using C++23 features:

```cpp
namespace ms::test {

class TestSuite {
public:
    void addTest(std::string name, std::function<void()> test);
    void run();
    void reportResults();
    
private:
    struct Test {
        std::string name;
        std::function<void()> func;
        bool passed;
        std::string error;
    };
    
    std::vector<Test> tests_;
};

// Assertions
#define ASSERT_TRUE(cond) \
    if (!(cond)) throw TestFailure(#cond);
    
#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) \
        throw TestFailure(std::format("Expected {}, got {}", expected, actual));

} // namespace ms::test
```

## 6. Build Configuration

### 6.1 CMakeLists.txt Structure

```cmake
cmake_minimum_required(VERSION 3.25)
project(Maple LANGUAGES CXX)

# C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific flags
if(MSVC)
    add_compile_options(/W4 /WX-)
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Werror)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.cc")
file(GLOB_RECURSE HEADERS "src/*.hh" "include/**/*.hh")

# Main executable
add_executable(maple ${SOURCES})
target_include_directories(maple PRIVATE src include)

# Optional: Shared library
add_library(maple_lib SHARED ${SOURCES})
target_include_directories(maple_lib PUBLIC include)

# Tests
option(BUILD_TESTS "Build test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS maple RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
```

## 7. Performance Considerations

### 7.1 Optimizations

1. **String Interning**: All strings are interned in a global table
2. **Inline Caching**: Cache method lookups (future)
3. **Bytecode Optimization**: Peephole optimizer (optional)
4. **Stack Allocation**: Pre-allocate stack to avoid reallocation
5. **Hash Table**: Prime number sizes, good hash function

### 7.2 Memory Layout

- Objects allocated on heap
- Values stored inline (NaN boxing or tagged union)
- Stack pre-allocated, fixed size
- Bytecode chunks grow dynamically

## 8. Future Extensions

### 8.1 Potential Enhancements

- **JIT Compilation**: Compile hot paths to native code
- **Generational GC**: Improve collection performance
- **Coroutines**: Lightweight concurrency
- **Pattern Matching**: Destructuring assignments
- **Type System**: Optional static typing
- **Standard Library**: I/O, networking, etc.

### 8.2 Tooling

- **Debugger**: Breakpoints, stepping, inspection
- **Profiler**: Identify hot spots
- **LSP**: IDE integration
- **Package Manager**: Dependency management

---

This design document provides a comprehensive blueprint for implementing the Maple scripting language, covering all major components, data structures, algorithms, and implementation strategies necessary to build a production-ready interpreter.
