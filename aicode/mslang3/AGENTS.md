# Maple 脚本语言项目 - AGENTS.md

## 项目概述

Maple是一个基于字节码的高性能脚本语言，参考自《Crafting Interpreters》中的clox实现，使用C++23标准进行现代化重构。

> **当前AI助手**: Kimi-K2.5

### 项目目标
- 实现功能完整的字节码虚拟机
- 自动内存管理（Mark-Sweep GC）
- 模块化import系统
- 现代化的调试工具

## 项目结构

```
maple/
├── CMakeLists.txt          # CMake构建配置
├── AGENTS.md               # 本文件 - 项目指南
├── DESIGN.md               # 设计文档 - 架构和技术细节
├── PLAN.md                 # 实施计划 - 可运行验证的任务
├── REQUIREMENTS.md         # 需求文档 - 功能和性能需求
├── src/                    # 源代码目录
│   ├── common.hh           # 基础定义、配置常量
│   ├── memory.hh/cc        # 内存管理、GC
│   ├── value.hh/cc         # 值类型系统（NaN Boxing）
│   ├── object.hh/cc        # 对象系统
│   ├── chunk.hh/cc         # 字节码块
│   ├── debug.hh/cc         # 调试工具
│   ├── logger.hh/cc        # 日志系统
│   ├── scanner.hh/cc       # 词法分析器
│   ├── compiler.hh/cc      # 编译器
│   ├── vm.hh/cc            # 虚拟机
│   ├── import.hh/cc        # 模块导入系统
│   └── main.cc             # 程序入口
├── lib/                    # 标准库
│   └── std/
│       ├── math.maple      # 数学模块
│       ├── io.maple        # IO模块
│       └── sys.maple       # 系统模块
└── tests/                  # 测试目录
    ├── unit/               # 单元测试
    ├── integration/        # 集成测试
    ├── scripts/            # 测试脚本(.maple)
    └── run_tests.py        # 测试运行器
```

## 编码规范

### 文件命名
- 头文件：`.hh` 扩展名
- 实现文件：`.cc` 扩展名
- 命名空间：`namespace ms`

### 头文件规范
- 使用 `#pragma once` 代替传统的 `#ifndef/#define/#endif`
- 尽量避免宏定义，优先使用 `constexpr`

### 命名约定
- 类型名（类/结构体/枚举）：`PascalCase`
- 函数名：`camelCase`
- 私有成员变量：`snake_case_`（尾部下划线）
- 常量/枚举值：`UPPER_SNAKE_CASE`
- 文件名：`snake_case.cc`

### C++23特性使用
- 优先使用 `std::expected` 进行错误处理
- 使用 `std::optional` 表示可选值
- 使用 `std::string_view` 进行零拷贝字符串引用
- 使用 `constexpr`/`consteval` 进行编译期优化
- 使用 `using enum` 简化枚举使用
- 使用 `std::to_underlying` 枚举转底层类型

### 代码风格
- 使用现代C++特性，避免裸指针
- 使用RAII管理资源
- 明确的const正确性
- 移动语义优化
- 强类型：使用 `using` 定义语义化的类型别名

## 调试配置

### 编译期调试开关
在 `common.hh` 中定义以下宏来控制调试功能：

```cpp
#define DEBUG_SCANNER       // 词法分析跟踪
#define DEBUG_PARSER        // 语法分析跟踪
#define DEBUG_COMPILE       // 编译输出（反汇编）
#define DEBUG_TRACE_EXEC    // 执行跟踪（VM指令）
#define DEBUG_STRESS_GC     // GC压力测试（每次分配都触发GC）
#define DEBUG_LOG_GC        // GC详细日志
```

### 日志级别
使用Logger系统的分级日志：
- `Trace` - 最详细的执行跟踪（灰色）
- `Debug` - 调试信息（青色）
- `Info` - 一般信息（绿色）
- `Warn` - 警告（黄色）
- `Error` - 错误（红色）
- `Fatal` - 致命错误（紫色）

## 构建说明

### 依赖
- CMake 3.20+
- C++23兼容编译器：
  - Windows: MSVC 2022+
  - Linux: GCC 13+ / Clang 16+

### 构建命令
```bash
# 创建构建目录
mkdir build && cd build

# 配置（Debug模式）
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG=ON

# 配置（Release模式）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建
cmake --build .

# 运行
./maple ../tests/scripts/test_basic.maple
```

## 测试说明

### 测试类型
1. **单元测试** - 测试单个组件（Value、Chunk、Scanner等）
2. **集成测试** - 测试完整流程（编译+执行）
3. **脚本测试** - 使用.maple脚本进行端到端测试

### 运行测试
```bash
# 使用测试运行器
cd tests && python run_tests.py

# 运行单个测试脚本
./maple tests/scripts/test_arithmetic.maple
```

## 开发指南

### 添加新功能的一般流程
1. 阅读DESIGN.md了解架构设计
2. 查看PLAN.md找到对应任务
3. 先编写头文件定义接口
4. 实现功能
5. 添加单元测试
6. 更新PLAN.md标记任务完成

### 提交检查清单
- [ ] 代码符合命名规范
- [ ] 无编译器警告（-Wall -Wextra）
- [ ] 添加了必要的单元测试
- [ ] 更新了PLAN.md中的任务状态
- [ ] 关键代码有注释说明

## 常用命令

```bash
# 快速构建并测试
mkdir -p build && cd build && cmake .. && cmake --build . && ./maple ../tests/scripts/hello.maple

# 启用GC压力测试
cmake .. -DENABLE_STRESS_GC=ON

# 运行特定调试测试
./maple --debug-scan ../tests/scripts/test.maple
./maple --debug-exec ../tests/scripts/test.maple
```

## 参考资源

- 设计文档：[DESIGN.md](./DESIGN.md)
- 实施计划：[PLAN.md](./PLAN.md)
- 需求文档：[REQUIREMENTS.md](./REQUIREMENTS.md)
- 参考书籍：《Crafting Interpreters》- Robert Nystrom
