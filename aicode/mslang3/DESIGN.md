# Maple 脚本语言设计文档

## 1. 概述

Maple是一个基于字节码的脚本语言，参考自Crafting Interpreters中的clox实现，使用C++23标准进行现代化重构。

### 1.1 设计目标
- 高性能的字节码执行引擎
- 自动内存管理（Mark-Sweep GC）
- 现代化的C++实现
- 模块化的import系统
- 跨平台支持（Windows/Linux）

## 2. 架构设计

### 2.1 整体架构
```
Source Code -> Scanner -> Compiler -> Bytecode -> VM -> Output
                                    ^
                                    |
                              Garbage Collector
```

### 2.2 模块划分

| 模块 | 描述 | 核心文件 |
|------|------|----------|
| common | 基础定义、配置常量 | common.hh |
| memory | 内存管理、GC | memory.hh/cc |
| value | 值类型系统 | value.hh/cc |
| object | 对象系统 | object.hh/cc |
| chunk | 字节码块 | chunk.hh/cc |
| debug | 调试工具 | debug.hh/cc |
| logger | 日志系统 | logger.hh/cc |
| scanner | 词法分析 | scanner.hh/cc |
| compiler | 编译器 | compiler.hh/cc |
| vm | 虚拟机 | vm.hh/cc |
| import | 模块导入 | import.hh/cc |
| main | 入口 | main.cc |

## 3. 核心组件设计

### 3.1 Value系统（值类型）

采用NaN Boxing技术实现高效的动态类型：

```cpp
// 64位Value实现
// 使用IEEE 754 NaN的特殊编码空间存储指针
// quiet NaN: 0x7FF8000000000000
// 使用低位52位存储指针信息

enum class ValueType : uint8_t {
    Nil,
    Bool,
    Number,
    Object  // String, Function, Class, Instance等
};

class Value {
private:
    uint64_t bits_;
    
public:
    // 构造、类型检查、类型转换
    // 使用constexpr优化编译期计算
};
```

**优势**：
- 单一8字节存储，无需堆分配
- 直接值传递，缓存友好
- 无指针解引用开销（基础类型）

### 3.2 对象系统

所有堆分配对象继承自Obj基类：

```cpp
enum class ObjType : uint8_t {
    String,
    Function,
    Native,
    Closure,
    Upvalue,
    Class,
    Instance,
    BoundMethod,
    Module  // 新增：模块对象
};

struct Obj {
    ObjType type;
    bool isMarked;      // GC标记
    Obj* next;          // GC链表
};
```

### 3.3 字节码设计

```cpp
enum class OpCode : uint8_t {
    // 常量
    CONSTANT,           // 加载常量 (idx: u8)
    CONSTANT_LONG,      // 加载常量 (idx: u24)
    
    // 字面量
    NIL,
    TRUE,
    FALSE,
    
    // 栈操作
    POP,
    DUP,
    
    // 局部变量
    GET_LOCAL,          // (slot: u8)
    SET_LOCAL,          // (slot: u8)
    GET_LOCAL_LONG,     // (slot: u16)
    SET_LOCAL_LONG,     // (slot: u16)
    
    // 全局变量
    GET_GLOBAL,         // (name: const_idx)
    SET_GLOBAL,         // (name: const_idx)
    DEFINE_GLOBAL,      // (name: const_idx)
    
    // Upvalue
    GET_UPVALUE,        // (slot: u8)
    SET_UPVALUE,        // (slot: u8)
    
    // 属性访问
    GET_PROPERTY,       // (name: const_idx)
    SET_PROPERTY,       // (name: const_idx)
    
    // 方法调用
    GET_SUPER,          // (name: const_idx)
    
    // 比较
    EQUAL,
    GREATER,
    LESS,
    
    // 算术
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO,             // 新增
    NEGATE,
    NOT,
    
    // 跳转
    JUMP,               // (offset: u16)
    JUMP_IF_FALSE,      // (offset: u16)
    LOOP,               // (offset: u16)
    
    // 函数调用
    CALL,               // (arg_count: u8)
    INVOKE,             // (name: const_idx, arg_count: u8)
    SUPER_INVOKE,       // (name: const_idx, arg_count: u8)
    CLOSURE,            // (func: const_idx, upvalues...)
    
    // 返回
    RETURN,
    
    // 类
    CLASS,              // (name: const_idx)
    INHERIT,
    METHOD,             // (name: const_idx)
    
    // 闭包支持
    CLOSE_UPVALUE,
    
    // 模块导入（新增）
    IMPORT,             // (module: const_idx)
    IMPORT_FROM,        // (module: const_idx, names...)
    IMPORT_AS,          // (module: const_idx, alias: const_idx)
    
    // 集合（可选）
    BUILD_LIST,         // (count: u16)
    BUILD_MAP,          // (count: u16)
    INDEX_GET,
    INDEX_SET,
};
```

