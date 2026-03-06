# Maple 脚本语言实施计划

本文档将DESIGN.md中的设计拆分为具体的、可独立运行验证的开发任务。

## 当前进度

| 阶段 | 状态 | 说明 |
|------|------|------|
| 阶段1：基础基础设施 | ✅ 已完成 | CMake构建系统、基础定义模块、日志系统已实现并通过编译验证 |
| 阶段2：内存与值系统 | ✅ 已完成 | 动态数组、NaN Boxing值系统、内存管理、字符串对象已实现并通过测试 |
| 阶段3：字节码与调试 | ⏳ 未开始 | 等待实现 |
| 阶段4：虚拟机核心 | ⏳ 未开始 | 等待实现 |
| 阶段5：词法分析器 | ⏳ 未开始 | 等待实现 |
| 阶段6：编译器基础 | ⏳ 未开始 | 等待实现 |
| 阶段7：控制流 | ⏳ 未开始 | 等待实现 |
| 阶段8：函数系统 | ⏳ 未开始 | 等待实现 |
| 阶段9：面向对象 | ⏳ 未开始 | 等待实现 |
| 阶段10：垃圾回收 | ⏳ 未开始 | 等待实现 |
| 阶段11：Import系统 | ⏳ 未开始 | 等待实现 |
| 阶段12：标准库 | ⏳ 未开始 | 等待实现 |

---

## 阶段1：基础基础设施


### 任务1.1：项目构建系统
**目标**：建立CMake构建系统，创建项目目录结构

**验收标准**：
- [x] CMakeLists.txt配置正确，支持C++20
- [x] 可创建build目录并编译成功
- [x] 支持Debug/Release模式切换
- [x] 支持调试宏开关（ENABLE_DEBUG, ENABLE_STRESS_GC）

**验证命令**：
```bash
mkdir -p build && cd build
cmake ..
cmake --build . --config Debug
```

**实际验证结果**：✅ 编译成功，生成 `build/bin/maple.exe`
```
./bin/maple.exe
[INFO] Maple Script Language v0.1.0
[WARN] No script provided, starting REPL...
[ERROR] REPL not implemented yet
```

**实现文件**：`CMakeLists.txt`

---

### 任务1.2：基础定义模块（common.hh）
**目标**：定义项目基础常量、类型别名、调试宏

**验收标准**：
- [x] 定义命名空间 `ms`
- [x] 定义基础类型别名（u8, u16, u32, u64, i32, i64, usize等）
- [x] 定义编译期常量（STACK_MAX=256, FRAMES_MAX=64, GC_HEAP_GROW_FACTOR=2）
- [x] 定义调试配置结构体 DebugConfig（支持CMake传递的宏定义）
- [x] 定义工具宏 MS_ASSERT, MS_DISABLE_COPY, MS_DISABLE_MOVE等
- [x] 定义工具函数 arraySize, divCeil, alignUp

**验证方式**：✅ 编译通过，logger.hh/cc可正确包含

**实现文件**：`src/common.hh`

---

### 任务1.3：日志系统（logger.hh/cc）
**目标**：实现分级彩色日志系统

**验收标准**：
- [x] 实现LogLevel枚举（Trace, Debug, Info, Warn, Error, Fatal）
- [x] 实现带颜色的日志输出（ANSI颜色代码）
- [x] 提供日志级别控制（setLevel/getLevel）
- [x] 提供静态方法：trace, debug, info, warn, error, fatal

**实现说明**：
- 使用模板方法支持任意可流式输出的类型
- 颜色映射：Trace(灰), Debug(青), Info(绿), Warn(黄), Error(红), Fatal(紫)
- Fatal级别输出后调用 `std::abort()` 终止程序

**实现文件**：
- `src/logger.hh` - 日志系统头文件
- `src/logger.cc` - 日志系统实现

**验证方式**：✅ 编译通过，运行输出：
```
[INFO] Maple Script Language v0.1.0
[WARN] No script provided, starting REPL...
[ERROR] REPL not implemented yet
```

---

