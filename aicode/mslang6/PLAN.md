# Maple 脚本语言 — 实现架构与设计方案

## 1. 项目概述

**Maple** 是一门参考 [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) 中 **clox**（C 实现）设计的脚本语言。在保留 clox 的字节码虚拟机、垃圾回收、闭包与 OOP 等核心能力的基础上，使用 **C++23** 以多文件、跨平台方式实现，并扩展 **import** 机制与内部彩色 **Logger**。

- **语言名称**：Maple  
- **命名空间**：`ms`  
- **参考实现**：clox（非 jlox）  
- **构建**：CMake，支持 Windows（MSVC）与 Linux（GCC）

---

## 2. 需求与约束

### 2.1 功能需求

| 类别 | 内容 |
|------|------|
| **clox 已有能力** | 词法/语法、字节码编译、VM 执行、值类型（nil/bool/number/string）、全局/局部变量、闭包与 upvalue、类与实例、继承与 super、方法调用与 `this`、原生函数、GC（标记-清扫） |
| **扩展能力** | **import**：`import mod`、`from mod import a, b`、`from mod import x as y` |
| **工具与调试** | 内部 Logger：多级别（如 DEBUG/INFO/WARN/ERROR），按级别输出不同颜色 |

### 2.2 非功能约束

- **C++ 标准**：C++23 及以上  
- **文件规范**：头文件 `.hh`，实现 `.cc`；头文件使用 `#pragma once`  
- **代码组织**：多文件实现，避免单文件集中  
- **依赖**：优先标准库，尽量少用第三方库  
- **平台**：Windows（MSVC）、Linux（GCC）  
- **测试**：在单独测试目录中提供可执行的 Maple 测试脚本及说明  

---

## 3. 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                         Maple 运行时                              │
├─────────────┬─────────────┬─────────────┬─────────────┬─────────┤
│   Scanner   │  Compiler   │    Chunk    │     VM      │   GC    │
│  (词法分析)  │  (语法→字节码) │  (字节码+常量) │ (执行+调用栈) │ (标记清扫) │
├─────────────┴─────────────┴─────────────┴─────────────┴─────────┤
│  Value / Object (String, Function, Closure, Class, Instance, …)  │
│  Table (哈希表)  │  Memory (统一分配 + GC 触发)                     │
├─────────────────────────────────────────────────────────────────┤
│  Logger (级别 + 颜色)  │  Module Loader (import / from...import)   │
└─────────────────────────────────────────────────────────────────┘
```

- **前端**：Scanner → Compiler → Chunk（单遍编译，无显式 AST 持久化）。  
- **后端**：VM 读 Chunk 执行；所有堆对象经 Memory 分配并参与 GC。  
- **扩展**：Module Loader 在编译/运行期解析 `import` / `from ... import ... as ...`，并注入全局或局部绑定；Logger 仅在宿主（C++）侧使用，不暴露到 Maple 脚本。

---

## 4. 目录与模块划分

建议的源码与测试目录结构如下（仅规划，不写具体代码）：

```
mslang6/
├── CMakeLists.txt              # 根 CMake，C++23，MSVC/GCC 兼容
├── PLAN.md                     # 本设计文档
├── README.md                   # 项目说明与构建/运行指引
├── include/                    # 或 src 下按模块分子目录，此处统一列出
│   └── ms/
│       ├── common.hh           # 公共常量、UINT8_COUNT、调试开关等
│       ├── chunk.hh / chunk.cc  # 字节码 Chunk + 常量表
│       ├── value.hh / value.cc # Value 类型、ValueArray、打印与相等
│       ├── memory.hh / memory.cc # 统一 reallocate、GC 入口、对象链表
│       ├── table.hh / table.cc # 哈希表（key: ObjString*, value: Value）
│       ├── object.hh / object.cc # Obj 基类及 String/Function/Closure/Class/Instance/Upvalue/BoundMethod/Native
│       ├── scanner.hh / scanner.cc # 词法分析，Token 与 Scanner 状态
│       ├── compiler.hh / compiler.cc # 递归下降编译，生成 Chunk
│       ├── vm.hh / vm.cc       # VM 状态、栈、CallFrame、解释循环、内置 native
│       ├── debug.hh / debug.cc # 反汇编等（可选与 Logger 结合）
│       ├── logger.hh / logger.cc # 多级别 + 颜色 Logger
│       ├── module.hh / module.cc # 模块解析、缓存、import/from-import 语义
│       └── maple.hh            # 对外入口：interpret(source)、run_file 等
├── src/
│   └── main.cc                 # 入口：REPL / 文件模式，参数解析
└── tests/
    ├── README.md               # 如何运行测试、测试脚本说明
    ├── run_tests.*              # 运行脚本（如 .bat / .sh）或说明
    ├── *.ms                    # Maple 脚本测试用例
    │   ├── expr_basic.ms
    │   ├── control_flow.ms
    │   ├── functions_closures.ms
    │   ├── classes_inheritance.ms
    │   └── import_*.ms          # import / from...import 专项
    └── fixtures/                # 可选：被 import 的示例模块
        └── lib/
            └── *.ms