### 3.4 VM设计

```cpp
class VM {
private:
    // 执行状态
    CallFrame frames_[FRAMES_MAX];
    int frameCount_;
    
    // 值栈
    Value stack_[STACK_MAX];
    Value* stackTop_;
    
    // 对象链表（GC用）
    Obj* objects_;
    
    // 全局变量
    Table globals_;
    
    // 字符串驻留池
    Table strings_;
    
    // 开放upvalue链表
    ObjUpvalue* openUpvalues_;
    
    // 加载的模块
    Table modules_;     // 新增
    
    // GC状态
    size_t bytesAllocated_;
    size_t nextGC_;
    
    // 当前模块（新增）
    ObjModule* currentModule_;
    
public:
    InterpretResult interpret(const std::string& source);
    
private:
    InterpretResult run();
    
    // 栈操作
    void push(Value value);
    Value pop();
    Value peek(int distance);
    
    // 调用
    bool callValue(Value callee, int argCount);
    bool call(ObjClosure* closure, int argCount);
    bool invoke(ObjString* name, int argCount);
    bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount);
    
    // 属性绑定
    bool bindMethod(ObjClass* klass, ObjString* name);
    
    // Upvalue
    ObjUpvalue* captureUpvalue(Value* local);
    void closeUpvalues(Value* last);
    void defineMethod(ObjString* name);
    
    // 运行时错误
    void runtimeError(const char* format, ...);
    
    // 模块导入（新增）
    bool importModule(ObjString* name);
    bool importFromModule(ObjString* module, ObjString* name, ObjString* alias);
};
```

## 4. 垃圾回收设计

### 4.1 Mark-Sweep算法

```cpp
class GC {
public:
    // 触发GC
    void collect();
    
private:
    // 标记阶段
    void markRoots();
    void markValue(Value value);
    void markObject(Obj* object);
    void markArray(ValueArray* array);
    void traceReferences();
    void blackenObject(Obj* object);
    
    // 清理阶段
    void sweep();
    void removeWhiteStrings();  // 清理驻留池
    
    // 标记栈
    std::vector<Obj*> grayStack_;
};
```

### 4.2 根集合

- VM栈上的所有值
- 全局变量表
- 开放upvalue链表
- 当前模块
- 编译器中的常量（编译期）

### 4.3 触发时机

```cpp
// 分配时检查
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;
    
    if (newSize > oldSize) {
        #ifdef DEBUG_STRESS_GC
        collectGarbage();
        #endif
        
        if (vm.bytesAllocated > vm.nextGC) {
            collectGarbage();
        }
    }
    
    return std::realloc(pointer, newSize);
}
```

## 5. Import系统设计

### 5.1 语法

```maple
// 完整导入
import "math"
math.sin(3.14)

// 选择性导入
from "math" import sin, cos
sin(3.14)

// 别名导入
import "io" as filesystem
filesystem.read("file.txt")

from "math" import sin as sine
eine(3.14)
```

### 5.2 模块搜索路径

```cpp
class ImportResolver {
private:
    std::vector<std::string> searchPaths_;
    
public:
    // 搜索模块文件
    std::optional<std::string> resolve(const std::string& name);
    
    // 标准路径：
    // 1. 当前目录 ./
    // 2. MAPLE_PATH环境变量
    // 3. 标准库目录 $MAPLE_HOME/lib/
};
```

### 5.3 模块缓存

```cpp
// 模块只加载一次，结果缓存在vm.modules_中
Table modules_;  // name -> ObjModule*

// ObjModule结构
struct ObjModule : Obj {
    ObjString* name;
    Table exports;      // 导出的变量/函数
    Table globals;      // 模块私有全局变量
    ObjFunction* init;  // 模块初始化函数
};
```

### 5.4 字节码实现

