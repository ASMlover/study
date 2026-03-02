# Maple Language Architecture & Design Document

> Version: 1.0 | Date: 2026-03-01 | Author: ASMlover

---

## 1. System Architecture

### 1.1 Pipeline Overview

```
┌──────────┐    ┌──────────┐    ┌──────────────┐    ┌──────────┐
│  Source   │───>│ Scanner  │───>│   Compiler   │───>│ Bytecode │
│  Code    │    │ (Lexer)  │    │ (Pratt Parser)│    │  Chunk   │
└──────────┘    └──────────┘    └──────────────┘    └─────┬────┘
                                                          │
                                                          v
                ┌──────────┐    ┌──────────────┐    ┌──────────┐
                │  Output  │<───│     VM       │<───│ Dispatch │
                │          │    │ (Stack-based) │    │   Loop   │
                └──────────┘    └──────┬───────┘    └──────────┘
                                       │
                                       v
                                ┌──────────────┐
                                │  GC (M&S)    │
                                │ Mark → Sweep │
                                └──────────────┘
```

### 1.2 Key Design Principles

1. **单遍编译（Single-pass Compilation）** — 无 AST，Scanner 按需提供 token，Compiler 直接生成字节码
2. **栈式虚拟机（Stack-based VM）** — 操作数隐式位于栈顶，指令短小紧凑
3. **Mark-and-Sweep GC** — 三色标记，暂停式回收，simple but correct
4. **字符串驻留（String Interning）** — 所有字符串在自定义 Table 中去重，实现 O(1) 字符串比较
5. **Upvalue 机制** — 闭包通过 open/closed upvalue 实现变量捕获

---

## 2. Module Architecture

### 2.1 File Mapping

```
src/
├── Foundation Layer
│   ├── Macros.hh          → 平台检测宏 (MAPLE_GNUC, MAPLE_MSVC)
│   ├── Types.hh           → 类型别名 (u8_t, str_t, sz_t, ...)
│   ├── Consts.hh          → 编译期常量 (栈大小, GC 阈值)
│   └── Common.hh          → 工具基类 (Copyable, UnCopyable, Singleton<T>)
│
├── Infrastructure Layer
│   ├── Colorful.hh/cc     → 终端颜色抽象
│   ├── ColorfulHelper.hh  → 平台分发 (#include 对应平台 Helper)
│   ├── ColorfulHelper_POSIX.hh → ANSI 转义码实现
│   ├── ColorfulHelper_WIN32.hh → Win32 Console API 实现
│   └── Logger.hh/cc       → 分级彩色日志
│
├── Bytecode Layer
│   ├── Opcode.hh          → 操作码枚举 (enum class OpCode : u8_t)
│   ├── Value.hh/cc        → 运行时值 (std::variant<monostate, bool, double, Object*>)
│   └── Chunk.hh/cc        → 字节码容器 (code[] + constants[] + lines[])
│
├── Frontend Layer
│   ├── TokenTypes.hh      → X-macro Token 类型定义
│   ├── Token.hh/cc        → Token 结构
│   ├── Scanner.hh/cc      → 词法分析器 (逐 token 按需返回)
│   └── Compiler.hh/cc     → 编译器 (递归下降 + Pratt 表达式解析)
│
├── Runtime Layer
│   ├── Object.hh/cc       → GC 管理的对象类型层次
│   ├── Table.hh/cc        → 开放寻址哈希表 (字符串驻留)
│   ├── VM.hh/cc           → 虚拟机 (栈 + 调用帧 + 分发循环)
│   ├── Memory.hh/cc       → 垃圾回收器 (Mark-and-Sweep)
│   └── Module.hh/cc       → 模块加载器 (import 系统)
│
├── Debug Layer
│   └── Debug.hh/cc        → 字节码反汇编器
│
└── main.cc                → 入口 (REPL + 文件执行)
```

### 2.2 Dependency Graph