```

- **命名空间**：所有公开类型与函数置于 `namespace ms`。  
- **头文件**：一律 `#pragma once`，避免 `#ifndef`/`#define`/`#endif`。  
- **实现**：按模块拆分为 `.cc`，便于维护与增量编译。

---

## 5. 核心模块设计

### 5.1 公共与常量 (common)

- 定义 `UINT8_COUNT`、`FRAMES_MAX`、`STACK_MAX` 等常量。  
- 可选：编译期调试开关（如 `DEBUG_TRACE_EXECUTION`、`DEBUG_PRINT_CODE`、`DEBUG_LOG_GC`）通过宏或 `constexpr` 控制，便于与 Logger 配合。  
- 与 clox 的 `common.h` 对应，但用 C++ 类型（如 `std::size_t`、`bool`）。

### 5.2 值类型 (value)

- **Value**：可选用类似 clox 的 NaN-boxing（单一 `uint64_t`）或 tagged union（`std::variant` 或手写 union + type tag）。  
  - 需支持：nil、bool、number（double）、obj（指向 Obj 的指针）。  
- **ValueArray**：动态数组，用于 Chunk 常量表；可用 `std::vector<Value>` 或自管数组。  
- 提供 `valuesEqual`、`printValue`，供 VM 与调试使用。

### 5.3 字节码块 (chunk)

- **Chunk**：`code`（`uint8_t` 序列）、`lines`（行号信息）、`constants`（ValueArray）。  
- 操作：`writeChunk(byte, line)`、`addConstant(value)`、`initChunk`/`freeChunk`。  
- **OpCode** 枚举与 clox 对齐（OP_CONSTANT, OP_NIL, OP_TRUE, OP_FALSE, OP_GET_LOCAL, OP_SET_LOCAL, OP_GET_GLOBAL, OP_DEFINE_GLOBAL, OP_SET_GLOBAL, OP_GET_UPVALUE, OP_SET_UPVALUE, OP_GET_PROPERTY, OP_SET_PROPERTY, OP_GET_SUPER, OP_EQUAL, OP_GREATER, OP_LESS, OP_ADD~OP_DIVIDE, OP_NOT, OP_NEGATE, OP_PRINT, OP_JUMP, OP_JUMP_IF_FALSE, OP_LOOP, OP_CALL, OP_INVOKE, OP_SUPER_INVOKE, OP_CLOSURE, OP_CLOSE_UPVALUE, OP_RETURN, OP_CLASS, OP_INHERIT, OP_METHOD）。  
- 若实现 import，可后续增加 OP_LOAD_MODULE / OP_GET_MODULE 等（见 5.9）。

### 5.4 内存与垃圾回收 (memory)

