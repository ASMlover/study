# AGENTS.md

## 项目信息

- **项目名称**: Maple (枫) 脚本语言解释器
- **实现语言**: C++23
- **参考实现**: clox (Crafting Interpreters)
- **目标平台**: Windows (MSVC), Linux (GCC)
- **命名空间**: `ms`

## 模型信息

- **当前使用模型**: MiniMax-M2.5 (minimax-cn-coding-plan/MiniMax-M2.5)
- **设计日期**: 2026-03-03

## 构建命令

```bash
# 创建构建目录
mkdir build && cd build

# Windows (MSVC)
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

# Linux (GCC)
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# 运行
./maple ../tests/hello.mp
```

## 开发规范

1. 头文件使用 `.hh` 扩展名
2. 实现文件使用 `.cc` 扩展名
3. 使用 `#pragma once` 防止重复包含
4. 命名空间统一使用 `ms`
5. 尽量使用标准库，减少第三方依赖
6. 代码文件使用 UTF-8 格式 LF 换行
7. 自动清除文件的行尾空白符