## 阶段2：内存与值系统

### 任务2.1：动态数组实现
**目标**：实现模板化的动态数组，用于常量池等

**验收标准**：
- [x] 模板类 `DynArray<T>`
- [x] 支持动态扩容
- [x] 支持索引访问
- [x] 支持迭代

**验证测试**：
```cpp
// test_dynarray.cpp
ms::DynArray<int> arr;
arr.write(10);
arr.write(20);
arr.write(30);
assert(arr.count() == 3);
assert(arr[0] == 10);
```

---

### 任务2.2：值类型系统（value.hh/cc）
**目标**：实现NaN Boxing技术的动态类型系统

**验收标准**：
- [x] 实现ValueType枚举（Nil, Bool, Number, Object）
- [x] 实现Value类，使用NaN Boxing存储
- [x] 实现类型检查和转换方法
- [x] 支持nil, bool, double, Obj*的统一存储
- [x] 实现值的比较操作（==, <, >等）

**单元测试** `tests/unit/test_value.cpp`：
```cpp
void test_value() {
    ms::Value nil_val = ms::nil_val();
    assert(nil_val.isNil());
    
    ms::Value bool_val = ms::bool_val(true);
    assert(bool_val.isBool() && bool_val.asBool() == true);
    
    ms::Value num_val = ms::number_val(3.14);
    assert(num_val.isNumber() && num_val.asNumber() == 3.14);
    
    // NaN Boxing验证
    assert(sizeof(ms::Value) == sizeof(uint64_t));
    
    print("Value tests passed!");
}
```

---

### 任务2.3：内存管理基础（memory.hh/cc）
**目标**：实现内存分配器和GC基础接口

**验收标准**：
- [x] 实现 `reallocate()` 函数
- [x] 实现分配时GC触发检查
- [x] 定义GC相关宏（GROW_CAPACITY, FREE_ARRAY等）

**验证方式**：Valgrind/ASan检测无内存泄漏

---

### 任务2.4：对象系统基础（object.hh/cc）
**目标**：实现对象基类和字符串对象

**验收标准**：
- [x] 实现Obj结构体（type, isMarked, next字段）
- [x] 实现ObjType枚举
- [x] 实现ObjString（字符串驻留）
- [x] 实现字符串哈希预计算
- [x] 实现GC链表管理

**单元测试** `tests/unit/test_object.cpp`：
```cpp
void test_string_interning() {
    auto* s1 = ms::copyString("hello", 5);
    auto* s2 = ms::copyString("hello", 5);
    assert(s1 == s2);  // 同一指针（驻留）
    print("String interning tests passed!");
}
```

---

## 阶段3：字节码与调试

### 任务3.1：字节码块（chunk.hh/cc）
**目标**：实现字节码存储和常量池

**验收标准**：
- [ ] 实现Chunk类
- [ ] 支持字节码写入
- [ ] 支持常量池管理
- [ ] 支持行号信息（调试）
- [ ] 实现OP_CONSTANT, OP_CONSTANT_LONG

**单元测试** `tests/unit/test_chunk.cpp`：
```cpp
void test_chunk() {
    ms::Chunk chunk;
    int constant = chunk.addConstant(ms::number_val(1.2));
    chunk.write(ms::to_underlying(ms::OpCode::CONSTANT), 123);
    chunk.write(constant, 123);
    chunk.write(ms::to_underlying(ms::OpCode::RETURN), 123);
    
    assert(chunk.count() == 3);
    print("Chunk tests passed!");
}
```

---

### 任务3.2：反汇编调试器（debug.hh/cc）
**目标**：实现字节码反汇编功能

**验收标准**：
- [ ] 实现 `disassembleChunk()` 函数
- [ ] 实现 `disassembleInstruction()` 函数
- [ ] 支持所有指令类型的格式化输出
- [ ] 显示指令偏移、行号、操作码、操作数

**验证命令**：
```bash
./maple --debug-compile tests/scripts/simple.maple
# 预期输出：
# == <script> ==
# 0000    1 CONSTANT        0 '1.2'
# 0002    | RETURN
```

