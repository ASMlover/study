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

---

## Phase 9: Improvements — Bug 修复、代码质量与语言特性增强

> 基于对已完成 Phase 1-8 核心实现的全面审查，列出以下改进点。
> 按优先级分为：Bug 修复（必须）、代码质量（建议）、语言特性（扩展）。

### 9.1 Bug 修复

#### Task 9.1.1: GC `remove_white()` 时序错误 [严重]
- **Files**: `src/VM.cc`
- **Problem**: `VM::allocate()` 中非 stress-GC 路径下 `strings_.remove_white()` 在 `collect_garbage()` **之前**调用。此时所有对象 `is_marked_` 为 false（上轮 sweep 已重置），导致所有 interned string 被从驻留表移除。
- **Fix**: 将 `remove_white()` 移至 `Memory.cc::collect_garbage()` 中 `trace_references()` 之后、`sweep()` 之前，与 clox 原实现一致。
- **Verify**: 启用 `MAPLE_DEBUG_STRESS_GC` 运行全部测试通过。

#### Task 9.1.2: `VM::push()` 无栈溢出检查
- **Files**: `src/VM.cc`
- **Problem**: `push()` 直接写入 `*stack_top_++`，无边界检查。恶意或深度递归脚本可越界写入。
- **Fix**: 添加 `stack_top_ < stack_.data() + stack_.size()` 断言或运行时检查。
- **Verify**: 构造深度超限脚本，验证报错而非崩溃。

#### Task 9.1.3: `import_module` 未使用 `ModuleLoader`
- **Files**: `src/VM.cc`, `src/Module.hh/cc`
- **Problem**: `VM::import_module()` 直接用 `std::ifstream` 读文件，未使用已有的 `ModuleLoader::read_source()` 和 `ModuleLoader::resolve_path()`。导致：(1) 代码重复 (2) 无相对路径解析——import 路径只能是相对于 CWD 的路径，无法相对于导入文件路径解析。
- **Fix**: 重构 `import_module` 使用 `ModuleLoader`，传入当前执行文件路径用于相对路径解析。
- **Verify**: 跨目录 import 测试（如 `tests/nested/` 目录结构）。

#### Task 9.1.4: `Value::stringify()` 大数值转 int 溢出
- **Files**: `src/Value.cc`
- **Problem**: `stringify()` 将整数 double 强转 `i32_t`，当值超过 `INT32_MAX` 时溢出产生错误输出。
- **Fix**: 改用 `i64_t` 或 `static_cast<long long>` 并检查范围。
- **Verify**: `print 2147483648;` 应输出 `2147483648` 而非负数。

### 9.2 代码质量改进

#### Task 9.2.1: Compiler 静态状态消除
- **Files**: `src/Compiler.hh`, `src/Compiler.cc`
- **Problem**: `scanner_`, `current_`, `previous_`, `had_error_`, `panic_mode_`, `current_class_`, `current_compiler_` 全部是 static 成员，定义在 `.cc` 文件中。这使得 Compiler 不可重入、非线程安全，且静态成员定义在 .cc 中违反 ODR 惯例。
- **Fix**: 将 Parser 状态抽取为 `ParseState` 结构体，通过指针在 Compiler 实例间共享（第一个 Compiler 创建时分配，嵌套 Compiler 继承同一指针）。
- **Verify**: 编译通过，全部测试通过。

#### Task 9.2.2: GC 耦合解耦 — 消除全局 `vm_*` 函数
- **Files**: `src/VM.hh/cc`, `src/Memory.hh/cc`
- **Problem**: `Memory.cc` 通过 6 个全局 `vm_*()` 自由函数访问 VM 的 GC 状态。VM.cc 中还有大量 `allocate_object` 模板显式实例化。耦合严重。
- **Fix**: 引入 `GCContext` 结构体（持有 objects_, bytes_allocated_, next_gc_, gray_stack_ 引用），`collect_garbage(GCContext&)` 接收上下文；或将 GC 逻辑移入 VM 类方法。
- **Verify**: 编译通过，全部测试通过。