```
main.cc
  └── VM
       ├── Compiler
       │    ├── Scanner
       │    │    └── Token, TokenTypes
       │    ├── Chunk
       │    │    └── Value, Opcode
       │    └── Object
       ├── Memory (GC)
       │    └── Object, Table
       ├── Table
       ├── Module
       │    └── Compiler (for compiling imported files)
       ├── Debug
       │    └── Chunk, Opcode
       └── Logger
            └── Colorful
```

---

## 3. Core Data Structures

### 3.1 Value

```cpp
// 基于 std::variant 的 tagged union
using ValueStorage = std::variant<std::monostate, bool, double, Object*>;

class Value {
  ValueStorage storage_;
public:
  // 隐式构造: Value v = 3.14; Value v = nullptr; Value v = obj;
  Value() noexcept;                    // nil
  Value(bool b) noexcept;              // boolean
  Value(double d) noexcept;            // number
  Value(Object* obj) noexcept;         // object

  // 类型检查
  bool is_nil() const noexcept;
  bool is_boolean() const noexcept;
  bool is_number() const noexcept;
  bool is_object() const noexcept;

  // 值提取
  bool as_boolean() const noexcept;
  double as_number() const noexcept;
  Object* as_object() const noexcept;

  // 语义操作
  bool is_truthy() const noexcept;     // false/nil → false, 其余 → true
  bool is_equal(const Value& o) const noexcept;
  str_t stringify() const noexcept;
};
```

**Why `std::variant` over manual tagged union**:
- 编译期类型安全，`std::visit` 确保处理所有分支
- 与 C++ 类型系统一致
- 日后可无缝替换为 NaN Boxing（仅修改 Value 实现，接口不变）

**Why not `std::any` or `std::shared_ptr`**:
- `std::any` 有堆分配开销且无类型约束
- `std::shared_ptr` 与 GC 所有权冲突

### 3.2 Chunk — 字节码容器

```cpp
class Chunk {
  std::vector<u8_t> code_;       // 字节码指令流
  std::vector<Value> constants_; // 常量池
  std::vector<int> lines_;      // 每条指令对应的源码行号
public:
  void write(u8_t byte, int line);
  sz_t add_constant(Value value);
  // ... accessors
};
```

**指令编码**：所有指令以 1 字节操作码开头，后跟 0~N 字节操作数。

### 3.3 Object — GC 管理对象基类

```cpp
class Object {
public:
  ObjectType type;           // 对象类型标签
  bool is_marked = false;    // GC 标记位
  Object* next = nullptr;    // GC 对象链表（侵入式）

  virtual ~Object() = default;
  virtual str_t stringify() const noexcept = 0;
};
```

**对象类型层次**:
```
Object (abstract)
├── ObjString      — 不可变字符串 + hash
├── ObjFunction    — 用户函数 (arity, chunk, name)
├── ObjNative      — 原生函数 (C++ function pointer)
├── ObjClosure     — 函数 + upvalue 数组
├── ObjUpvalue     — 捕获的变量引用 (open: 指向栈; closed: 存值)
├── ObjClass       — 类 (name, methods table)
├── ObjInstance    — 实例 (klass pointer, fields table)
├── ObjBoundMethod — 绑定方法 (receiver + closure)
└── ObjModule      — 模块 (name, exports table)
```

**Why C++ inheritance over clox's struct embedding**:
- C++ `virtual` 提供类型安全的多态
- `dynamic_cast` 可用于调试，`static_cast` 用于高性能路径
- `virtual ~Object()` 确保正确析构

**Why raw pointers**:
- GC 拥有所有 Object 的生命周期
- `shared_ptr` 引入引用计数开销且与 GC 回收冲突
- 所有 `Object*` 通过 GC 的 `objects_` 链表追踪

### 3.4 Table — 开放寻址哈希表

```cpp
struct Entry {
  ObjString* key = nullptr;
  Value value;
};

class Table {
  std::vector<Entry> entries_;
  int count_ = 0;
  static constexpr double kMAX_LOAD = 0.75;
  // ...
};
```

**设计要点**:
- **开放寻址 + 线性探测** — 缓存友好，实现简单
- **Tombstone 删除** — 删除时标记为 tombstone（key=null, value=true），保持探测链完整
- **容量为 2 的幂** — 用 `hash & (capacity - 1)` 替代取模
- **字符串驻留查找** — `find_string(chars, length, hash)` 按内容查找，避免分配

