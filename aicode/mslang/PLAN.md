# PLAN.md — Maple Language Implementation Task Plan

> 按 Phase 分阶段、按任务分粒度的实现计划。每个任务标注依赖关系和验证标准。

---

## Phase 1: Foundation — 基础设施搭建

### Task 1.1: CMake 构建系统
- **Files**: `CMakeLists.txt`
- **Depends**: (none)
- **Work**:
  - cmake_minimum_required 3.22, project(mslang CXX)
  - MSVC: `/std:c++latest /W3 /EHsc /GF /GS /Gs /Zi`
  - GCC: `-std=c++2b -Wall -Werror -march=native`
  - Clang: 额外 `-stdlib=libc++`
  - Debug/Release 配置
  - GLOB_RECURSE 收集 `src/*.hh` `src/*.cc`
  - MSVC source_group_by_dir 宏
  - `option()` 调试开关: MAPLE_DEBUG_TRACE, MAPLE_DEBUG_PRINT, MAPLE_DEBUG_STRESS_GC, MAPLE_DEBUG_LOG_GC
  - CTest 集成（遍历 `tests/*.ms`）
- **Verify**: `cmake -B build && cmake --build build` 编译空 main.cc 通过

### Task 1.2: 平台基础头文件
- **Files**: `src/Macros.hh`, `src/Types.hh`, `src/Consts.hh`, `src/Common.hh`
- **Depends**: Task 1.1
- **Work**:
  - `Macros.hh` — MAPLE_GNUC / MAPLE_MSVC 检测, MAPLE_UNUSED 宏
  - `Types.hh` — 类型别名 (u8_t, str_t, sz_t, ssz_t, ...)
  - `Consts.hh` — kSTACK_MAX=256, kFRAMES_MAX=64, kUINT8_COUNT=256, kGC_HEAP_GROW=2, kGC_INITIAL_SIZE=1MB, kALIGNMENT=8
  - `Common.hh` — Copyable, UnCopyable, Singleton<T>, as_type<T>, as_down<T>, as_ptr<T>, as_string()
- **Verify**: main.cc 包含所有头文件编译通过

### Task 1.3: 颜色输出系统
- **Files**: `src/Colorful.hh`, `src/Colorful.cc`, `src/ColorfulHelper.hh`, `src/ColorfulHelper_POSIX.hh`, `src/ColorfulHelper_WIN32.hh`
- **Depends**: Task 1.2
- **Work**:
  - `Colorful.hh` — Color 枚举, fg::red/green/... 和 bg::red/green/... stream manipulators, reset_colorful()
  - `ColorfulHelper_POSIX.hh` — ANSI 转义码 (`\033[Nm`) 实现 set_foreground_colorful / set_background_colorful
  - `ColorfulHelper_WIN32.hh` — SetConsoleTextAttribute 实现
  - `ColorfulHelper.hh` — 根据 MAPLE_MSVC/MAPLE_GNUC 分发 include
  - `Colorful.cc` — set_colorful 实现
- **Verify**: main.cc 输出彩色文字到终端

### Task 1.4: Logger 日志系统
- **Files**: `src/Logger.hh`, `src/Logger.cc`
- **Depends**: Task 1.3
- **Work**:
  - LogLevel 枚举: TRACE, DEBUG, INFO, WARN, ERROR
  - Logger : Singleton<Logger>
  - set_level() / level()
  - trace/debug/info/warn/error 方法 (tag + message)
  - 模板变参版本支持 std::format 格式化
  - 颜色映射: TRACE=gray, DEBUG=cyan, INFO=green, WARN=yellow, ERROR=red
  - 输出到 stderr: `[LEVEL] [tag] message`
- **Verify**: 各级别日志输出到终端，颜色正确

### Task 1.5: Opcode 定义
- **Files**: `src/Opcode.hh`
- **Depends**: Task 1.2
- **Work**:
  - `enum class OpCode : u8_t` — 全部 40+ 操作码
  - 包含扩展: OP_IMPORT, OP_IMPORT_FROM, OP_IMPORT_ALIAS
  - 可选: opcode_name() 返回操作码名称字符串
- **Verify**: 编译通过