#### Task 9.2.3: Object 成员可见性封装
- **Files**: `src/Object.hh`
- **Problem**: 所有 Object 子类成员（`value_`, `hash_`, `function_`, `klass_`, `methods_`, `fields_`, `exports_` 等）全部 public，外部直接读写。
- **Fix**: 将成员改为 private/protected，提供 accessor 方法。对 VM 热路径（如 `function_->chunk_`）可用 friend 或保留 public。
- **Verify**: 编译通过。

#### Task 9.2.4: `Table::get()` 消除 `const_cast`
- **Files**: `src/Table.hh/cc`
- **Problem**: `get()` 是 const 方法但内部 `const_cast` 调用非 const 的 `find_entry()`。
- **Fix**: 将 `find_entry` 拆分为 const 和 non-const 版本，或使其内部不修改状态（已经满足）则标记为 const。
- **Verify**: 编译通过。

#### Task 9.2.5: 统一 Object 字段/方法存储 — Table 替代 `std::unordered_map`
- **Files**: `src/Object.hh/cc`, `src/VM.cc`
- **Problem**: `globals_` 和 `strings_` 使用自定义 `Table`（支持 GC mark/remove_white），但 `ObjClass::methods_`、`ObjInstance::fields_`、`ObjModule::exports_` 使用 `std::unordered_map<ObjString*, Value>`。GC 交互方式不一致（Table 用 mark_table，unordered_map 需手动遍历 trace_references）。
- **Fix**: 统一使用 `Table` 存储所有 ObjString* -> Value 映射，或为 unordered_map 提供统一的 GC mark 工具函数。
- **Verify**: 全部测试通过，GC stress 测试通过。

#### Task 9.2.6: ParseRule 表安全化
- **Files**: `src/Compiler.cc`
- **Problem**: `rules[]` 是 C 风格数组，按 `TokenType` 枚举值索引。如果 TokenTypes.hh 新增/调整枚举顺序，rules 数组与枚举不同步将导致静默错误。
- **Fix**: 改用 `std::array<ParseRule, TOKEN_COUNT>` 或 `std::unordered_map<TokenType, ParseRule>`，并用静态断言验证大小匹配；或在每条规则前使用 `[static_cast<int>(TokenType::TOKEN_XXX)] = {...}` designated initializer 风格。
- **Verify**: 编译通过。

#### Task 9.2.7: `Compiler::number()` 避免临时 string 构造
- **Files**: `src/Compiler.cc`
- **Problem**: `number()` 用 `std::strtod(str_t(previous_.lexeme).c_str(), nullptr)` 从 `strv_t` 解析数字，每次分配临时 `std::string`。
- **Fix**: 使用 `std::from_chars()`（C++17），直接操作 `strv_t` 的底层指针，零分配。
- **Verify**: 数值解析测试通过。

### 9.3 语言特性增强

#### Task 9.3.1: `break` / `continue` 语句
- **Files**: `src/TokenTypes.hh`, `src/Scanner.cc`, `src/Compiler.cc`, `src/Opcode.hh`, `src/VM.cc`
- **Work**:
  - 新增 `TOKEN_BREAK`, `TOKEN_CONTINUE` 关键字
  - Compiler 记录当前循环上下文（loop_start, scope_depth, break patch 列表）
  - `break` 发射 OP_JUMP 跳至循环后，`continue` 发射 OP_LOOP 跳至增量/条件
  - 循环结束时 patch 所有 break jump
- **Verify**: `tests/break_continue.ms` 测试通过。

#### Task 9.3.2: 取模运算符 `%`
- **Files**: `src/TokenTypes.hh`, `src/Scanner.cc`, `src/Opcode.hh`, `src/Compiler.cc`, `src/VM.cc`
- **Work**:
  - 新增 `TOKEN_PERCENT` 标点、`OP_MODULO` 操作码
  - Scanner 识别 `%`，Compiler 按 PREC_FACTOR 优先级处理
  - VM 中使用 `std::fmod(a, b)` 实现
