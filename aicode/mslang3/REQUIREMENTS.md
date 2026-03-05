# Maple 脚本语言需求文档

## 1. 项目概述

### 1.1 项目名称
Maple - 基于字节码的高性能脚本语言

### 1.2 项目目标
实现一个功能完整、高性能的脚本语言，包含：
- 字节码虚拟机
- 自动垃圾回收
- 模块化导入系统
- 现代化的调试工具

### 1.3 参考实现
基于 Crafting Interpreters 中的 clox（不是 jlox）

## 2. 技术需求

### 2.1 开发环境
- **语言**: C++23 及更新标准
- **构建工具**: CMake 3.20+
- **平台支持**:
  - Windows (MSVC 2022+)
  - Linux (GCC 13+, Clang 16+)

### 2.2 文件命名规范
- 头文件: `.hh` 扩展名
- 实现文件: `.cc` 扩展名
- 命名空间: `namespace ms`

### 2.3 头文件规范
- 使用 `#pragma once` 代替传统的 `#ifndef/#define/#endif` 保护
- 尽量避免宏定义

## 3. 语言特性需求

### 3.1 数据类型
- [x] 基础类型: `nil`, `bool`, `number` (double)
- [x] 字符串类型
- [x] 函数类型（一等公民）
- [x] 闭包支持
- [x] 类与对象（面向对象）
- [x] 继承（单继承）
- [x] 模块/导入系统

### 3.2 表达式
- [x] 算术表达式: `+`, `-`, `*`, `/`, `%` (取模)
- [x] 比较表达式: `==`, `!=`, `<`, `>`, `<=`, `>=`
- [x] 逻辑表达式: `and`, `or`, `!`
- [x] 赋值表达式: `=`, 复合赋值
- [x] 函数调用
- [x] 属性访问: `obj.property`
- [x] 方法调用: `obj.method()`
- [x] `this` 表达式
- [x] `super` 表达式

### 3.3 语句
- [x] 变量声明: `var`
- [x] 函数声明: `fun`
- [x] 类声明: `class`
- [x] 条件: `if/else`
- [x] 循环: `while`, `for`
- [x] 返回: `return`
- [x] 打印: `print`
- [x] 表达式语句

### 3.4 控制流
- [x] `if/else` 条件分支
- [x] `while` 循环
- [x] `for` 循环
- [x] `return` 返回值
- [x] 短路求值 (`and`/`or`)

## 4. Import系统需求

### 4.1 基本导入
```maple
import "math"
```
- 导入整个模块
- 通过模块名访问成员: `math.sin(3.14)`

### 4.2 选择性导入
```maple
from "math" import sin, cos
```
- 导入指定的标识符到当前命名空间
- 直接使用: `sin(3.14)`

### 4.3 别名导入
```maple
import "io" as filesystem
from "math" import sin as sine
```
- 支持模块别名
- 支持标识符别名

### 4.4 模块搜索
1. 当前目录 `./`
2. 环境变量 `MAPLE_PATH`
3. 标准库目录 `MAPLE_HOME/lib/`
4. 内置模块

### 4.5 模块缓存
- 模块只加载一次
- 重复导入返回缓存的模块对象
- 模块级变量保持状态

## 5. 调试与日志系统需求

### 5.1 日志等级
| 等级 | 颜色 | 用途 |
|------|------|------|
| Trace | Gray | 最详细的执行跟踪 |
| Debug | Cyan | 调试信息 |
| Info | Green | 一般信息 |
| Warn | Yellow | 警告 |
| Error | Red | 错误 |
| Fatal | Magenta | 致命错误 |

### 5.2 颜色输出
- 支持 ANSI 颜色代码
- Windows 平台自动启用 ANSI 支持
- 可通过环境变量禁用颜色

### 5.3 编译期调试开关
```cpp
#define DEBUG_SCANNER       // 词法分析跟踪
#define DEBUG_PARSER        // 语法分析跟踪  
#define DEBUG_COMPILE       // 编译输出
#define DEBUG_TRACE_EXEC    // 执行跟踪
#define DEBUG_STRESS_GC     // GC压力测试
#define DEBUG_LOG_GC        // GC详细日志
```

### 5.4 运行期日志API（未来扩展）
```maple
log.debug("value = {}", x)
log.info("Starting...")
log.error("Failed: {}", err)
```

