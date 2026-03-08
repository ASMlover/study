# AGENTS.md — AI 助手协作指南

本文档为参与本仓库开发的 AI Agent 提供项目上下文与协作约定，便于正确理解架构并保持风格一致。

---

## 1. 项目身份

- **项目名**：Maple 脚本语言（mslang6）
- **类型**：参考 [Crafting Interpreters](https://github.com/munificent/craftinginterpreters) 中 **clox** 的字节码脚本语言实现
- **实现语言**：C++23，命名空间 `ms`
- **构建**：CMake，支持 Windows（MSVC）与 Linux（GCC）
- **开发环境**：使用 [Cursor](https://cursor.com/) 开发

---

## 2. 必读文档

| 文档 | 用途 |
|------|------|
| [REQUIREMENTS.md](REQUIREMENTS.md) | 功能与非功能需求、交付物 |
| [PLAN.md](PLAN.md) | 实现架构、模块划分、接口设计、实现顺序 |
| 本文档 (AGENTS.md) | Agent 协作约定与代码规范 |

修改架构或需求时，请同步更新上述文档。

---

## 3. 代码规范（必须遵守）

- **C++ 标准**：C++23 及以上；不使用已废弃或非标准扩展。
- **文件命名**：
  - 头文件：`.hh`
  - 实现文件：`.cc`
- **头文件保护**：使用 `#pragma once`，不要使用 `#ifndef` / `#define` / `#endif`。
- **命名空间**：所有对外 API 与类型放在 `namespace ms` 中。
- **依赖**：优先使用 C++ 标准库；若需第三方库，需在 PLAN/README 中说明理由。
- **代码组织**：按模块拆分到不同 `.hh`/`.cc`，避免单文件集中实现；新增模块时在 PLAN.md 的目录结构中补充说明。

---

## 4. 文件格式与版本控制

### 4.1 文件格式

- **编码**：所有源文件、配置与文档均使用 **UTF-8** 编码。
- **换行符**：使用 **LF**（`\n`），不使用 CRLF（`\r\n`）。
- **行尾空白**：编辑或生成文件时**自动清除行尾空白符**（trailing whitespace），提交前确保无行末空格或制表符。

### 4.2 Git 提交

- **提交信息语言**：Git 提交信息（commit message）须使用**英文**书写。
- **Gitmoji**：提交信息中须包含 **gitmoji** 标识（如 `:sparkles:`、`:bug:`、`:memo:` 等），便于在历史与列表中快速识别变更类型。  
  参考：[gitmoji](https://gitmoji.dev/)  
  示例：`✨ Add UTF-8 and LF rules to AGENTS.md`、`🐛 Fix null check in scanner`

---

## 5. 目录与模块速查

```
mslang6/
├── CMakeLists.txt
├── REQUIREMENTS.md
├── PLAN.md
├── AGENTS.md
├── README.md
├── include/ms/          # 或 src/ms/ 下的头文件
│   ├── common.hh
│   ├── chunk.hh/cc
│   ├── value.hh/cc
│   ├── memory.hh/cc
│   ├── table.hh/cc
│   ├── object.hh/cc
│   ├── scanner.hh/cc
│   ├── compiler.hh/cc
│   ├── vm.hh/cc
│   ├── debug.hh/cc
│   ├── logger.hh/cc
│   ├── module.hh/cc
│   └── maple.hh
├── src/
│   └── main.cc
└── tests/
    ├── README.md
    └── *.ms
```

- **参考实现**：clox 的 scanner → compiler → chunk → vm → memory/object/table；Maple 在此之上增加 logger、module（import）。
- 修改或新增模块时，请与 PLAN.md 中的“目录与模块划分”“核心模块设计”保持一致，并在 AGENTS.md 本节的目录树中更新（若结构有变）。

---

## 6. 实现与修改时的注意点

- **不要偏离 PLAN.md**：架构、模块职责、接口约定以 PLAN.md 为准；若确有更好的设计，先更新 PLAN.md 再改代码。
- **保持 clox 语义**：字节码指令集、值类型、对象类型、GC 根集合等与 clox 对齐，便于对照原书与参考实现；扩展（如 import）在 PLAN 的“模块与 import”一节有说明。
- **跨平台**：涉及路径、控制台颜色、行尾等时，需同时考虑 Windows 与 Linux；可用 `#ifdef _WIN32` 等，但尽量收敛到少量平台抽象（如 logger、module 的路径解析）。
- **测试**：新增或修改语言特性时，在 `tests/` 下补充或修改 `.ms` 脚本，并更新 `tests/README.md` 的说明与期望行为。
- **Logger**：仅用于 C++ 内部调试（编译、VM、GC），不向 Maple 脚本暴露；颜色与级别约定见 PLAN.md 的“日志 (logger)”小节。
- **任务状态同步**：**任一任务实现并验证通过后，必须立即在 PLAN.md 的「8. 实现任务清单」中将该任务的状态更新为已完成**（在对应任务行的「状态」列中填写 `✅ 已完成` 或约定符号）；未完成任务的「状态」列保持 `待办`。Agent 在完成并验证某任务后，应同时提交对 PLAN.md 的状态更新，保证文档与实现一致。

---

## 7. 常见任务指引

- **实现清单中的任务**：按 PLAN.md 第 8 节任务顺序实现；**每完成并验证一个任务后，须在 PLAN.md 对应任务行的「状态」列中更新为已完成**（如 `✅ 已完成`），再继续下一任务。
- **新增字节码指令**：在 chunk（OpCode）、compiler（生成）、vm（解释）三处修改，必要时在 object/memory 中处理新对象或 GC 根；更新 PLAN 中“字节码块”或“虚拟机”描述。
- **新增内置/原生函数**：在 vm 中注册（如 `defineNative`），实现签名与 clox 一致（参数个数 + Value*）；在 REQUIREMENTS/PLAN 中列出新增 API。
- **修改 import 语义**：改 module 与 compiler（解析 import/from-import）、vm（加载与注入）；同步更新 PLAN 的“模块与 import”和 tests 中的 import 用例。
- **调整 GC 或内存**：在 memory 与 object 中修改；确保所有 GC 根在 PLAN 的“内存与垃圾回收”中已列出并在 mark 阶段处理。
- **构建/依赖变更**：修改 CMakeLists.txt，并在 README 与 PLAN 的“构建与平台”中说明。

---

## 8. 与用户沟通

- 用户规则要求使用**简体中文**回复。
- 涉及架构或需求时，引用 REQUIREMENTS.md 与 PLAN.md 的具体章节；建议的变更若会影响设计，先给出对文档的修改建议再改代码。

---

以上约定请在与本仓库交互时遵守，以保证实现与设计文档一致、便于后续维护与协作。