**Why custom Table instead of `std::unordered_map`**:
- 字符串驻留需要按内容查找（`std::unordered_map` 需要先构造 key）
- GC 需要 `remove_white()` 清理不可达字符串
- 教学和性能控制目的

**Where `std::unordered_map` is used**:
- `ObjInstance::fields` — 按 `ObjString*` 指针查找（已驻留，指针即身份）
- `ObjClass::methods` — 同上
- `VM::modules_` — 按 `std::string` 路径查找模块缓存

---

## 4. Compiler Architecture

### 4.1 Single-Pass Design

```
Token Stream (on-demand from Scanner)
         │
         v
   ┌─────────────┐
   │   Compiler   │── current/previous Token
   │              │── Pratt Parser Rules Table
   │              │── Local variables array
   │              │── Upvalues array
   │              │── Scope depth tracking
   │              │── enclosing Compiler (for nested functions)
   └──────┬──────┘
          │
          v
     Bytecode Chunk
```

### 4.2 Pratt Parser

每个 TokenType 关联一个 `ParseRule`：

```cpp
struct ParseRule {
  ParseFn prefix;         // 前缀解析函数（一元运算、字面量、分组）
  ParseFn infix;          // 中缀解析函数（二元运算、调用、属性访问）
  Precedence precedence;  // 优先级
};
```

核心算法 `parse_precedence(Precedence)`:
1. 读取当前 token，调用其 prefix handler
2. 循环：如果下一个 token 的优先级 >= 当前优先级，调用其 infix handler
3. 递归下降自然处理了运算符优先级和结合性

### 4.3 Scope & Variable Resolution

```
全局变量:
  OP_DEFINE_GLOBAL → 在 globals table 中定义
  OP_GET_GLOBAL    → 从 globals table 读取
  OP_SET_GLOBAL    → 向 globals table 写入

局部变量:
  编译器维护 locals[] 数组，按声明顺序对应栈槽位
  OP_GET_LOCAL(slot) → 从 frame->slots[slot] 读取
  OP_SET_LOCAL(slot) → 向 frame->slots[slot] 写入

Upvalue (闭包捕获):
  OP_GET_UPVALUE(index) → 从 closure->upvalues[index] 读取
  OP_SET_UPVALUE(index) → 向 closure->upvalues[index] 写入
  OP_CLOSE_UPVALUE      → 关闭 upvalue (栈 → 堆)
```

### 4.4 Nested Function Compilation

函数编译创建新的 `Compiler` 实例（ `enclosing_` 指向外层）:

```
Compiler (script-level)
  │
  ├── compile "fun outer() { ... }"
  │     └── Compiler (outer)
  │           │
  │           ├── compile "fun inner() { ... }"
  │           │     └── Compiler (inner)
  │           │           └── resolve upvalues → outer's locals
  │           │
  │           └── emit OP_CLOSURE + upvalue metadata
  │
  └── emit OP_DEFINE_GLOBAL "outer"
```

---

## 5. Virtual Machine Design

### 5.1 VM State

```cpp
class VM : public Singleton<VM> {
  // 调用帧栈
  std::array<CallFrame, kFRAMES_MAX> frames_;
  int frame_count_ = 0;

  // 值栈
  std::array<Value, kSTACK_MAX * kFRAMES_MAX> stack_;
  Value* stack_top_;

  // 全局状态
  Table globals_;                    // 全局变量
  Table strings_;                    // 驻留字符串
  ObjString* init_string_;          // 缓存的 "init" 字符串
  ObjUpvalue* open_upvalues_;       // 打开的 upvalue 链表

  // GC 状态
  Object* objects_;                  // 所有对象链表头
  sz_t bytes_allocated_ = 0;
  sz_t next_gc_;
  std::vector<Object*> gray_stack_; // 灰色对象栈

  // 模块缓存
  std::unordered_map<str_t, ObjModule*> modules_;
};
```

### 5.2 CallFrame