- **reallocate**：统一入口，替代直接 malloc/free/realloc；用于 Chunk、Table、所有 Obj 等。  
- **对象链表**：所有堆对象挂到全局链表，GC 时遍历。  
- **GC 算法**：标记-清扫（Mark-Sweep）。  
  - **Mark**：从根集合（VM 栈、当前调用帧中的 closure、open upvalues、globals、编译器根、initString 等）出发，递归/迭代标记所有可达对象。  
  - **Sweep**：遍历对象链表，未标记者释放并从链表移除。  
- **触发条件**：分配时若 `bytesAllocated > nextGC` 则触发 GC；`nextGC` 按堆增长策略更新（如 2 倍）。  
- 与 clox 的 `memory.c` 对应；在 C++ 中可用类封装分配器与 GC 状态，便于与 Logger 结合（如 DEBUG_LOG_GC 时打日志）。

### 5.5 哈希表 (table)

- Key：`ObjString*`；Value：`Value`。  
- 支持 `get`、`set`、`delete`、`addAll`（用于继承时复制父类方法）。  
- 支持 **GC**：`markTable`、`tableRemoveWhite`（清扫阶段移除白节点）。  
- 实现：开放寻址或链式哈希均可，与 clox 一致。

### 5.6 对象系统 (object)

- **Obj** 基类：`type`、`isMarked`、`next`（链表）。  
- 派生类型：**ObjString**（length, chars, hash）、**ObjFunction**（arity, upvalueCount, chunk, name）、**ObjClosure**（function, upvalues[]）、**ObjUpvalue**（location/closed, next）、**ObjClass**（name, methods Table）、**ObjInstance**（klass, fields Table）、**ObjBoundMethod**（receiver, method）、**ObjNative**（function 指针）。  
- 工厂函数：`copyString`、`takeString`、`newFunction`、`newClosure`、`newUpvalue`、`newClass`、`newInstance`、`newBoundMethod`、`newNative`。  
- 所有分配经 `memory` 的 `reallocate`，并挂到对象链表；GC 的 `blacken` 按类型遍历引用。

### 5.7 词法分析 (scanner)

- **Token**：type、start、length、line。  
- **Scanner**：持有一段源码与当前偏移，产出 Token 流。  
- Token 类型与 clox 一致（含关键字、标识符、字面量、运算符、括号等）。  
- 可增加 **TOKEN_IMPORT**、**TOKEN_FROM**、**TOKEN_AS** 等以支持 import 语法。

### 5.8 编译器 (compiler)

- **递归下降**：按优先级解析表达式与语句，直接生成字节码（无独立 AST 结构）。  
- **Compiler** 状态：enclosing、function、locals[]、upvalues[]、scopeDepth、type（SCRIPT/FUNCTION/METHOD/INITIALIZER）。  
- **ClassCompiler**：处理 class 与继承、super。  
- 语句/声明：print、表达式语句、var、block、if/while/for、fun、return、class、import/from-import（见 5.9）。  
- 与 GC 协作：编译器根在 `markRoots` 中标记（若有编译器全局状态）。

### 5.9 模块与 import (module)

- **语义目标**：  
  - `import foo`：加载模块 `foo`，将模块对象（或模块的命名空间 Table）以名 `foo` 放入当前作用域。  
  - `from foo import a, b`：加载 `foo`，将 `foo.a`、`foo.b` 绑定到当前作用域的 `a`、`b`。  
  - `from foo import x as y`：加载 `foo`，将 `foo.x` 绑定到当前作用域的 `y`。  
- **实现要点**：  
  - **模块解析**：给定模块名（如 `foo`），映射到文件路径（如 `foo.ms` 或 `foo/init.ms`），规则可配置（当前目录、脚本所在目录、LIB 路径等）。  
  - **模块缓存**：同一模块名在同一 VM 生命周期内只编译/执行一次，结果缓存在 VM 或独立 ModuleCache 中。  
  - **编译期**：遇到 `import`/`from ... import` 时，可生成 OP_LOAD_MODULE + 常量（模块名），再 OP_GET_MODULE 或 OP_DEFINE_GLOBAL；或由 VM 在运行时加载并注入全局表。  
  - **作用域**：import 的绑定写入当前 Compiler 的全局槽位（或顶层 local），与 var 一致。  