### Task 1.6: Value 类型
- **Files**: `src/Value.hh`, `src/Value.cc`
- **Depends**: Task 1.2
- **Work**:
  - ValueStorage = std::variant<std::monostate, bool, double, Object*>
  - Value class: 构造函数 (nil, bool, double, Object*)
  - 类型检查: is_nil/is_boolean/is_number/is_object
  - 值提取: as_boolean/as_number/as_object
  - 语义操作: is_truthy(), is_equal(), stringify()
  - 对象类型便捷检查: is_string/is_function/is_closure/is_class/is_instance
- **Verify**: 单元级测试——创建各种 Value 并验证类型检查和 stringify

### Task 1.7: Chunk 字节码容器
- **Files**: `src/Chunk.hh`, `src/Chunk.cc`
- **Depends**: Task 1.5, Task 1.6
- **Work**:
  - code_ (vector<u8_t>), constants_ (vector<Value>), lines_ (vector<int>)
  - write(byte, line), write(OpCode, line)
  - add_constant(Value) → sz_t
  - code_at, constant_at, line_at, count
  - operator[] 用于 patch
- **Verify**: 创建 chunk、写入指令和常量、读取验证

### Task 1.8: Debug 反汇编器
- **Files**: `src/Debug.hh`, `src/Debug.cc`
- **Depends**: Task 1.7
- **Work**:
  - disassemble_chunk(chunk, name) — 输出整个 chunk
  - disassemble_instruction(chunk, offset) → sz_t — 输出单条指令
  - 支持所有指令格式: simple, byte, constant, jump, invoke, closure
  - 行号压缩显示（连续同行用 `|`）
- **Verify**: 手动构建 chunk，反汇编输出格式正确

### Task 1.9: main.cc 入口骨架
- **Files**: `src/main.cc`
- **Depends**: Task 1.8
- **Work**:
  - REPL 模式骨架 (argc == 1)
  - 文件执行模式骨架 (argc == 2)
  - 用法错误提示 (argc > 2)
  - 退出码: 0/64/65/70/74
  - Phase 1 验证: 手动创建 chunk，添加 OP_CONSTANT + OP_ADD + OP_RETURN，反汇编输出
- **Verify**: 构建运行，输出反汇编结果和彩色日志

---

## Phase 2: Frontend — 词法分析与编译器（表达式）

### Task 2.1: Token 系统
- **Files**: `src/TokenTypes.hh`, `src/Token.hh`, `src/Token.cc`
- **Depends**: Phase 1
- **Work**:
  - `TokenTypes.hh` — X-macro 定义: PUNCTUATOR, TOKEN, KEYWORD
    - 扩展 clox: 添加 KEYWORD(FROM, "from"), KEYWORD(AS, "as")
  - Token struct: type(TokenType), start(cstr_t), length(sz_t), line(int)
  - TokenType enum class (由 X-macro 生成)
  - token_type_name() 辅助函数
- **Verify**: 编译通过，TokenType 枚举值正确

