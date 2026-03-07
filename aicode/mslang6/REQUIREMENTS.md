# Maple 脚本语言 — 需求方案

## 1. 项目目标

实现一门参考 [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) 中 **clox**（C 语言字节码实现）的脚本语言 **Maple**，使用 C++23 多文件、跨平台实现，并扩展模块导入与调试日志能力。

## 2. 功能需求

### 2.1 clox 已有功能（必须实现）

- **词法分析**：完整 Token 集合（关键字、标识符、字面量、运算符等）。
- **编译**：单遍递归下降，生成字节码 Chunk（无独立 AST）。
- **虚拟机**：基于栈的字节码执行、调用栈与 CallFrame、局部/全局变量、闭包与 upvalue。
- **值类型**：nil、bool、number（double）、string；对象类型：String、Function、Closure、Class、Instance、BoundMethod、Native、Upvalue。
- **控制流**：if/else、while、for；逻辑短路 and/or。
- **函数**：函数字面量、调用、返回值；原生函数（如 `clock()`）。
- **面向对象**：class、实例、方法、构造器 init、继承、super、this。
- **内存**：统一分配入口、对象链表、**标记-清扫垃圾回收**。

### 2.2 扩展功能

- **import 机制**：  
  - `import mod`：加载模块并以名称 `mod` 绑定到当前作用域。  
  - `from mod import a, b`：从模块 `mod` 导入 `a`、`b` 到当前作用域。  
  - `from mod import x as y`：从模块 `mod` 导入 `x` 并绑定为 `y`。

### 2.3 工具与调试

- **内部 Logger**：  
  - 多级别：如 DEBUG、INFO、WARN、ERROR。  
  - 按级别使用不同颜色输出（需兼容 Windows 与 Linux 终端）。

## 3. 非功能需求

| 项目     | 要求 |
|----------|------|
| 语言标准 | C++23 及以上 |
| 文件规范 | 头文件 `.hh`，实现 `.cc`；头文件使用 `#pragma once` |
| 代码组织 | 多文件实现，避免单文件集中 |
| 依赖     | 尽量使用标准库，尽量少用第三方库 |
| 平台     | Windows（MSVC）、Linux（GCC） |
| 构建     | 使用 CMake 作为跨平台构建工具 |
| 命名     | 语言名 Maple，命名空间 `ms` |
| 测试     | 在单独测试目录中提供详细测试脚本（.ms）及说明 |

## 4. 交付物

- **需求方案**：本文档（REQUIREMENTS.md）。
- **设计实现方案**：见 [PLAN.md](PLAN.md)，包含实现架构、模块划分、接口与实现顺序。
- **测试**：`tests/` 目录下的 Maple 测试脚本及运行说明。

---

实现架构与详细设计见 **PLAN.md**。