```cpp
struct CallFrame {
  ObjClosure* closure;   // 当前执行的闭包
  u8_t* ip;              // 指令指针（指向 closure->function->chunk.code）
  Value* slots;          // 栈基址（指向 VM 栈中的帧起始位置）
};
```

**调用约定**:
```
栈布局 (调用 f(a, b)):

slot[0] = f (closure itself)    ← frame->slots
slot[1] = a (arg 0)
slot[2] = b (arg 1)
slot[3] = (first local var)
...
```

### 5.3 Execution Loop

```cpp
InterpretResult VM::run() {
  CallFrame* frame = &frames_[frame_count_ - 1];

  for (;;) {
    u8_t instruction = READ_BYTE();
    switch (static_cast<OpCode>(instruction)) {
      case OpCode::OP_CONSTANT: { ... }
      case OpCode::OP_ADD:      { ... }
      case OpCode::OP_CALL:     { ... }
      case OpCode::OP_RETURN:   { ... }
      // ... 40+ opcodes
    }
  }
}
```

**辅助宏/内联函数**:
- `READ_BYTE()` — 读取 `*frame->ip++`
- `READ_SHORT()` — 读取 2 字节（跳转偏移）
- `READ_CONSTANT()` — 读取常量 `chunk.constants[READ_BYTE()]`
- `READ_STRING()` — 读取字符串常量
- `BINARY_OP(op)` — 弹出两个操作数，执行运算，推入结果

---

## 6. Bytecode Instruction Set

### 6.1 Instruction Format

| 格式 | 字节数 | 描述 |
|------|--------|------|
| Simple | 1 | 仅操作码，无操作数 (OP_ADD, OP_RETURN, ...) |
| Byte | 2 | 操作码 + 1字节操作数 (OP_GET_LOCAL slot) |
| Constant | 2 | 操作码 + 1字节常量索引 (OP_CONSTANT idx) |
| Jump | 3 | 操作码 + 2字节偏移 (OP_JUMP offset_hi offset_lo) |
| Closure | variable | 操作码 + 常量索引 + N*(isLocal + index) |
| Invoke | 3 | 操作码 + 方法名索引 + 参数数 |

### 6.2 Complete Opcode Table