- **文件组织**：`module.hh`/`module.cc` 提供模块查找、缓存、加载接口；VM 或 Compiler 调用这些接口完成语义。

### 5.10 虚拟机 (vm)

- **VM 状态**：栈（Value[]）、栈顶指针、CallFrame 数组（closure、ip、slots）、当前 frame、globals Table、strings 常量表、objects 链表、openUpvalues、bytesAllocated、nextGC、grayStack（GC）、initString 等。  
- **解释循环**：取指、解码、执行；处理所有 OpCode（与 5.3 一致）。  
- **调用约定**：栈上 [ receiver(若为方法) ] [ arg1..argN ] [ callee ]；CallFrame 记录 closure、ip、slots 基址。  
- **原生函数**：如 `clock()`；通过 `defineNative` 注册到 globals。  
- **错误处理**：运行时错误（类型错误、未定义变量等）通过 `runtimeError` 报告并返回 INTERPRET_RUNTIME_ERROR；编译错误由 Compiler 设置 hadError 并返回 INTERPRET_COMPILE_ERROR。

### 5.11 调试与反汇编 (debug)

- 反汇编：将 Chunk 的字节码按 OP_* 打印为可读指令序列（含常量、跳转偏移等）。  
- 可选：与 Logger 结合，在 DEBUG_TRACE_EXECUTION 时按日志级别输出，便于排查。

### 5.12 日志 (logger)

- **级别**：至少 DEBUG、INFO、WARN、ERROR（可再细分）。  
- **颜色**：按级别映射到不同 ANSI 颜色（如 DEBUG=灰、INFO=绿、WARN=黄、ERROR=红）；Windows 控制台需在运行时启用 ANSI 或使用 Win32 API 设置属性（C++23 下可用 `<print>` 或手动写序列）。  
- **接口**：如 `Logger::debug(msg)`、`info`、`warn`、`error`；支持格式化（可基于 `std::format` C++20）。  
- **用途**：编译/VM/GC 内部调试用，不暴露给 Maple 脚本；可选通过环境变量或启动参数控制级别与是否着色。

---

## 6. 构建与平台

- **CMake**：  
  - 设置 `CMAKE_CXX_STANDARD 23`（或更高）。  
  - 对 MSVC：启用 UTF-8、适当警告；对 GCC：`-std=c++23`、`-Wall -Wextra`。  
  - 可选项：`DEBUG_TRACE_EXECUTION`、`DEBUG_PRINT_CODE`、`DEBUG_LOG_GC` 通过 `add_compile_definitions` 或 option 控制。  
- **输出**：可执行文件 `maple`（或 `maple.exe`），接受文件路径或无参数时进入 REPL。  
- **依赖**：仅标准库 + 平台 API（如 Windows 控制台颜色）；不引入第三方解析器或 VM 库。

---

## 7. 测试策略