### Task 2.2: Scanner 词法分析器
- **Files**: `src/Scanner.hh`, `src/Scanner.cc`
- **Depends**: Task 2.1
- **Work**:
  - Scanner class: source 指针 + current/start/line 状态
  - scan_token() — 按需返回单个 Token
  - 单字符: ( ) { } , . - + ; / *
  - 双字符: ! != = == > >= < <=
  - 字面量: string (双引号), number (整数+小数)
  - 标识符 + 关键字识别 (trie-style check_keyword)
  - 空白跳过 + 注释跳过 (//)
  - 错误 token 生成
- **Verify**: 扫描测试源码 `print 1 + 2;`，输出 token 序列正确

### Task 2.3: Compiler — 表达式编译
- **Files**: `src/Compiler.hh`, `src/Compiler.cc`
- **Depends**: Task 2.2, Task 1.7
- **Work**:
  - Parser 状态: current_, previous_, had_error_, panic_mode_
  - advance(), consume(), check(), match()
  - 错误处理: error_at_current(), error(), error_at(), synchronize()
  - 字节码发射: emit_byte, emit_bytes, emit_op, emit_return, emit_constant, make_constant
  - Pratt Parser 核心:
    - ParseRule 表 (prefix, infix, precedence) for 每个 TokenType
    - parse_precedence(Precedence)
    - expression()
  - 表达式解析函数:
    - number() — 解析数字字面量
    - string() — 解析字符串字面量 (此时创建 ObjString 需延后到有 Object 系统)
    - grouping() — 括号
    - unary() — ! -
    - binary() — + - * / == != < > <= >=
    - literal() — true false nil
  - compile(source) → ObjFunction* (暂返回 nullptr，用 Chunk 直接测试)
  - print_statement() — print expr ;
  - expression_statement() — expr ;
  - declaration() → statement() 主循环
- **Verify**: 编译 `print 1 + 2 * 3;`，反汇编输出正确字节码序列

---

## Phase 3: VM Core — 虚拟机执行

### Task 3.1: Table 哈希表
- **Files**: `src/Table.hh`, `src/Table.cc`
- **Depends**: Phase 1
- **Work**:
  - Entry struct: ObjString* key + Value value
  - Table class: entries_ (vector<Entry>), count_
  - find_entry(key) — 线性探测查找
  - get(key, value*), set(key, value), remove(key)
  - add_all(from) — 复制所有条目
  - find_string(chars, length, hash) — 字符串驻留查找
  - adjust_capacity() — 扩容 + rehash
  - tombstone 删除策略
  - GC 支持: mark_entries(), remove_white()
- **Verify**: 插入、查找、删除、扩容测试

### Task 3.2: Object 基础类型
- **Files**: `src/Object.hh`, `src/Object.cc`
- **Depends**: Task 1.6, Task 3.1
- **Work**:
  - ObjectType 枚举: OBJ_STRING, OBJ_FUNCTION, OBJ_NATIVE, OBJ_CLOSURE, OBJ_UPVALUE, OBJ_CLASS, OBJ_INSTANCE, OBJ_BOUND_METHOD, OBJ_MODULE
  - Object 基类: type, is_marked, next, virtual stringify()
  - ObjString: value(str_t), hash(u32_t), stringify()
  - ObjFunction: arity, upvalue_count, chunk, name(ObjString*), stringify()
  - as_obj<T>() 模板辅助函数
  - hash_string() 哈希函数 (FNV-1a)
  - 其余对象类型在后续 Phase 添加
- **Verify**: 创建 ObjString 和 ObjFunction，stringify 正确

### Task 3.3: VM 核心 — 算术与栈操作
- **Files**: `src/VM.hh`, `src/VM.cc`
- **Depends**: Task 3.2, Task 2.3
- **Work**:
  - VM : Singleton<VM>
  - 栈: stack_ (array), stack_top_, push(), pop(), peek(), reset_stack()
  - 全局状态: globals_ (Table), strings_ (Table), objects_ (Object* 链表头)
  - interpret(source) — 调用 Compiler::compile，然后 run
  - run() — 主分发循环 (for + switch)
  - 实现操作码:
    - OP_CONSTANT, OP_NIL, OP_TRUE, OP_FALSE
    - OP_ADD, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_NEGATE
    - OP_NOT, OP_EQUAL, OP_GREATER, OP_LESS
    - OP_POP, OP_PRINT, OP_RETURN
  - READ_BYTE, READ_SHORT, READ_CONSTANT, READ_STRING 辅助宏/函数
  - runtime_error() 报错
  - 字符串分配: allocate_string() + 驻留到 strings_ table
  - OP_ADD 特殊处理: 数字相加 / 字符串拼接
  - 连接 Compiler → VM → 输出
- **Verify**: `print 1 + 2;` → 输出 3, `print "hello" + " world";` → 输出 hello world

### Task 3.4: 全局变量
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Task 3.3
- **Work**:
  - Compiler:
    - var_declaration() — `var name = expr;` / `var name;`
    - identifier_constant() — 将标识符名写入常量池
    - parse_variable(), define_variable()
    - named_variable() — 变量读取/赋值
    - variable() prefix handler
  - VM:
    - OP_DEFINE_GLOBAL, OP_GET_GLOBAL, OP_SET_GLOBAL
    - 全局变量存储在 globals_ Table
- **Verify**: `var x = 10; print x;` → 输出 10

### Task 3.5: 局部变量与作用域
- **Files**: `src/Compiler.cc` (扩展)
- **Depends**: Task 3.4
- **Work**:
  - Compiler:
    - Local struct: name(Token), depth(int), is_captured(bool)
    - locals_ 数组, local_count_, scope_depth_
    - begin_scope(), end_scope()
    - block() — `{ declarations }`
    - add_local(), declare_variable(), mark_initialized()
    - resolve_local() — 按名称查找局部变量槽位
  - VM:
    - OP_GET_LOCAL, OP_SET_LOCAL — 直接读写 frame->slots[slot]
- **Verify**: `{ var a = 1; var b = 2; print a + b; }` → 输出 3; 作用域遮蔽测试

### Task 3.6: 控制流
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Task 3.5
- **Work**:
  - Compiler:
    - if_statement() — emit OP_JUMP_IF_FALSE + patch
    - while_statement() — emit OP_LOOP
    - for_statement() — 脱糖为 while
    - and_() — 短路求值 (OP_JUMP_IF_FALSE)
    - or_() — 短路求值 (OP_JUMP)
    - emit_jump(), patch_jump(), emit_loop()
  - VM:
    - OP_JUMP, OP_JUMP_IF_FALSE, OP_LOOP
- **Verify**: `tests/control_flow.ms` 通过

### Task 3.7: Phase 3 测试脚本
- **Files**: `tests/arithmetic.ms`, `tests/variables.ms`, `tests/strings.ms`, `tests/control_flow.ms`
- **Depends**: Task 3.6
- **Work**: 编写带 `// expect:` 注释的测试脚本
- **Verify**: 手动运行全部通过

---

## Phase 4: Functions & Closures

### Task 4.1: ObjNative 与原生函数
- **Files**: `src/Object.hh/cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Phase 3
- **Work**:
  - ObjNative class: function(NativeFn), name
  - NativeFn = Value(*)(int arg_count, Value* args)
  - VM::define_native() — 注册原生函数到 globals
  - clock() 原生函数实现
  - VM::call_value() — 处理 ObjNative 调用
- **Verify**: `print clock();` 输出时间戳

### Task 4.2: 函数编译与调用
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展), `src/Object.hh/cc` (扩展)
- **Depends**: Task 4.1
- **Work**:
  - Compiler:
    - FunctionType 枚举: TYPE_FUNCTION, TYPE_SCRIPT, TYPE_INITIALIZER, TYPE_METHOD
    - fun_declaration() — 编译函数声明
    - function(FunctionType) — 创建嵌套 Compiler，编译参数和函数体
    - argument_list() — 解析调用参数
    - call() infix handler — 解析 `expr(args)`
    - return_statement()
  - VM:
    - CallFrame struct: closure, ip, slots
    - frames_ 数组, frame_count_
    - call(closure, arg_count) — 创建新 CallFrame
    - call_value() — 分发调用 (closure / native / class)
    - OP_CALL, OP_RETURN
  - ObjClosure: function + upvalues vector (此阶段 upvalues 为空)
  - compile() 返回 ObjFunction*，VM 包装为 ObjClosure 执行
- **Verify**: `fun add(a, b) { return a + b; } print add(1, 2);` → 输出 3

### Task 4.3: Closures 与 Upvalues
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展), `src/Object.hh/cc` (扩展)
- **Depends**: Task 4.2
- **Work**:
  - Compiler:
    - Upvalue struct: index, is_local
    - resolve_upvalue() — 递归查找外层变量
    - add_upvalue() — 添加 upvalue 记录
    - OP_CLOSURE 发射: 函数常量 + N*(isLocal + index) upvalue 元数据
    - 标记 Local::is_captured
    - end_scope() 中对 captured locals 发射 OP_CLOSE_UPVALUE
  - VM:
    - ObjUpvalue class: location(Value*), closed(Value), next(ObjUpvalue*)
    - open_upvalues_ 链表 (按栈位置降序)
    - capture_upvalue(Value* local) — 查找/创建 upvalue
    - close_upvalues(Value* last) — 关闭 upvalue (栈→堆)
    - OP_CLOSURE — 创建 ObjClosure + 捕获 upvalues
    - OP_GET_UPVALUE, OP_SET_UPVALUE
    - OP_CLOSE_UPVALUE
- **Verify**: `tests/closures.ms` 通过 (makeCounter 等经典测试)

### Task 4.4: Phase 4 测试脚本
- **Files**: `tests/functions.ms`, `tests/closures.ms`
- **Depends**: Task 4.3
- **Work**: 函数递归、原生函数、闭包捕获、upvalue 共享等测试
- **Verify**: 全部通过

---

## Phase 5: OOP — 类与继承

### Task 5.1: Class 声明与实例化
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展), `src/Object.hh/cc` (扩展)
- **Depends**: Phase 4
- **Work**:
  - ObjClass: name(ObjString*), methods (unordered_map<ObjString*, Value>)
  - ObjInstance: klass(ObjClass*), fields (unordered_map<ObjString*, Value>)
  - Compiler:
    - class_declaration() — 解析 class Name { ... }
    - ClassCompiler struct: enclosing, has_superclass
    - OP_CLASS 发射
  - VM:
    - OP_CLASS — 创建 ObjClass
    - call_value() 处理 ObjClass — 创建 ObjInstance
    - OP_GET_PROPERTY, OP_SET_PROPERTY — 实例字段访问
    - 属性查找链: 先字段 → 再方法
- **Verify**: `class Foo {} var f = Foo(); f.x = 42; print f.x;` → 输出 42

### Task 5.2: Methods 与 this
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展), `src/Object.hh/cc` (扩展)
- **Depends**: Task 5.1
- **Work**:
  - ObjBoundMethod: receiver(Value), method(ObjClosure*)
  - Compiler:
    - method() — 编译方法体
    - this_() — this 表达式（视为局部变量 slot 0）
    - TYPE_INITIALIZER 处理 (init 方法)
  - VM:
    - OP_METHOD — 将 closure 添加到 class.methods
    - bind_method() — 创建 ObjBoundMethod
    - OP_INVOKE — 优化的方法调用 (跳过 bind)
    - init 方法: 自动调用，返回 this
- **Verify**: `class Cat { init(n) { this.name = n; } speak() { print this.name; } } Cat("Tom").speak();` → 输出 Tom

### Task 5.3: Inheritance 与 super
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Task 5.2
- **Work**:
  - Compiler:
    - 继承语法: `class Sub : Super { ... }`
    - super_() — super.method 表达式
    - OP_INHERIT, OP_GET_SUPER 发射
  - VM:
    - OP_INHERIT — 复制父类方法到子类
    - OP_GET_SUPER — 从父类查找方法并 bind
    - OP_SUPER_INVOKE — 优化的 super 方法调用
- **Verify**: `tests/classes.ms` 通过（含继承、super 测试）

### Task 5.4: Phase 5 测试脚本
- **Files**: `tests/classes.ms`
- **Depends**: Task 5.3
- **Work**: 类、实例、方法、this、继承、super、方法覆盖等测试
- **Verify**: 全部通过

---

## Phase 6: Garbage Collection

### Task 6.1: GC 基础设施
- **Files**: `src/Memory.hh`, `src/Memory.cc`, `src/VM.hh/cc` (扩展)
- **Depends**: Phase 5
- **Work**:
  - VM GC 状态: bytes_allocated_, next_gc_, gray_stack_
  - allocate_object<T>() — 分配时跟踪字节数，检查 GC 触发
  - collect_garbage() — 入口函数
  - Logger 输出 GC 活动 (MAPLE_DEBUG_LOG_GC)
- **Verify**: GC 框架就绪，尚未实际回收

### Task 6.2: Mark Phase
- **Files**: `src/Memory.cc` (扩展)
- **Depends**: Task 6.1
- **Work**:
  - mark_roots() — 遍历所有 GC roots
    - VM stack (stack_ → stack_top_)
    - globals_ table
    - frames_ closures
    - open_upvalues_ 链
    - compiler function_ (编译期)
    - init_string_
  - mark_object(Object*) — 设置 is_marked, 加入 gray_stack
  - mark_value(Value) — 如果是 Object 则 mark_object
  - mark_table(Table&) — 遍历 table entries
- **Verify**: Mark 阶段正确标记可达对象

### Task 6.3: Trace & Sweep Phase
- **Files**: `src/Memory.cc` (扩展)
- **Depends**: Task 6.2
- **Work**:
  - trace_references() — 处理 gray_stack
    - blacken_object() — 根据对象类型追踪引用
    - ObjClosure → function + upvalues
    - ObjClass → name + methods
    - ObjInstance → klass + fields
    - ObjBoundMethod → receiver + method
    - ObjUpvalue → closed value
    - ObjFunction → name + chunk.constants
    - ObjModule → name + exports
  - sweep() — 遍历 objects_ 链表
    - 未标记: 从链表移除 + delete
    - 已标记: 重置 is_marked = false
  - table remove_white() — 清理字符串驻留表
  - 更新 next_gc_ = bytes_allocated_ * kGC_HEAP_GROW
- **Verify**: 所有 Phase 1-5 测试在 GC 启用下通过

### Task 6.4: GC Stress 测试
- **Files**: (编译选项)
- **Depends**: Task 6.3
- **Work**:
  - MAPLE_DEBUG_STRESS_GC — 每次分配触发 GC
  - 运行全部测试脚本
  - 确保编译器中 GC 安全（分配前将对象 push 到栈保护）
- **Verify**: Stress GC 模式下全部测试通过

---

## Phase 7: Import System

### Task 7.1: Module 加载器
- **Files**: `src/Module.hh`, `src/Module.cc`, `src/Object.hh/cc` (扩展 ObjModule)
- **Depends**: Phase 6
- **Work**:
  - ObjModule class: name(ObjString*), exports (unordered_map<ObjString*, Value>)
  - ModuleLoader class:
    - resolve_path(module_name, importing_file) — 相对路径解析 (std::filesystem)
    - read_source(path) — std::ifstream 读取文件
  - VM::modules_ 缓存 (unordered_map<str_t, ObjModule*>)
  - 循环导入检测 (loading set)
- **Verify**: 可读取模块文件

### Task 7.2: import 语句编译与执行
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Task 7.1
- **Work**:
  - Compiler:
    - import_statement() — `import "path";` → OP_CONSTANT(path) + OP_IMPORT
  - VM:
    - OP_IMPORT 处理:
      1. 取路径字符串
      2. 调用 ModuleLoader 加载（或从缓存取）
      3. 编译执行模块代码
      4. 收集顶层声明为 exports
      5. 创建 ObjModule
      6. 以模块名（文件名去扩展名）定义全局变量
  - 模块成员访问: 通过 OP_GET_PROPERTY 在 ObjModule 上查找
- **Verify**: `import "math.ms"; print math.add(1, 2);` → 输出 3

### Task 7.3: from-import-as 语句
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Task 7.2
- **Work**:
  - Compiler:
    - from_import_statement() — 解析 `from "path" import name;` 和 `from "path" import name as alias;`
    - OP_IMPORT_FROM: path + name → 提取导出，定义为全局
    - OP_IMPORT_ALIAS: path + name + alias → 提取导出，以别名定义
  - VM:
    - OP_IMPORT_FROM, OP_IMPORT_ALIAS 处理
- **Verify**: `from "math.ms" import add as plus; print plus(1, 2);` → 输出 3

### Task 7.4: Phase 7 测试脚本
- **Files**: `tests/import_test.ms`, `tests/modules/math.ms`, `tests/modules/utils.ms`
- **Depends**: Task 7.3
- **Work**:
  - math.ms — `fun add(a, b) { return a + b; }` 等
  - utils.ms — `fun greet(name) { print "Hello, " + name + "!"; }` 等
  - import_test.ms — 测试 import / from-import / from-import-as
- **Verify**: 全部通过

---

## Phase 8: Polish — 完善与测试

### Task 8.1: REPL 完善
- **Files**: `src/main.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Phase 7
- **Work**:
  - 交互式提示符 `maple> `
  - 逐行编译执行
  - 编译错误恢复（不退出 REPL）
  - 空行跳过
- **Verify**: REPL 交互测试

### Task 8.2: 错误信息完善
- **Files**: `src/Compiler.cc` (扩展), `src/VM.cc` (扩展)
- **Depends**: Task 8.1
- **Work**:
  - 编译错误: `[line N] Error at 'token': message`
  - 运行时错误: 错误信息 + 调用栈回溯
  - 常见错误: 未定义变量、类型不匹配、参数数量、除零、属性不存在等
- **Verify**: `tests/errors.ms` 通过

### Task 8.3: 测试运行器
- **Files**: `tests/run_tests.py`
- **Depends**: Phase 7
- **Work**:
  - 解析 `// expect:` 注释
  - 解析 `// expect runtime error:` 注释
  - 运行 mslang 可执行文件
  - 比较实际输出 vs 期望输出
  - 统计通过/失败/总数
  - 支持指定单个测试或目录
- **Verify**: 运行全部测试并报告

### Task 8.4: 示例脚本
- **Files**: `examples/fibonacci.ms`, `examples/counter.ms`, `examples/oop.ms`
- **Depends**: Phase 7
- **Work**:
  - fibonacci.ms — 递归 + 循环两种实现
  - counter.ms — 闭包计数器
  - oop.ms — 类继承 + 多态示例
- **Verify**: 全部示例可运行

### Task 8.5: 文档最终化
- **Files**: `docs/REQUIREMENTS.md`, `docs/DESIGN.md`
- **Depends**: Phase 7
- **Work**:
  - 根据实际实现更新文档
  - 补充实现中发现的细节差异
  - 添加使用说明

---

## Task Dependency Summary

```
Phase 1: 1.1 → 1.2 → 1.3 → 1.4
          1.2 → 1.5, 1.6
          1.5 + 1.6 → 1.7 → 1.8 → 1.9

Phase 2: Phase 1 → 2.1 → 2.2 → 2.3

Phase 3: 2.3 + 3.1 + 3.2 → 3.3 → 3.4 → 3.5 → 3.6 → 3.7

Phase 4: Phase 3 → 4.1 → 4.2 → 4.3 → 4.4

Phase 5: Phase 4 → 5.1 → 5.2 → 5.3 → 5.4

Phase 6: Phase 5 → 6.1 → 6.2 → 6.3 → 6.4

Phase 7: Phase 6 → 7.1 → 7.2 → 7.3 → 7.4

Phase 8: Phase 7 → 8.1, 8.2, 8.3, 8.4, 8.5 (parallel)
```

## Progress Tracking

| Phase | Task | Status |
|-------|------|--------|
| 1 | 1.1 CMake | [x] |
| 1 | 1.2 Platform headers | [x] |
| 1 | 1.3 Colorful system | [x] |
| 1 | 1.4 Logger | [x] |
| 1 | 1.5 Opcode | [x] |
| 1 | 1.6 Value | [x] |
| 1 | 1.7 Chunk | [x] |
| 1 | 1.8 Debug disassembler | [x] |
| 1 | 1.9 main.cc skeleton | [x] |
| 2 | 2.1 Token system | [ ] |
| 2 | 2.2 Scanner | [ ] |
| 2 | 2.3 Compiler (expressions) | [ ] |
| 3 | 3.1 Table | [ ] |
| 3 | 3.2 Object basics | [ ] |
| 3 | 3.3 VM core | [ ] |
| 3 | 3.4 Global variables | [ ] |
| 3 | 3.5 Local variables & scope | [ ] |
| 3 | 3.6 Control flow | [ ] |
| 3 | 3.7 Phase 3 tests | [ ] |
| 4 | 4.1 Native functions | [ ] |
| 4 | 4.2 Function compilation | [ ] |
| 4 | 4.3 Closures & upvalues | [ ] |
| 4 | 4.4 Phase 4 tests | [ ] |
| 5 | 5.1 Classes & instances | [ ] |
| 5 | 5.2 Methods & this | [ ] |
| 5 | 5.3 Inheritance & super | [ ] |
| 5 | 5.4 Phase 5 tests | [ ] |
| 6 | 6.1 GC infrastructure | [ ] |
| 6 | 6.2 Mark phase | [ ] |
| 6 | 6.3 Trace & sweep | [ ] |
| 6 | 6.4 GC stress test | [ ] |
| 7 | 7.1 Module loader | [ ] |
| 7 | 7.2 import statement | [ ] |
| 7 | 7.3 from-import-as | [ ] |
| 7 | 7.4 Phase 7 tests | [ ] |
| 8 | 8.1 REPL polish | [ ] |
| 8 | 8.2 Error messages | [ ] |
| 8 | 8.3 Test runner | [ ] |
| 8 | 8.4 Example scripts | [ ] |
| 8 | 8.5 Documentation | [ ] |