---

## 阶段4：虚拟机核心

### 任务4.1：基础VM实现（vm.hh/cc）
**目标**：实现栈式虚拟机基础框架

**验收标准**：
- [ ] 实现VM类
- [ ] 实现值栈（push, pop, peek）
- [ ] 实现CallFrame结构
- [ ] 实现 `interpret()` 入口
- [ ] 实现基础指令分发循环

**测试脚本** `tests/scripts/test_stack.maple`：
```maple
// 测试基础栈操作
print 1 + 2;
print 3.14;
print nil;
print true;
print false;
```

**预期输出**：
```
3
3.14
nil
true
false
```

---

### 任务4.2：字面量和栈操作指令
**目标**：实现字面量和栈操作指令

**指令清单**：
- [ ] OP_NIL
- [ ] OP_TRUE
- [ ] OP_FALSE
- [ ] OP_POP
- [ ] OP_DUP
- [ ] OP_CONSTANT / OP_CONSTANT_LONG

**测试脚本** `tests/scripts/test_literals.maple`：
```maple
print nil;
print true;
print false;
print 42;
print "hello";
```

---

### 任务4.3：算术运算指令
**目标**：实现算术运算指令

**指令清单**：
- [ ] OP_ADD（数字和字符串连接）
- [ ] OP_SUBTRACT
- [ ] OP_MULTIPLY
- [ ] OP_DIVIDE
- [ ] OP_MODULO
- [ ] OP_NEGATE
- [ ] OP_NOT

**测试脚本** `tests/scripts/test_arithmetic.maple`：
```maple
// 基础算术
print 1 + 2;
print 10 - 3;
print 4 * 5;
print 20 / 4;
print 17 % 5;
print -42;
print !true;
print !false;

// 字符串连接
print "Hello, " + "World!";

// 复杂表达式
print (1 + 2) * (3 + 4);
```

**预期输出**：
```
3
7
20
5
2
-42
false
true
Hello, World!
21
```

---

### 任务4.4：比较和逻辑指令
**目标**：实现比较和逻辑运算指令

**指令清单**：
- [ ] OP_EQUAL
- [ ] OP_GREATER
- [ ] OP_LESS
- [ ] OP_NOT

**测试脚本** `tests/scripts/test_comparison.maple`：
```maple
print 1 == 1;
print 1 == 2;
print 1 != 2;
print 3 > 2;
print 3 < 2;
print 3 >= 3;
print 3 <= 2;

// 短路求值
print true or false;
print false and true;
print nil or "default";
```

---

## 阶段5：词法分析器

### 任务5.1：词法分析器（scanner.hh/cc）
**目标**：实现完整的词法分析器

**验收标准**：
- [ ] 实现Scanner类
- [ ] 实现所有TokenType
- [ ] 识别单字符token（(){}[],.;+-*/%等）
- [ ] 识别双字符token（==, !=, <=, >=等）
- [ ] 识别关键字（if, else, while, for, fun, class等）
- [ ] 识别标识符、数字、字符串
- [ ] 支持行号跟踪
- [ ] 处理注释和空白

**Token类型清单**：
- [ ] 单字符: `(` `)` `{` `}` `[` `]` `,` `.` `-` `+` `;` `/` `*` `%`
- [ ] 双字符: `!` `!=` `=` `==` `>` `>=` `<` `<=`
- [ ] 关键字: `and` `class` `else` `false` `fun` `for` `if` `nil` `or` `print` `return` `super` `this` `true` `var` `while` `import` `from` `as`
- [ ] 字面量: `IDENTIFIER` `STRING` `NUMBER`
- [ ] 特殊: `ERROR` `EOF`

**测试脚本** `tests/scripts/test_scanner.maple`：
```maple
// 测试各种token
var x = 42;
var name = "Maple";
var pi = 3.14159;

// 运算符
if (x > 0 and x < 100) {
    print "in range";
}

// 关键字
fun greet(name) {
    return "Hello, " + name;
}
```