- **Verify**: `print 10 % 3;` → 输出 1。

#### Task 9.3.3: 字符串转义序列
- **Files**: `src/Scanner.cc` 或 `src/Compiler.cc`
- **Work**:
  - 在 `scan_string()` 或 `string()` 编译时处理 `\n`, `\t`, `\\`, `\"`, `\r`, `\0`
  - 遇到 `\` 时消费下一字符并替换为对应控制字符
- **Verify**: `print "hello\tworld\n";` 输出含制表符和换行。

#### Task 9.3.4: OP_CONSTANT_LONG — 扩展常量池
- **Files**: `src/Opcode.hh`, `src/Chunk.hh/cc`, `src/Compiler.cc`, `src/VM.cc`, `src/Debug.cc`
- **Problem**: `make_constant()` 将常量索引限制为 u8_t（最多 256 个），大函数或含大量字符串的脚本会报 "Too many constants"。
- **Work**:
  - 新增 `OP_CONSTANT_LONG`：操作数为 3 字节（24 位索引，最多 16M 常量）
  - `make_constant()` 在 index > 255 时自动切换为 long 版本
  - Debug 反汇编器支持 long constant
- **Verify**: 构造超过 256 个常量的测试脚本。

#### Task 9.3.5: Chunk 行号 RLE 压缩
- **Files**: `src/Chunk.hh/cc`
- **Problem**: `lines_` 为 `vector<int>`，每个字节码对应一个 int。一个 1000 行脚本可能产生 4000+ 字节码，占用 16KB+ 仅用于行号。
- **Work**:
  - 改用 RLE 编码：`vector<pair<int, int>>` 或 `vector<LineStart>`（{line, offset}）
  - `line_at(offset)` 用二分查找
- **Verify**: 全部测试通过，内存占用下降。

#### Task 9.3.6: 更多原生函数
- **Files**: `src/VM.cc`
- **Work**:
  - `type(value)` → 返回值类型字符串 ("nil"/"bool"/"number"/"string"/"function"/"class"/"instance")
  - `strlen(str)` → 字符串长度
  - `str(value)` → 值转字符串
  - `num(str)` → 字符串转数字
  - `input([prompt])` → 读取标准输入行
- **Verify**: 各原生函数测试通过。

#### Task 9.3.7: 列表（Array）数据类型
- **Files**: 新增 `ObjList` 类型，修改 Scanner/Compiler/VM
- **Work**:
  - `ObjList`: 持有 `vector<Value>`，GC trace 所有元素
  - 语法: `var a = [1, 2, 3]; a[0]; a[1] = 42; a.push(5); a.len();`
  - 新增 `TOKEN_LEFT_BRACKET`, `TOKEN_RIGHT_BRACKET`
  - 新增 `OP_BUILD_LIST`, `OP_INDEX_GET`, `OP_INDEX_SET`
- **Verify**: `tests/lists.ms` 测试通过。

#### Task 9.3.8: Map（Dictionary）数据类型
- **Files**: 新增 `ObjMap` 类型，修改 Compiler/VM
- **Work**:
  - `ObjMap`: 持有 `unordered_map<Value, Value>` 或基于 Table 的实现
  - 语法: `var m = {"a": 1, "b": 2}; m["a"]; m["c"] = 3;`
  - 复用 `OP_INDEX_GET`, `OP_INDEX_SET`
- **Verify**: `tests/maps.ms` 测试通过。

### 9.4 性能优化（可选）

#### Task 9.4.1: NaN-boxing Value 表示
- **Files**: `src/Value.hh/cc`
- **Problem**: `std::variant` 占 16 字节（8 字节 double + 8 字节 type tag/alignment），且 `holds_alternative` / `get` 有分支开销。
- **Work**:
  - 利用 IEEE 754 NaN 的 quiet NaN 空间编码 nil/bool/Object*
  - Value 缩小为 8 字节，类型检查变为位运算
  - 保持 API 兼容（is_nil/is_number/as_number 等接口不变）
- **Verify**: 全部测试通过，fibonacci 基准测试性能提升。

#### Task 9.4.2: computed goto 分发（GCC/Clang）
- **Files**: `src/VM.cc`
- **Problem**: `switch` 分发在每次迭代有间接跳转，现代 CPU 分支预测不友好。
- **Work**:
  - 使用 GCC/Clang `&&label` 扩展实现 threaded dispatch
  - 用 `#ifdef MAPLE_GNUC` 条件编译，MSVC 回退 switch