```
┌─────────────────────┬────────┬──────────────────────────────────────────┐
│ Opcode              │ 操作数 │ 栈效果                                   │
├─────────────────────┼────────┼──────────────────────────────────────────┤
│ OP_CONSTANT         │ idx    │ [...] → [..., constants[idx]]            │
│ OP_NIL              │ -      │ [...] → [..., nil]                       │
│ OP_TRUE             │ -      │ [...] → [..., true]                      │
│ OP_FALSE            │ -      │ [...] → [..., false]                     │
│ OP_POP              │ -      │ [..., a] → [...]                         │
│ OP_GET_LOCAL        │ slot   │ [...] → [..., stack[slot]]               │
│ OP_SET_LOCAL        │ slot   │ [..., a] → [..., a] (stack[slot]=a)      │
│ OP_GET_GLOBAL       │ idx    │ [...] → [..., globals[name]]             │
│ OP_DEFINE_GLOBAL    │ idx    │ [..., a] → [...] (globals[name]=a)       │
│ OP_SET_GLOBAL       │ idx    │ [..., a] → [..., a] (globals[name]=a)    │
│ OP_GET_UPVALUE      │ idx    │ [...] → [..., upvalues[idx]]             │
│ OP_SET_UPVALUE      │ idx    │ [..., a] → [..., a] (upvalues[idx]=a)    │
│ OP_GET_PROPERTY     │ idx    │ [..., obj] → [..., obj.prop]             │
│ OP_SET_PROPERTY     │ idx    │ [..., obj, val] → [..., val]             │
│ OP_GET_SUPER        │ idx    │ [..., obj] → [..., bound_method]         │
│ OP_EQUAL            │ -      │ [..., a, b] → [..., a==b]                │
│ OP_GREATER          │ -      │ [..., a, b] → [..., a>b]                 │
│ OP_LESS             │ -      │ [..., a, b] → [..., a<b]                 │
│ OP_ADD              │ -      │ [..., a, b] → [..., a+b]                 │
│ OP_SUBTRACT         │ -      │ [..., a, b] → [..., a-b]                 │
│ OP_MULTIPLY         │ -      │ [..., a, b] → [..., a*b]                 │
│ OP_DIVIDE           │ -      │ [..., a, b] → [..., a/b]                 │
│ OP_NOT              │ -      │ [..., a] → [..., !a]                     │
│ OP_NEGATE           │ -      │ [..., a] → [..., -a]                     │
│ OP_PRINT            │ -      │ [..., a] → [...] (print a)               │
│ OP_JUMP             │ off    │ ip += offset                             │
│ OP_JUMP_IF_FALSE    │ off    │ if falsey(peek) ip += offset             │
│ OP_LOOP             │ off    │ ip -= offset                             │
│ OP_CALL             │ argc   │ [..., fn, args...] → [..., result]       │
│ OP_INVOKE           │ idx,n  │ [..., obj, args...] → [..., result]      │
│ OP_SUPER_INVOKE     │ idx,n  │ [..., obj, args...] → [..., result]      │
│ OP_CLOSURE          │ idx,.. │ [...] → [..., closure]                   │
│ OP_CLOSE_UPVALUE    │ -      │ close topmost open upvalue               │
│ OP_RETURN           │ -      │ [..., result] → (pop frame, push result) │
│ OP_CLASS            │ idx    │ [...] → [..., class]                     │
│ OP_INHERIT          │ -      │ [..., super, sub] → [..., sub]           │
│ OP_METHOD           │ idx    │ [..., class, closure] → [..., class]     │
│ OP_IMPORT           │ -      │ [..., path] → [...] (define module)      │
│ OP_IMPORT_FROM      │ -      │ [..., path, name] → [...] (define name)  │
│ OP_IMPORT_ALIAS     │ -      │ [..., path, name, alias] → [...]         │
└─────────────────────┴────────┴──────────────────────────────────────────┘
```

---

## 7. Garbage Collector

### 7.1 Algorithm: Tri-color Mark-and-Sweep

```
Phase 1: Mark Roots (white → gray)
  遍历所有 GC roots，标记为 gray，加入 gray_stack

Phase 2: Trace References (gray → black)
  while gray_stack 非空:
    取出一个 gray 对象
    遍历其所有引用的对象
    将未标记的引用对象标记为 gray
    当前对象标记为 black (is_marked = true)

Phase 3: Sweep (回收 white)
  遍历 objects_ 链表:
    if is_marked: 重置标记 (black → white)
    else: 从链表移除，释放内存
```

### 7.2 GC Roots

| Root | 位置 | 描述 |
|------|------|------|
| Stack | `VM::stack_[0..stack_top_]` | 所有栈上的 Value |
| Globals | `VM::globals_` | 全局变量表 |
| Frames | `VM::frames_[0..frame_count_]` | 调用帧中的 closure |
| Open Upvalues | `VM::open_upvalues_` | 打开的 upvalue 链 |
| Compiler | `Compiler::function_` | 编译中的函数对象 |
| Init String | `VM::init_string_` | 缓存的 "init" 字符串 |

### 7.3 Object Tracing Rules

| 对象类型 | 需要追踪的引用 |
|---------|---------------|
| ObjString | （无引用） |
| ObjFunction | name (ObjString*), chunk.constants[] |
| ObjNative | （无引用） |
| ObjClosure | function, upvalues[] |
| ObjUpvalue | closed value |
| ObjClass | name, methods table |
| ObjInstance | klass, fields table |
| ObjBoundMethod | receiver, method |
| ObjModule | name, exports table |

### 7.4 GC Trigger & Tuning

```cpp
// 对象分配时检查
template <typename T, typename... Args>
T* allocate_object(Args&&... args) {
  bytes_allocated_ += sizeof(T);
  if (bytes_allocated_ > next_gc_)
    collect_garbage();

  auto* obj = new T(std::forward<Args>(args)...);
  obj->next = objects_;
  objects_ = obj;
  return obj;
}

// GC 后调整阈值
next_gc_ = bytes_allocated_ * kGC_HEAP_GROW;  // kGC_HEAP_GROW = 2
```