**验证命令**：
```bash
./maple --debug-scan tests/scripts/test_scanner.maple
```

---

## 阶段6：编译器基础

### 任务6.1：表达式编译
**目标**：实现表达式的编译（使用Pratt Parser）

**验收标准**：
- [ ] 实现Compiler类基础框架
- [ ] 实现ParseRule表
- [ ] 实现prefix/infix解析
- [ ] 支持优先级处理

**支持的表达式**：
- [ ] 字面量: number, string, true, false, nil
- [ ] 分组: `(expr)`
- [ ] 一元: `-expr`, `!expr`
- [ ] 二元: `+`, `-`, `*`, `/`, `%`
- [ ] 比较: `==`, `!=`, `<`, `>`, `<=`, `>=`
- [ ] 逻辑: `and`, `or`

**测试脚本** `tests/scripts/test_expr.maple`：
```maple
print 1 + 2 * 3;
print (1 + 2) * 3;
print -5 + +3;
print true and false or true;
print 1 < 2 == 3 < 4;
```

---

### 任务6.2：语句编译
**目标**：实现语句的编译

**验收标准**：
- [ ] 表达式语句
- [ ] print语句
- [ ] 变量声明
- [ ] 块语句

**测试脚本** `tests/scripts/test_stmt.maple`：
```maple
var x = 10;
var y = 20;
print x + y;

{
    var z = 30;
    print x + y + z;
}
```

---

### 任务6.3：变量系统
**目标**：实现局部变量和全局变量

**验收标准**：
- [ ] 全局变量声明和访问
- [ ] 局部变量声明和访问
- [ ] 变量赋值
- [ ] 局部变量作用域（块级）

**指令清单**：
- [ ] OP_GET_GLOBAL / OP_SET_GLOBAL / OP_DEFINE_GLOBAL
- [ ] OP_GET_LOCAL / OP_SET_LOCAL / OP_GET_LOCAL_LONG / OP_SET_LOCAL_LONG

**测试脚本** `tests/scripts/test_variables.maple`：
```maple
// 全局变量
var global_var = 100;
print global_var;

global_var = 200;
print global_var;

// 局部变量
{
    var local_var = 50;
    print local_var;
    print global_var;
}
// local_var 这里不可见
```

---

## 阶段7：控制流

### 任务7.1：条件语句
**目标**：实现if/else条件分支

**验收标准**：
- [ ] if语句编译和执行
- [ ] if/else语句
- [ ] else if链

**指令清单**：
- [ ] OP_JUMP
- [ ] OP_JUMP_IF_FALSE

**测试脚本** `tests/scripts/test_if.maple`：
```maple
var x = 10;

if (x > 5) {
    print "greater";
}

if (x < 5) {
    print "less";
} else {
    print "not less";
}

if (x < 5) {
    print "a";
} else if (x == 10) {
    print "b";
} else {
    print "c";
}
```

**预期输出**：
```
greater
not less
b
```

---

### 任务7.2：循环语句
**目标**：实现while和for循环

**验收标准**：
- [ ] while循环
- [ ] for循环（含初始化、条件、增量）
- [ ] break语句（可选）
- [ ] continue语句（可选）

**指令清单**：
- [ ] OP_LOOP

**测试脚本** `tests/scripts/test_loop.maple`：
```maple
// while循环
var i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}

// for循环
for (var j = 0; j < 3; j = j + 1) {
    print j * 10;
}

// 嵌套循环
for (var a = 1; a <= 2; a = a + 1) {
    for (var b = 1; b <= 2; b = b + 1) {
        print a * b;
    }
}
```

**预期输出**：
```
0
1
2
3
4
0
10
20
1
2
2
4
```

---

## 阶段8：函数系统

### 任务8.1：函数定义和调用
**目标**：实现函数声明和调用

**验收标准**：
- [ ] 函数声明（fun关键字）
- [ ] 函数调用
- [ ] 参数传递
- [ ] 返回值（return语句）
- [ ] 函数作为一等公民

**指令清单**：
- [ ] OP_CALL
- [ ] OP_RETURN
- [ ] OP_CLOSURE