```
import "math"
--------
CONSTANT    "math"
IMPORT      
POP         // 导入结果（模块对象）通常不留在栈上

from "math" import sin
--------
CONSTANT    "math"
CONSTANT    "sin"
GET_GLOBAL  "math"
GET_PROPERTY "sin"
DEFINE_GLOBAL "sin"

import "io" as filesystem
--------
CONSTANT    "io"
CONSTANT    "filesystem"
IMPORT_AS
```

## 6. Logger系统设计

### 6.1 日志等级

```cpp
enum class LogLevel : uint8_t {
    Trace,      // 最详细
    Debug,      // 调试信息
    Info,       // 一般信息
    Warn,       // 警告
    Error,      // 错误
    Fatal       // 致命
};
```

### 6.2 颜色支持

```cpp
class Logger {
public:
    // 带颜色的日志输出
    template<typename... Args>
    static void debug(const char* fmt, Args&&... args) {
        #ifdef DEBUG_MODE
        log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        #endif
    }
    
private:
    static constexpr std::string_view RESET = "\033[0m";
    static constexpr std::string_view RED = "\033[31m";
    static constexpr std::string_view GREEN = "\033[32m";
    static constexpr std::string_view YELLOW = "\033[33m";
    static constexpr std::string_view BLUE = "\033[34m";
    static constexpr std::string_view MAGENTA = "\033[35m";
    static constexpr std::string_view CYAN = "\033[36m";
    static constexpr std::string_view WHITE = "\033[37m";
    static constexpr std::string_view GRAY = "\033[90m";
    
    #ifdef _WIN32
    static void enableWindowsAnsi();  // Windows需要启用ANSI支持
    #endif
};
```

### 6.3 颜色映射

| Level | 颜色 | 用途 |
|-------|------|------|
| Trace | Gray | 最详细的执行跟踪 |
| Debug | Cyan | 调试信息，变量值 |
| Info | Green | 一般信息 |
| Warn | Yellow | 警告信息 |
| Error | Red | 错误信息 |
| Fatal | Magenta | 致命错误 |

### 6.4 特定领域日志

```cpp
// 启用/禁用特定调试功能
#define DEBUG_SCANNER      // 词法分析跟踪
#define DEBUG_PARSER       // 语法分析跟踪
#define DEBUG_COMPILE      // 编译输出
#define DEBUG_TRACE_EXEC   // 执行跟踪
#define DEBUG_STRESS_GC    // GC压力测试
#define DEBUG_LOG_GC       // GC详细日志
```

## 7. 编译器设计

### 7.1 整体流程

```cpp
class Compiler {
public:
    ObjFunction* compile(const std::string& source);
    
private:
    // 解析函数
    void declaration();
    void statement();
    void expression();
    
    // 表达式解析（Pratt Parser）
    void parsePrecedence(Precedence precedence);
    
    // 特定语法解析
    void variable(bool canAssign);
    void binary(bool canAssign);
    void unary(bool canAssign);
    void grouping(bool canAssign);
    void number(bool canAssign);
    void string(bool canAssign);
    void literal(bool canAssign);
    void call(bool canAssign);
    void dot(bool canAssign);
    void this_(bool canAssign);
    void super_(bool canAssign);
    
    // 新增：import解析
    void importDeclaration();
    void fromImportDeclaration();
};
```

### 7.2 词法分析器

```cpp
enum class TokenType : uint8_t {
    // 单字符
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,  // 新增：列表支持
    COMMA, DOT, MINUS, PLUS,
    SEMICOLON, SLASH, STAR, PERCENT,  // 新增：%
    
    // 单/双字符
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    
    // 字面量
    IDENTIFIER, STRING, NUMBER,
    
    // 关键字
    AND, CLASS, ELSE, FALSE,
    FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS,
    TRUE, VAR, WHILE,
    
    // 新增关键字
    IMPORT, FROM, AS,  // import系统
    
    // 特殊
    ERROR, EOF
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
};

class Scanner {
public:
    explicit Scanner(std::string_view source);
    Token scanToken();
    
private:
    std::string_view source_;
    const char* start_;
    const char* current_;
    int line_;
    
    // 辅助方法
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    
    Token makeToken(TokenType type);
    Token errorToken(const char* message);
    
    void skipWhitespace();
    Token string();
    Token number();
    Token identifier();
    TokenType identifierType();
};
```

## 8. 文件组织