### 7.5 String Table Cleanup

Sweep 阶段之前，清理字符串驻留表中指向已回收字符串的条目：

```cpp
void Table::remove_white() {
  for (auto& entry : entries_) {
    if (entry.key != nullptr && !entry.key->is_marked) {
      remove(entry.key);  // 转为 tombstone
    }
  }
}
```

---

## 8. Import System Design

### 8.1 Compilation

```
import "math.ms";
→ OP_CONSTANT  "math.ms"     // 模块路径
→ OP_IMPORT                  // 加载模块，定义全局 "math"

from "math.ms" import add;
→ OP_CONSTANT  "math.ms"     // 模块路径
→ OP_CONSTANT  "add"         // 导出名
→ OP_IMPORT_FROM             // 提取并定义全局 "add"

from "math.ms" import add as plus;
→ OP_CONSTANT  "math.ms"     // 模块路径
→ OP_CONSTANT  "add"         // 导出名
→ OP_CONSTANT  "plus"        // 别名
→ OP_IMPORT_ALIAS            // 提取并定义全局 "plus"
```

### 8.2 Module Loading Flow

```
VM::import_module(path)
  │
  ├── 检查 modules_ 缓存 → 命中则返回
  │
  ├── 标记 path 为 "loading"（循环检测）
  │
  ├── ModuleLoader::read_source(path)
  │     └── std::ifstream 读取文件
  │
  ├── Compiler::compile(source)
  │     └── 返回 ObjFunction*
  │
  ├── 创建新 CallFrame 执行模块代码
  │     └── 模块有独立的 globals scope
  │
  ├── 收集模块 globals 作为 exports
  │
  ├── 创建 ObjModule，存入 modules_ 缓存
  │
  └── 返回 ObjModule*
```

### 8.3 Path Resolution

```
resolve_path("math.ms", "/project/src/main.ms")
→ "/project/src/math.ms"

resolve_path("lib/utils.ms", "/project/src/main.ms")
→ "/project/src/lib/utils.ms"
```

- 路径相对于**导入文件所在目录**解析
- 使用 `std::filesystem::path` 处理跨平台路径

---

## 9. Logger Design

### 9.1 Architecture

```cpp
class Logger : public Singleton<Logger> {
  LogLevel level_ = LogLevel::INFO;

  void log_impl(LogLevel level, strv_t tag, strv_t message) {
    if (level < level_) return;

    auto color = level_color(level);
    std::cerr << color << "[" << level_name(level) << "]"
              << reset_colorful << " [" << tag << "] "
              << message << std::endl;
  }
};
```

### 9.2 Color Mapping

| Level | Color | ANSI Code | Win32 Attribute |
|-------|-------|-----------|-----------------|
| TRACE | Gray | `\033[90m` | `FOREGROUND_INTENSITY` |
| DEBUG | Cyan | `\033[36m` | `FOREGROUND_GREEN \| FOREGROUND_BLUE` |
| INFO | Green | `\033[32m` | `FOREGROUND_GREEN` |
| WARN | Yellow | `\033[33m` | `FOREGROUND_RED \| FOREGROUND_GREEN` |
| ERROR | Red | `\033[31m` | `FOREGROUND_RED` |

### 9.3 Usage

```cpp
auto& log = Logger::get_instance();
log.set_level(LogLevel::DEBUG);
log.debug("GC", "collecting garbage, {} bytes allocated", bytes_allocated_);
log.info("VM", "executing {}", function->name->value);
log.error("Compiler", "undefined variable '{}'", name);
```

---

## 10. Closures & Upvalues

### 10.1 Upvalue Lifecycle

```
Open State (变量在栈上):
  ┌──────────┐
  │ ObjUpvalue│
  │ location ─┼──→ stack[slot]   (指向栈上的 Value)
  │ closed    │    (unused)
  │ next     ─┼──→ (next open upvalue)
  └──────────┘

Close Transition (变量离开栈):
  upvalue->closed = *upvalue->location;
  upvalue->location = &upvalue->closed;

Closed State (变量在堆上):
  ┌──────────┐
  │ ObjUpvalue│
  │ location ─┼──→ closed (指向自身的 closed 字段)
  │ closed    │    = captured value
  │ next      │    (nullptr)
  └──────────┘
```