**测试脚本** `tests/scripts/test_functions.maple`：
```maple
fun add(a, b) {
    return a + b;
}

fun greet(name) {
    print "Hello, " + name;
}

print add(3, 4);
greet("Maple");

// 函数作为值
var myfun = add;
print myfun(10, 20);

// 递归
fun factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

print factorial(5);
```

**预期输出**：
```
7
Hello, Maple
30
120
```

---

### 任务8.2：闭包支持
**目标**：实现闭包和Upvalue

**验收标准**：
- [ ] 捕获外部变量
- [ ] 修改外部变量
- [ ] 开放Upvalue处理
- [ ] 关闭Upvalue处理

**指令清单**：
- [ ] OP_GET_UPVALUE / OP_SET_UPVALUE
- [ ] OP_CLOSE_UPVALUE

**测试脚本** `tests/scripts/test_closure.maple`：
```maple
fun makeCounter() {
    var count = 0;
    fun counter() {
        count = count + 1;
        return count;
    }
    return counter;
}

var c = makeCounter();
print c();
print c();
print c();

// 多个闭包共享同一个upvalue
fun outer() {
    var x = 0;
    fun get() { return x; }
    fun set(v) { x = v; }
    return [get, set];
}

var accessors = outer();
var get = accessors[0];
var set = accessors[1];
print get();
set(42);
print get();
```

**预期输出**：
```
1
2
3
0
42
```

---

## 阶段9：面向对象

### 任务9.1：类和实例
**目标**：实现类和实例创建

**验收标准**：
- [ ] 类声明（class关键字）
- [ ] 实例创建
- [ ] 属性访问（get/set）
- [ ] 构造函数（init方法）

**指令清单**：
- [ ] OP_CLASS
- [ ] OP_GET_PROPERTY / OP_SET_PROPERTY
- [ ] OP_INVOKE

**测试脚本** `tests/scripts/test_class.maple`：
```maple
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        print "Hi, I'm " + this.name;
    }
}

var p = Person("Alice", 30);
print p.name;
print p.age;
p.greet();

p.age = 31;
print p.age;
```

**预期输出**：
```
Alice
30
Hi, I'm Alice
31
```

---

### 任务9.2：方法和继承
**目标**：实现类方法和继承

**验收标准**：
- [ ] 方法定义
- [ ] 方法调用（this绑定）
- [ ] 单继承（<语法）
- [ ] 方法重写
- [ ] super调用

**指令清单**：
- [ ] OP_METHOD
- [ ] OP_INHERIT
- [ ] OP_GET_SUPER
- [ ] OP_SUPER_INVOKE

**测试脚本** `tests/scripts/test_inheritance.maple`：
```maple
class Animal {
    init(name) {
        this.name = name;
    }
    
    speak() {
        print this.name + " makes a sound";
    }
}

class Dog < Animal {
    init(name, breed) {
        super.init(name);
        this.breed = breed;
    }
    
    speak() {
        print this.name + " barks";
    }
    
    describe() {
        print this.name + " is a " + this.breed;
    }
}

var d = Dog("Buddy", "Golden Retriever");
d.speak();
d.describe();
```

**预期输出**：
```
Buddy barks
Buddy is a Golden Retriever
```

---

## 阶段10：垃圾回收

### 任务10.1：Mark-Sweep GC
**目标**：实现完整的垃圾回收器

**验收标准**：
- [ ] 实现GC类
- [ ] 实现标记阶段（mark roots, trace references）
- [ ] 实现清理阶段（sweep）
- [ ] 处理字符串驻留池
- [ ] 支持GC压力测试模式

**测试脚本** `tests/scripts/test_gc.maple`：
```maple
// 创建大量临时对象触发GC
for (var i = 0; i < 1000; i = i + 1) {
    var s = "string" + i;
}
print "GC test passed";

// 闭包中的循环引用
fun makeClosure() {
    var x = "outer";
    fun inner() {
        print x;
    }
    return inner;
}

for (var i = 0; i < 100; i = i + 1) {
    var f = makeClosure();
}
print "Closure GC test passed";
```