```
maple/
├── CMakeLists.txt
├── docs/
│   ├── DESIGN.md
│   └── REQUIREMENTS.md
├── src/
│   ├── common.hh
│   ├── memory.hh/cc
│   ├── value.hh/cc
│   ├── object.hh/cc
│   ├── chunk.hh/cc
│   ├── debug.hh/cc
│   ├── logger.hh/cc
│   ├── scanner.hh/cc
│   ├── compiler.hh/cc
│   ├── vm.hh/cc
│   ├── import.hh/cc
│   └── main.cc
├── lib/
│   └── std/
│       ├── math.maple
│       ├── io.maple
│       └── sys.maple
└── tests/
    ├── test_basic.maple
    ├── test_functions.maple
    ├── test_classes.maple
    ├── test_import.maple
    └── run_tests.py
```

## 9. C++23特性使用

### 9.1 现代化特性

| 特性 | 用途 |
|------|------|
| `std::expected` | 错误处理（替代异常） |
| `std::optional` | 可选值返回 |
| `std::string_view` | 零拷贝字符串引用（scanner） |
| `std::span` | 安全的数组视图 |
| `constexpr` | 编译期计算 |
| `consteval` | 强制编译期计算 |
| `using enum` | 简化枚举使用 |
| `std::format` | 格式化字符串 |
| `std::print` | 格式化输出（C++23） |
| `std::ranges` | 范围算法 |
| `std::to_underlying` | 枚举转底层类型 |

### 9.2 类型安全

```cpp
// 使用strong type区分索引类型
using LocalIndex = std::uint16_t;
using ConstantIndex = std::uint16_t;
using StackIndex = std::uint8_t;

// 使用enum class代替enum
enum class ObjType : uint8_t { ... };
```

## 10. 性能优化

### 10.1 已考虑的优化

1. **NaN Boxing**: 高效的值表示
2. **Copy-down继承**: 类方法快速查找
3. **String interning**: 字符串常量池
4. **Computed goto** (GCC扩展): 快速字节码分发
5. **内联缓存** (未来): 方法调用优化

### 10.2 内存布局

```cpp
// 缓存友好的对象布局
struct ObjString {
    Obj obj;
    size_t length;
    char* chars;
    uint32_t hash;  // 预计算哈希
};

// 动态数组使用连续内存
template<typename T>
class DynArray {
    T* data_;
    size_t count_;
    size_t capacity_;
};
```

## 11. 扩展性设计

### 11.1 添加新类型

1. 在`ObjType`中添加类型标识
2. 创建对应的ObjXXX结构体
3. 实现GC遍历（mark/blacken）
4. 实现释放函数
5. 添加Value转换函数

### 11.2 添加新指令

1. 在`OpCode`中添加操作码
2. 在`debug.cc`中添加反汇编支持
3. 在`compiler.cc`中生成指令
4. 在`vm.cc`的`run()`中实现执行逻辑

### 11.3 添加标准库

```cpp
// 原生函数注册
void defineNative(const char* name, NativeFn function);

// 示例：注册math模块
void initMathModule() {
    auto& module = vm.createModule("math");
    module.defineNative("sin", mathSin);
    module.defineNative("cos", mathCos);
    module.defineNative("sqrt", mathSqrt);
    module.defineNative("PI", 3.14159265358979);
}
```

## 12. 错误处理

### 12.1 编译期错误

```cpp
struct CompileError {
    int line;
    TokenType where;
    std::string message;
};
```

### 12.2 运行时错误

```cpp
struct RuntimeError {
    int line;
    CallFrame* frame;
    std::string message;
    std::vector<std::string> stackTrace;
};
```

### 12.3 错误恢复

- 编译期：Panic模式跳过至同步点
- 运行时：打印堆栈跟踪并退出

## 13. 与clox的改进对比

| 特性 | clox (C) | Maple (C++23) |
|------|----------|---------------|
| 值表示 | NaN Boxing | NaN Boxing + 类型安全包装 |
| 内存管理 | 手动malloc/free | RAII + 智能指针辅助 |
| 字符串 | C字符串 | std::string_view + 驻留池 |
| 错误处理 | 返回码 | std::expected/std::optional |
| 容器 | 手动实现 | 标准库容器 |
| 模块化 | 无 | import系统 |
| 调试 | printf | 分级Logger |
| 跨平台 | 条件编译 | CMake + 平台抽象 |
| 模块组织 | 单文件 | 多文件，清晰分离 |
| 代码组织 | .h/.c | .hh/.cc, namespace |
