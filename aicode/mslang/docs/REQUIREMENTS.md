# Maple Language Requirements Specification

> Version: 1.0 | Date: 2026-03-01 | Author: ASMlover

---

## 1. Overview

Maple 是一门动态类型、面向对象的脚本语言，基于 [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) 中 clox 的架构设计，使用现代 C++23 实现。核心运行时包含：字节码编译器、栈式虚拟机（Stack-based VM）和 Mark-and-Sweep 垃圾回收器。

### 1.1 Design Goals

| 目标 | 描述 |
|------|------|
| **简洁** | 语法精炼，学习曲线平缓，参考 Lox 语法 |
| **高效** | 字节码编译 + VM 执行，避免 AST 遍历开销 |
| **安全** | 自动垃圾回收，无手动内存管理 |
| **可扩展** | 模块系统支持 `import` / `from ... import ... as ...` |
| **跨平台** | 支持 Windows (MSVC) 和 Linux (GCC) |
| **可调试** | 内置彩色日志系统，支持字节码反汇编 |

### 1.2 Non-Goals

- 不实现 JIT 编译
- 不实现多线程/并发原语
- 不实现异常处理（try/catch）
- 不实现标准库（仅提供 `clock()` 原生函数）

---

## 2. Lexical Grammar

### 2.1 Tokens

#### 单字符标记
```
(  )  {  }  ,  .  -  +  ;  /  *
```

#### 一到两字符标记
```
!  !=  =  ==  >  >=  <  <=
```

#### 字面量
```
IDENTIFIER    ::= ALPHA ( ALPHA | DIGIT )*
STRING        ::= '"' <any char except '"'>* '"'
NUMBER        ::= DIGIT+ ( '.' DIGIT+ )?
```

其中：
```
ALPHA  ::= 'a' ... 'z' | 'A' ... 'Z' | '_'
DIGIT  ::= '0' ... '9'
```

#### 关键字
```
and    class   else   false   for    fun    if     import
from   as      nil    or      print  return super  this
true   var     while
```

> `from` 和 `as` 是相比 clox 新增的关键字，用于模块导入。

#### 注释
```
// 单行注释，直到行末
```

### 2.2 Whitespace

空格、制表符、回车（`\r`）和换行（`\n`）均为空白字符，仅用于分隔 token，不产生任何语义效果。

---

## 3. Syntax Grammar

### 3.1 Declarations

```ebnf
TERM           → ";" | <<ASI>> ;   // explicit or scanner-inserted semicolon

program        → declaration* EOF ;

declaration    → classDecl
               | funDecl
               | varDecl
               | importDecl
               | statement ;

classDecl      → "class" IDENTIFIER ( ":" IDENTIFIER )? "{" function* "}" ;
funDecl        → "fun" IDENTIFIER function ;
varDecl        → "var" IDENTIFIER ( "=" expression )? TERM ;

importDecl     → "import" STRING TERM
               | "from" STRING "import" IDENTIFIER ( "as" IDENTIFIER )? TERM ;
```

### 3.2 Statements

```ebnf
statement      → exprStmt
               | forStmt
               | ifStmt
               | printStmt
               | returnStmt
               | whileStmt
               | block ;

exprStmt       → expression TERM ;
forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
                 expression? ";" expression? ")" statement ;
ifStmt         → "if" "(" expression ")" statement
                 ( "else" statement )? ;
printStmt      → "print" expression TERM ;
returnStmt     → "return" expression? TERM ;
whileStmt      → "while" "(" expression ")" statement ;
block          → "{" declaration* "}" ;
```

### 3.3 Expressions

```ebnf
expression     → assignment ;

assignment     → ( call "." )? IDENTIFIER "=" assignment
               | logic_or ;

logic_or       → logic_and ( "or" logic_and )* ;
logic_and      → equality ( "and" equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary | call ;
call           → primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
primary        → "true" | "false" | "nil" | "this"
               | NUMBER | STRING | IDENTIFIER
               | "(" expression ")"
               | "super" "." IDENTIFIER ;

function       → "(" parameters? ")" block ;
parameters     → IDENTIFIER ( "," IDENTIFIER )* ;
arguments      → expression ( "," expression )* ;
```

### 3.4 Operator Precedence (Low → High)

| 优先级 | 运算符 | 结合性 |
|--------|--------|--------|
| 1 | `=` | 右结合 |
| 2 | `or` | 左结合 |
| 3 | `and` | 左结合 |
| 4 | `==` `!=` | 左结合 |
| 5 | `<` `>` `<=` `>=` | 左结合 |
| 6 | `+` `-` | 左结合 |
| 7 | `*` `/` | 左结合 |
| 8 | `!` `-`(unary) | 右结合 |
| 9 | `.` `()` | 左结合 |

---

## 4. Type System

Maple 是**动态类型**语言。值的类型在运行时确定。

### 4.1 Primitive Types

| 类型 | 描述 | 字面量示例 |
|------|------|-----------|
| `nil` | 空值 | `nil` |
| `boolean` | 布尔值 | `true`, `false` |
| `number` | 64位双精度浮点数 | `42`, `3.14` |