**验证命令**：
```bash
# 压力测试模式（每次分配都触发GC）
./maple --stress-gc tests/scripts/test_gc.maple
```

---

## 阶段11：Import系统

### 任务11.1：模块导入系统
**目标**：实现模块化导入系统

**验收标准**：
- [ ] import "module" 语法
- [ ] from "module" import name 语法
- [ ] import "module" as alias 语法
- [ ] from "module" import name as alias 语法
- [ ] 模块缓存（只加载一次）
- [ ] 模块搜索路径

**指令清单**：
- [ ] OP_IMPORT
- [ ] OP_IMPORT_FROM
- [ ] OP_IMPORT_AS

**测试文件结构**：
```
tests/scripts/
├── test_import.maple
└── modules/
    ├── math.maple
    └── utils.maple
```

**测试脚本** `tests/scripts/modules/math.maple`：
```maple
// 标准数学模块
fun add(a, b) { return a + b; }
fun mul(a, b) { return a * b; }
var PI = 3.14159;
```

**测试脚本** `tests/scripts/test_import.maple`：
```maple
// 完整导入
import "modules/math";
print math.add(2, 3);
print math.PI;

// 选择性导入
from "modules/math" import mul;
print mul(4, 5);

// 别名导入
import "modules/math" as m;
print m.PI;

from "modules/math" import add as plus;
print plus(10, 20);
```

**预期输出**：
```
5
3.14159
20
3.14159
30
```

---

## 阶段12：标准库

### 任务12.1：核心原生函数
**目标**：实现核心原生函数

**验收标准**：
- [ ] clock() - 返回当前时间（秒）
- [ ] 实现原生函数注册机制

**测试脚本** `tests/scripts/test_native.maple`：
```maple
var start = clock();

// 执行一些操作
var sum = 0;
for (var i = 0; i < 1000000; i = i + 1) {
    sum = sum + i;
}

var end = clock();
print "Time elapsed:";
print end - start;
print "Sum:";
print sum;
```

---

### 任务12.2：Math模块
**目标**：实现数学标准库

**验收标准**：
- [ ] sin, cos, tan
- [ ] asin, acos, atan, atan2
- [ ] sqrt, pow, exp, log
- [ ] ceil, floor, round
- [ ] abs
- [ ] 常量PI, E

**测试脚本** `tests/scripts/test_math.maple`：
```maple
import "std/math";

print math.sqrt(16);
print math.floor(3.14);
print math.ceil(3.14);
print math.abs(-42);
print math.PI;

// 三角函数
print math.sin(0);
print math.cos(0);
```

---

### 任务12.3：IO模块
**目标**：实现IO标准库

**验收标准**：
- [ ] 扩展print支持多参数
- [ ] readFile(path)
- [ ] writeFile(path, content)

**测试脚本** `tests/scripts/test_io.maple`：
```maple
import "std/io";

// 多参数print
io.print("Hello", "World", 123);

// 文件操作
io.writeFile("test_output.txt", "Hello from Maple!");
var content = io.readFile("test_output.txt");
print content;
```

---

## 阶段13：集合类型（可选）

### 任务13.1：列表支持
**目标**：实现列表类型

**验收标准**：
- [ ] 列表字面量 [1, 2, 3]
- [ ] 索引访问 list[0]
- [ ] 索引赋值 list[0] = 10
- [ ] 列表长度

**指令清单**：
- [ ] OP_BUILD_LIST
- [ ] OP_INDEX_GET
- [ ] OP_INDEX_SET

**测试脚本** `tests/scripts/test_list.maple`：
```maple
var list = [1, 2, 3, 4, 5];
print list;
print list[0];
print list[4];

list[0] = 100;
print list[0];

// 嵌套列表
var matrix = [[1, 2], [3, 4]];
print matrix[0][1];
```

---

### 任务13.2：映射支持（可选）
**目标**：实现映射/字典类型