## 6. 性能需求

### 6.1 执行性能
- 比 jlox 快至少 10 倍
- 使用 NaN Boxing 优化值表示
- 使用 Copy-down 继承优化方法调用

### 6.2 内存性能
- 自动垃圾回收
- 支持 GC 压力测试模式
- 字符串驻留池去重

### 6.3 启动性能
- 快速启动（< 100ms）
- 模块懒加载

## 7. 标准库需求

### 7.1 核心模块
- [x] `math` - 数学函数
- [x] `io` - 输入输出
- [x] `sys` - 系统功能
- [x] `time` - 时间函数

### 7.2 math模块
- `sin(x)`, `cos(x)`, `tan(x)`
- `asin(x)`, `acos(x)`, `atan(x)`, `atan2(y, x)`
- `sqrt(x)`, `pow(x, y)`, `exp(x)`, `log(x)`
- `ceil(x)`, `floor(x)`, `round(x)`
- `abs(x)`
- 常量: `PI`, `E`

### 7.3 io模块
- `print(...)`
- `readFile(path)`
- `writeFile(path, content)`

### 7.4 time模块
- `clock()` - 返回秒数
- `sleep(seconds)`

## 8. 测试需求

### 8.1 测试目录结构
```
tests/
├── unit/           # 单元测试
├── integration/    # 集成测试
├── scripts/        # 测试脚本
└── run_tests.py    # 测试运行器
```

### 8.2 测试覆盖
- [x] 词法分析器测试
- [x] 编译器测试
- [x] VM执行测试
- [x] GC测试
- [x] Import系统测试

### 8.3 测试脚本示例
```maple
// test_arithmetic.maple
assert(1 + 2 == 3)
assert(10 - 3 == 7)
assert(4 * 5 == 20)
assert(15 / 3 == 5)
assert(17 % 5 == 2)
print "Arithmetic tests passed!"
```

## 9. 文档需求

### 9.1 代码文档
- 所有公共API使用 Doxygen 风格注释
- 关键算法解释性注释

### 9.2 用户文档
- 语言语法参考
- 标准库文档
- 示例程序

## 10. 构建需求

### 10.1 CMake配置
```cmake
# 最小版本要求
cmake_minimum_required(VERSION 3.20)

# 项目信息
project(Maple VERSION 0.1.0 LANGUAGES CXX)

# C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建选项
option(ENABLE_DEBUG "Enable debug mode" OFF)
option(ENABLE_STRESS_GC "Enable GC stress test" OFF)
```

### 10.2 构建目标
- `maple` - 主可执行文件
- `maple_test` - 测试可执行文件

### 10.3 安装
```bash
# 标准安装路径
PREFIX/bin/maple
PREFIX/lib/maple/
PREFIX/share/maple/
```

## 11. 兼容性需求

### 11.1 源代码兼容性
- 符合 C++23 标准
- 不使用编译器特定扩展（除非必要）

### 11.2 平台兼容性
- Windows: MSVC 2022+
- Linux: GCC 13+, Clang 16+
- macOS: Clang 16+ (未来支持)

## 12. 代码质量需求

### 12.1 代码规范
- 使用现代C++特性
- 避免裸指针，使用RAII
- 明确的const正确性
- 移动语义优化

### 12.2 静态分析
- 无编译器警告（-Wall -Wextra -Werror）
- 支持静态分析工具

## 13. 开发计划

### Phase 1: 核心基础设施
- [x] 项目结构和构建系统
- [x] 基础数据类型（Value, Object）
- [x] 内存管理系统
- [x] Logger系统

### Phase 2: VM核心
- [x] 字节码块（Chunk）
- [x] 栈式虚拟机
- [x] 基础指令集
- [x] 调试工具

### Phase 3: 编译器
- [x] 词法分析器（Scanner）
- [x] 编译器（Compiler）
- [x] 表达式解析
- [x] 语句编译

### Phase 4: 高级特性
- [x] 局部变量
- [x] 跳转指令
- [x] 函数调用
- [x] 闭包和Upvalue

### Phase 5: 面向对象
- [x] 类和实例
- [x] 方法调用
- [x] 继承
- [x] super调用

### Phase 6: 扩展功能
- [x] 垃圾回收器
- [x] Import系统
- [x] 标准库
- [x] 完整测试套件