### 4.2 Object Types (Heap-allocated, GC-managed)

| 类型 | 描述 |
|------|------|
| `string` | 不可变字符串，支持驻留（interning） |
| `function` | 用户自定义函数 |
| `native` | 原生 C++ 函数 |
| `closure` | 函数 + 捕获的 upvalue |
| `class` | 类定义 |
| `instance` | 类实例 |
| `bound_method` | 绑定到接收者的方法 |
| `module` | 导入的模块 |

### 4.3 Truthiness

- `false` 和 `nil` 是假值（falsey）
- **其他所有值均为真值（truthy）**，包括 `0`、空字符串 `""`

### 4.4 Equality

- `nil == nil` → `true`
- `true == true` → `true`, `true == false` → `false`
- 数字：按值比较
- 字符串：按内容比较（由于字符串驻留，实际为指针比较）
- 不同类型之间：永远不相等

---

## 5. Semantics

### 5.1 Variables

```maple
var x = 10;      // 声明并初始化
var y;           // 声明，默认为 nil
x = 20;         // 赋值
print x;        // 输出: 20
```

- 变量必须先声明后使用
- 未初始化的变量默认为 `nil`
- 全局变量存储在哈希表中，局部变量存储在 VM 栈上

### 5.2 Scoping

```maple
var a = "global";
{
  var a = "local";
  print a;        // 输出: local
}
print a;          // 输出: global
```

- 块作用域 `{}` 创建新的作用域层级
- 内部作用域可以遮蔽外部同名变量
- 变量在声明它的作用域结束时被销毁

### 5.3 Control Flow

```maple
// if-else
if (condition) {
  // ...
} else {
  // ...
}

// while loop
while (condition) {
  // ...
}

// for loop
for (var i = 0; i < 10; i = i + 1) {
  print i;
}
```

- `for` 循环是 `while` 循环的语法糖
- 不支持 `break` / `continue`（与 clox 一致）

### 5.4 Functions

```maple
fun greet(name) {
  print "Hello, " + name + "!";
}

greet("World");   // 输出: Hello, World!

fun fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

print fib(10);    // 输出: 55
```

- 函数是一等公民（first-class）
- 支持递归
- 参数按值传递（对象通过引用传递，因为 Value 持有 Object*）
- `return` 无值时返回 `nil`

### 5.5 Closures

```maple
fun makeCounter() {
  var count = 0;
  fun increment() {
    count = count + 1;
    return count;
  }
  return increment;
}

var counter = makeCounter();
print counter();  // 输出: 1
print counter();  // 输出: 2
```

- 内部函数可以捕获外部函数的局部变量
- 被捕获的变量（upvalue）在外部函数返回后仍然有效
- 通过 upvalue 的 open/closed 状态切换实现

### 5.6 Classes

```maple
class Animal {
  init(name) {
    this.name = name;
  }

  speak() {
    print this.name + " makes a sound.";
  }
}

var a = Animal("Cat");
a.speak();        // 输出: Cat makes a sound.
```

- `init()` 是构造函数，在实例化时自动调用
- `this` 引用当前实例
- 实例可以自由添加字段：`a.age = 3;`
- 方法查找：先查实例字段，再查类方法

### 5.7 Inheritance

```maple
class Dog : Animal {
  speak() {
    print this.name + " barks!";
  }
}

class Puppy : Dog {
  speak() {
    super.speak();
    print "...with a tiny voice.";
  }
}

var p = Puppy("Buddy");
p.speak();
// 输出:
// Buddy barks!
// ...with a tiny voice.
```

- 单继承，使用 `:` 语法
- `super.method()` 调用父类方法
- 子类继承父类所有方法（编译时复制到子类方法表）
- 子类可以覆盖父类方法

### 5.8 String Operations

```maple
var s = "Hello" + ", " + "World!";
print s;          // 输出: Hello, World!
```

- `+` 运算符支持字符串拼接
- 字符串不可变
- 所有字符串自动驻留（interned），相同内容的字符串共享同一对象

### 5.9 Native Functions

| 函数 | 签名 | 描述 |
|------|------|------|
| `clock()` | `() → number` | 返回自程序启动以来的秒数（浮点数） |

---

## 6. Import System

### 6.1 Import Statement

```maple
import "path/to/module.ms";
```

**语义**：
1. 解析模块文件路径（相对于当前脚本所在目录）
2. 编译并执行模块脚本
3. 模块的顶层 `var` 和 `fun` 声明作为导出
4. 将模块对象绑定到以模块文件名（不含路径和扩展名）命名的全局变量

```maple
import "math.ms";
print math.add(1, 2);    // 通过模块名访问
```

### 6.2 From-Import Statement

```maple
from "math.ms" import add;
print add(1, 2);          // 直接访问

from "math.ms" import add as plus;
print plus(1, 2);         // 使用别名
```

**语义**：
1. 加载模块（如已缓存则复用）
2. 从模块导出中提取指定名称
3. 将其绑定到当前作用域（使用原名或别名）