### 10.2 Sharing Upvalues

```maple
fun outer() {
  var x = 10;
  fun a() { print x; }  // captures x
  fun b() { x = 20; }   // captures same x
  b();
  a();  // prints 20, because a and b share the same upvalue
}
```

VM 维护 `open_upvalues_` 链表（按栈位置降序排列），确保同一栈槽位只有一个 ObjUpvalue。

---

## 11. Cross-Platform Considerations

### 11.1 Colorful Output

| 平台 | 实现方式 |
|------|---------|
| POSIX (Linux/macOS) | ANSI 转义序列 (`\033[Nm`) |
| Win32 (MSVC) | `SetConsoleTextAttribute()` API |

`ColorfulHelper.hh` 根据 `MAPLE_MSVC` / `MAPLE_GNUC` 选择平台实现。

### 11.2 `std::format` Availability

| 编译器 | 最低版本 |
|--------|---------|
| MSVC | 19.29 (VS 2019 16.10+) |
| GCC | 13+ |

若 `std::format` 不可用，降级到 `std::stringstream` + `as_string()` 工具函数。

### 11.3 File System

| 操作 | 方案 |
|------|------|
| 路径分隔符 | `std::filesystem::path` 自动处理 |
| 文件读取 | `std::ifstream` |
| 当前目录 | `std::filesystem::current_path()` |

### 11.4 ssize_t

```cpp
#if defined(MAPLE_GNUC)
  using ssz_t = ssize_t;
#else
  using ssz_t = SSIZE_T;  // <basetsd.h>
#endif
```

---

## 12. CMake Build Configuration

```cmake
cmake_minimum_required(VERSION 3.22)
project(mslang CXX)

# C++23 标准
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W3 /EHsc")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2b -Wall -Werror")
endif()

# 调试选项
option(MAPLE_DEBUG_TRACE "Trace VM execution" OFF)
option(MAPLE_DEBUG_PRINT "Print compiled bytecode" OFF)
option(MAPLE_DEBUG_STRESS_GC "Stress test GC" OFF)
option(MAPLE_DEBUG_LOG_GC "Log GC activity" OFF)

# Sources
file(GLOB_RECURSE SOURCES src/*.hh src/*.cc)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)

# CTest
include(CTest)
file(GLOB TEST_SCRIPTS tests/*.ms)
foreach(script ${TEST_SCRIPTS})
  get_filename_component(name ${script} NAME_WE)
  add_test(NAME "test_${name}" COMMAND ${PROJECT_NAME} ${script})
endforeach()
```

---

## 13. Testing Strategy

### 13.1 Expect-Based Testing

每个 `.ms` 测试文件使用注释标记期望输出：

```maple
print 1 + 2;       // expect: 3
print "hello";     // expect: hello
```

### 13.2 Test Runner (`tests/run_tests.py`)

```python
# 伪代码
for test_file in glob("tests/*.ms"):
    expected = parse_expect_comments(test_file)
    actual = run_process(["./mslang", test_file])
    assert actual == expected
```

### 13.3 Test Matrix

| Phase | 测试文件 | 验证内容 |
|-------|---------|---------|
| 3 | arithmetic.ms | 四则运算、优先级、取负 |
| 3 | variables.ms | 全局/局部变量、作用域、赋值 |
| 3 | strings.ms | 字符串拼接、字面量 |
| 3 | control_flow.ms | if/else, while, for |
| 4 | functions.ms | 函数定义、调用、递归、原生函数 |
| 4 | closures.ms | 闭包捕获、upvalue sharing |
| 5 | classes.ms | 类、实例、方法、继承、super |
| 6 | (all above) | GC 启用下全部通过 |
| 7 | import_test.ms | import, from-import-as |
| 8 | errors.ms | 编译/运行时错误信息 |