- **Verify**: 性能基准测试对比。

---

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
| 2 | 2.1 Token system | [x] |
| 2 | 2.2 Scanner | [x] |
| 2 | 2.3 Compiler (expressions) | [x] |
| 3 | 3.1 Table | [x] |
| 3 | 3.2 Object basics | [x] |
| 3 | 3.3 VM core | [x] |
| 3 | 3.4 Global variables | [x] |
| 3 | 3.5 Local variables & scope | [x] |
| 3 | 3.6 Control flow | [x] |
| 3 | 3.7 Phase 3 tests | [x] |
| 4 | 4.1 Native functions | [x] |
| 4 | 4.2 Function compilation | [x] |
| 4 | 4.3 Closures & upvalues | [x] |
| 4 | 4.4 Phase 4 tests | [x] |
| 5 | 5.1 Classes & instances | [x] |
| 5 | 5.2 Methods & this | [x] |
| 5 | 5.3 Inheritance & super | [x] |
| 5 | 5.4 Phase 5 tests | [x] |
| 6 | 6.1 GC infrastructure | [x] |
| 6 | 6.2 Mark phase | [x] |
| 6 | 6.3 Trace & sweep | [x] |
| 6 | 6.4 GC stress test | [x] |
| 7 | 7.1 Module loader | [x] |
| 7 | 7.2 import statement | [x] |
| 7 | 7.3 from-import-as | [x] |
| 7 | 7.4 Phase 7 tests | [x] |
| 8 | 8.1 REPL polish | [x] |
| 8 | 8.2 Error messages | [x] |
| 8 | 8.3 Test runner | [x] |
| 8 | 8.4 Example scripts | [x] |
| 8 | 8.5 Documentation | [x] |
| **9** | **9.1.1 GC remove_white 时序** | [x] |
| 9 | 9.1.2 push 栈溢出检查 | [x] |
| 9 | 9.1.3 import 使用 ModuleLoader | [x] |
| 9 | 9.1.4 stringify 大数溢出 | [x] |
| 9 | 9.2.1 Compiler 静态状态消除 | [x] |
| 9 | 9.2.2 GC 耦合解耦 | [x] |
| 9 | 9.2.3 Object 成员封装 | [x] |
| 9 | 9.2.4 Table const_cast 消除 | [x] |
| 9 | 9.2.5 统一 Table 存储 | [x] |
| 9 | 9.2.6 ParseRule 表安全化 | [x] |
| 9 | 9.2.7 number 零分配解析 | [x] |
| 9 | 9.3.1 break/continue | [x] |
| 9 | 9.3.2 取模运算符 % | [x] |
| 9 | 9.3.3 字符串转义序列 | [x] |
| 9 | 9.3.4 OP_CONSTANT_LONG | [x] |
| 9 | 9.3.5 行号 RLE 压缩 | [ ] |
| 9 | 9.3.6 更多原生函数 | [ ] |
| 9 | 9.3.7 列表数据类型 | [ ] |
| 9 | 9.3.8 Map 数据类型 | [ ] |
| 9 | 9.4.1 NaN-boxing | [ ] |
| 9 | 9.4.2 computed goto | [ ] |