**验收标准**：
- [ ] 映射字面量 {"a": 1, "b": 2}
- [ ] 键值访问 map["key"]
- [ ] 键值赋值 map["key"] = value

**指令清单**：
- [ ] OP_BUILD_MAP

**测试脚本** `tests/scripts/test_map.maple`：
```maple
var person = {
    "name": "Alice",
    "age": 30
};
print person["name"];
person["age"] = 31;
print person["age"];
```

---

## 附录：任务状态总览

### 已完成任务
| 阶段 | 任务 | 状态 |
|------|------|------|
| 1 | 1.1 项目构建系统 | ✅ |
| 1 | 1.2 基础定义模块 | ✅ |
| 1 | 1.3 日志系统 | ✅ |
| 2 | 2.1 动态数组 | ✅ |
| 2 | 2.2 值类型系统 | ✅ |
| 2 | 2.3 内存管理基础 | ✅ |
| 2 | 2.4 对象系统基础 | ✅ |
| 3 | 3.1 字节码块 | ✅ |
| 3 | 3.2 反汇编调试器 | ✅ |
| 4 | 4.1 基础VM实现 | ✅ |
| 4 | 4.2 字面量和栈操作 | ✅ |
| 4 | 4.3 算术运算 | ✅ |
| 4 | 4.4 比较和逻辑 | ✅ |
| 5 | 5.1 词法分析器 | ✅ |
| 6 | 6.1 表达式编译 | ✅ |
| 6 | 6.2 语句编译 | ✅ |
| 6 | 6.3 变量系统 | ✅ |
| 7 | 7.1 条件语句 | ✅ |
| 7 | 7.2 循环语句 | ✅ |
| 8 | 8.1 函数定义和调用 | ✅ |
| 8 | 8.2 闭包支持 | ✅ |
| 9 | 9.1 类和实例 | ✅ |
| 9 | 9.2 方法和继承 | ✅ |
| 10 | 10.1 Mark-Sweep GC | ✅ |
| 11 | 11.1 模块导入系统 | ✅ |
| 12 | 12.1 核心原生函数 | ✅ |
| 12 | 12.2 Math模块 | ✅ |
| 12 | 12.3 IO模块 | ✅ |
| 13 | 13.1 列表支持 | ✅ |
| 13 | 13.2 映射支持 | ✅ |

### 任务依赖图
```
1.1 构建系统 ───────────────────────────────────────────┐
1.2 基础定义 ───┬──→ 1.3 日志系统 ───┬──→ 2.3 内存管理 ──┤
2.1 动态数组 ───┘                     │                   │
2.2 值类型 ─────┬─────────────────────┤                   │
2.4 对象系统 ───┘                     ↓                   ↓
                                 3.1 Chunk ───→ 3.2 Debug
                                      ↓
                                 4.1 基础VM
                                      ↓
    ┌─────────────────────────────────┼─────────────────────────────────┐
    ↓                                 ↓                                 ↓
4.2 字面量/栈 ───→ 4.3 算术 ───→ 4.4 比较/逻辑                    5.1 Scanner
                                                        ↓
                                                   6.1 表达式
                                                        ↓
    ┌─────────────────────────────────┬─────────────────────────────────┐
    ↓                                 ↓                                 ↓
 6.2 语句                        6.3 变量系统                      7.1 条件/7.2 循环
    │                                 │                                 │
    └─────────────────────────────────┼─────────────────────────────────┘
                                      ↓
                                 8.1 函数调用
                                      ↓
                                 8.2 闭包
                                      ↓
    ┌─────────────────────────────────┴─────────────────────────────────┐
    ↓                                                                   ↓
 9.1 类和实例 ─────────────────────────────────────────────────→ 9.2 继承
                                      ↓
                                 10.1 GC
                                      ↓
    ┌─────────────────────────────────┴─────────────────────────────────┐
    ↓                                                                   ↓
 11.1 Import系统 ─────────────────────────────────────────────────→ 12.x 标准库
```

## 更新记录

- **初始版本** - 将DESIGN.md拆分为可独立验证的任务