### 6.3 Module Caching

- 每个模块路径只编译执行一次
- 后续 import 同一路径返回缓存的模块对象
- 循环导入检测：导入进行中的模块会报编译错误

### 6.4 Module Scope

- 模块内的代码在独立的全局作用域中执行
- 模块的 `var` 和 `fun` 声明构成导出集
- 模块之间互相隔离

---

## 7. Runtime Behavior

### 7.1 Execution Modes

| 模式 | 启动方式 | 描述 |
|------|---------|------|
| REPL | `mslang` | 交互式读取-求值-打印循环 |
| File | `mslang script.ms` | 执行脚本文件 |

### 7.2 Exit Codes

| 代码 | 含义 |
|------|------|
| 0 | 成功 |
| 64 | 用法错误（参数错误） |
| 65 | 编译错误 |
| 70 | 运行时错误 |
| 74 | I/O 错误（文件不可读） |

### 7.3 Error Reporting

- 编译错误：`[line N] Error at 'token': message`
- 运行时错误：`message\n[line N] in function_name`
- 运行时错误包含调用栈回溯

### 7.4 Garbage Collection

- **算法**：Mark-and-Sweep（三色标记）
- **触发条件**：当 `bytesAllocated > nextGC` 时触发
- **增长因子**：`nextGC = bytesAllocated * 2`
- **Roots**：VM 栈、全局变量表、调用帧、打开的 upvalue 链、编译器引用
- **String Table 清理**：Sweep 阶段移除不可达的驻留字符串

---

## 8. Debug & Logging

### 8.1 Bytecode Disassembly

通过编译选项 `MAPLE_DEBUG_PRINT_CODE` 启用，编译后输出反汇编结果：

```
== <script> ==
0000    1 OP_CONSTANT         0 '1'
0002    | OP_CONSTANT         1 '2'
0004    | OP_ADD
0005    | OP_PRINT
0006    2 OP_RETURN
```

### 8.2 Execution Tracing

通过编译选项 `MAPLE_DEBUG_TRACE_EXECUTION` 启用，运行时输出每条指令及栈状态：

```
          [ 1 ][ 2 ]
0004    | OP_ADD
          [ 3 ]
0005    | OP_PRINT
3
```

### 8.3 Colored Logger

内置日志系统，支持 5 个级别：

| 级别 | 颜色 | 用途 |
|------|------|------|
| TRACE | 灰色 | 详细跟踪信息 |
| DEBUG | 青色 | 调试信息 |
| INFO | 绿色 | 一般信息 |
| WARN | 黄色 | 警告信息 |
| ERROR | 红色 | 错误信息 |

输出格式：`[LEVEL] [tag] message`

### 8.4 GC Stress Testing

通过编译选项 `MAPLE_DEBUG_STRESS_GC` 启用，每次对象分配都触发 GC，用于验证 GC 正确性。

---

## 9. Build Requirements

| 项目 | 要求 |
|------|------|
| C++ 标准 | C++23 (`/std:c++latest` on MSVC, `-std=c++2b` on GCC) |
| CMake | >= 3.22 |
| Windows 编译器 | MSVC (Visual Studio 2022+) |
| Linux 编译器 | GCC 13+ |
| 第三方依赖 | 无 |
| 文件扩展名 | 头文件 `.hh`，实现文件 `.cc` |
| 脚本扩展名 | `.ms` |
| 命名空间 | `ms` |
| 头文件保护 | `#pragma once` |

---

## 10. Test Requirements

### 10.1 Test Script Format

使用 `// expect: value` 注释标记期望输出：

```maple
print 1 + 2;        // expect: 3
print "hello";      // expect: hello
```

### 10.2 Test Coverage

| 测试文件 | 覆盖范围 |
|---------|---------|
| `arithmetic.ms` | 四则运算、取负、优先级、浮点数、比较、布尔运算 |
| `variables.ms` | 全局/局部变量声明、赋值、作用域、遮蔽 |
| `control_flow.ms` | if/else、while、for、逻辑运算符、短路求值 |
| `functions.ms` | 函数定义、调用、递归(fibonacci)、嵌套函数、高阶函数、返回nil |
| `closures.ms` | 闭包捕获、makeCounter模式、共享upvalue、嵌套闭包、独立计数器 |
| `classes.ms` | 类声明、实例化、字段、方法、this、init构造器、继承(:)、super、方法覆盖、绑定方法 |
| `strings.ms` | 字符串拼接、字面量、比较、空字符串 |
| `import_test.ms` | import、from-import、from-import-as |
| `errors/*.ms` | 运行时错误(未定义变量、参数数量、类型错误、非实例属性、调用非函数、继承非类) |

### 10.3 Test Runner

提供 `tests/run_tests.py` 脚本：
- 解析 `// expect:` 注释匹配 stdout 输出
- 解析 `// expect runtime error:` 注释匹配 stderr 错误信息
- 自动发现 `tests/*.ms` 和 `tests/errors/*.ms`
- 运行每个 `.ms` 文件并比较实际输出与期望输出
- 报告通过/失败/跳过统计