- **tests/** 下放置 `.ms` 脚本，覆盖：  
  - 表达式与类型（算术、比较、逻辑、字符串拼接）；  
  - 控制流（if/else、while、for）；  
  - 函数与闭包；  
  - 类、继承、super、方法；  
  - import / from...import...as... 及模块缓存、循环依赖边界情况。  
- **tests/README.md**：说明如何用 `maple tests/xxx.ms` 运行测试、期望输出或退出码。  
- 可选：用脚本（如 `run_tests.bat` / `run_tests.sh`）批量执行并比对输出，便于 CI。

---

## 8. 实现任务清单（可执行、可测试、可验证）

以下任务按依赖顺序排列，**每个任务**均满足：**可单独执行**、**有明确验收方式**（构建通过 / 运行输出符合预期 / 指定测试通过）。完成一项并验证后再进行下一项。**实现并验证通过后，须在本节该任务的「状态」列中更新为已完成**（如 `✅ 已完成`），参见 [AGENTS.md](AGENTS.md)。

### 8.1 阶段 0：项目骨架

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T0.1** | 创建 CMakeLists.txt（C++23、MSVC/GCC）、src/main.cc 入口 | `cmake -B build && cmake --build build` 成功；运行 `maple` 或 `maple --help` 有预期输出或退出码 0 | 待办 |

### 8.2 阶段 1：基础设施

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T1.1** | common.hh：UINT8_COUNT、FRAMES_MAX、STACK_MAX、调试开关（可选） | 被至少一个 .cc 包含，构建通过 | 待办 |
| **T1.2** | value.hh/cc：Value（nil/bool/number/obj 前向声明）、ValueArray、printValue、valuesEqual | 单元测试或 main 中构造 Value 并 printValue，输出符合预期 | 待办 |
| **T1.3** | chunk.hh/cc：Chunk、writeChunk(byte,line)、addConstant、initChunk/freeChunk | main 或测试中写入字节与常量，可断言 chunk 内容正确 | 待办 |
| **T1.4** | memory.hh/cc：reallocate 统一入口，暂不实现 GC | 所有分配经 reallocate；构建通过；可选简单分配/释放测试 | 待办 |
| **T1.5** | object.hh/cc：Obj、ObjString（length/chars/hash）、copyString、takeString，对象链表 | 创建字符串并 printValue(OBJ_VAL(s)) 输出正确 | 待办 |
| **T1.6** | table.hh/cc：Table、get/set/delete、init/free；markTable/tableRemoveWhite 占位 | 插入/查找 ObjString* key，Value 正确 | 待办 |
| **T1.7** | logger.hh/cc：DEBUG/INFO/WARN/ERROR 级别，按级别着色（ANSI/Win32） | main 中按级别打印，控制台可见级别与颜色（非 TTY 可降级为无颜色） | 待办 |

### 8.3 阶段 2：词法与编译（最小前端）

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T2.1** | scanner.hh/cc：Token、Scanner、initScanner、scanToken，完整 Token 类型 | 对给定源码扫描，得到预期 token 序列（可 C++ 测试或 tests/scanner_manual.ms 说明） | 待办 |
| **T2.2** | object 扩展：ObjFunction（chunk、arity、name）、newFunction | 编译器可创建 ObjFunction 并写入其 chunk；构建通过 | 待办 |
| **T2.3** | compiler.hh/cc：递归下降，仅表达式 + print 语句，无变量无函数 | 编译 `print 1+2;` 得到 ObjFunction，chunk 含 OP_CONSTANT/OP_ADD/OP_PRINT/OP_RETURN；可选反汇编肉眼验证 | 待办 |

### 8.4 阶段 3：虚拟机（最小可运行）

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T3.1** | vm.hh/cc：栈、单 CallFrame、interpret(ObjFunction*)；OP_CONSTANT、OP_ADD、OP_PRINT、OP_RETURN | main：compile("print 1+2;") + interpret(function)，stdout 输出 `3` | 待办 |
| **T3.2** | main.cc：支持 `maple script.ms` 执行文件、无参数时 REPL 或提示 | 运行 `maple tests/print_expr.ms` 得到预期输出；或 REPL 输入 `print 1+2;` 输出 `3` | 待办 |

### 8.5 阶段 4：变量与控制流

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T4.1** | 编译器 + VM：全局变量（OP_GET_GLOBAL、OP_DEFINE_GLOBAL、OP_SET_GLOBAL） | tests/expr_global.ms 定义并打印全局变量，通过 | 待办 |
| **T4.2** | 编译器 + VM：局部变量（OP_GET_LOCAL、OP_SET_LOCAL）、块与作用域 | tests/expr_local.ms 通过 | 待办 |
| **T4.3** | 编译器 + VM：if/else、while、for、逻辑 and/or | tests/control_flow.ms 通过 | 待办 |

### 8.6 阶段 5：函数与闭包

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T5.1** | object：ObjClosure、ObjUpvalue；VM：CallFrame 用 closure、OP_CALL、OP_RETURN、OP_GET_UPVALUE/OP_SET_UPVALUE、OP_CLOSURE、OP_CLOSE_UPVALUE | tests/functions_closures.ms（函数调用、返回值、闭包）通过 | 待办 |
| **T5.2** | VM：defineNative；内置 clock() 等 | 测试脚本中调用 clock() 有合理输出 | 待办 |

### 8.7 阶段 6：垃圾回收

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T6.1** | memory：mark-sweep（markRoots、traceReferences、sweep、gray stack）；VM/compiler 根注册 | 分配超过阈值触发 GC；无泄漏；可选 DEBUG_LOG_GC 观察日志 | 待办 |
| **T6.2** | table：markTable、tableRemoveWhite 与 GC 集成 | 含全局表/字符串的脚本多次运行触发 GC，无崩溃、无误回收 | 待办 |

### 8.8 阶段 7：类与 OOP

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T7.1** | object：ObjClass、ObjInstance、ObjBoundMethod；VM：OP_CLASS、OP_GET_PROPERTY、OP_SET_PROPERTY、OP_INVOKE、OP_GET_SUPER、OP_SUPER_INVOKE、OP_METHOD、OP_INHERIT；init、this、super | tests/classes_inheritance.ms 通过 | 待办 |

### 8.9 阶段 8：import

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T8.1** | module.hh/cc：模块路径解析（如 foo → foo.ms）、缓存、加载（编译+执行）返回模块命名空间（Table 或等价） | C++ 测试或内部验证：加载同一模块两次命中缓存 | 待办 |
| **T8.2** | compiler：`import mod`、`from mod import a,b`、`from mod import x as y`；VM 侧加载并注入当前作用域 | tests/import_basic.ms、import_from_as.ms 通过 | 待办 |

### 8.10 阶段 9：调试与测试完善

| 任务 ID | 内容 | 验收方式 | 状态 |
|---------|------|----------|------|
| **T9.1** | debug.hh/cc：反汇编 Chunk（含常量、跳转偏移） | 启用 DEBUG_PRINT_CODE 时编译脚本后输出可读字节码 | 待办 |
| **T9.2** | tests/README.md、run_tests 脚本（可选）、所有 .ms 用例与期望输出文档化 | 按 tests/README.md 执行全部测试脚本，输出与文档一致 | 待办 |

---

**任务依赖简图**：T0.1 → T1.1～T1.7（可并行部分内部按表顺序）→ T2.1→T2.2→T2.3 → T3.1→T3.2 → T4.1→T4.2→T4.3 → T5.1→T5.2 → T6.1→T6.2 → T7.1 → T8.1→T8.2 → T9.1→T9.2。

---

## 9. 与 clox 的对应关系（速查）

| clox 文件/概念 | Maple 对应 |
|----------------|------------|
| common.h       | common.hh |
| value.h/c      | value.hh/cc |
| chunk.h/c      | chunk.hh/cc |
| memory.h/c     | memory.hh/cc |
| table.h/c      | table.hh/cc |
| object.h/c     | object.hh/cc |
| scanner.h/c    | scanner.hh/cc |
| compiler.h/c   | compiler.hh/cc |
| vm.h/c         | vm.hh/cc |
| debug.h/c      | debug.hh/cc |
| main.c         | main.cc + maple.hh |
| (无)           | logger.hh/cc, module.hh/cc |

---

## 10. 文档与交付物

- **PLAN.md**（本文）：架构与设计方案，不包含具体代码实现。  
- **README.md**：项目简介、构建命令、运行方式、测试说明。  
- **tests/README.md**：测试脚本列表、运行方法、期望行为简述。  
- 代码实现阶段再按上述模块逐步落地，并保持与本文设计一致。

以上为 Maple 脚本语言的实现架构与设计方案，从语言实现角度覆盖了 clox 核心与 import、Logger 扩展，并满足 C++23、多文件、跨平台与测试要求。
