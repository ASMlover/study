# Maple 语言实现计划

## 1. 项目概述

基于 clox (Crafting Interpreters) 实现一个完整的脚本语言解释器 Maple，使用 C++23 编写。

## 2. 技术架构

### 2.1 核心组件

| 组件 | 文件 | 描述 |
|-----|------|------|
| Scanner | scanner.hh/cc | 词法分析器 |
| Compiler | compiler.hh/cc | 字节码编译器 |
| VM | vm.hh/cc | 虚拟机执行器 |
| Memory | memory.hh/cc | 内存管理与GC |
| Object | object.hh/cc | 对象系统 |
| Value | value.hh/cc | 值类型系统 |
| Table | table.hh/cc | 哈希表 |
| Chunk | chunk.hh/cc | 字节码块 |
| Debug | debug.hh/cc | 调试工具 |
| Logger | logger.hh/cc | 彩色日志系统 |
| Import | import.hh/cc | 模块导入系统 |

### 2.2 项目结构

```
mslang2/
├── include/ms/
│   ├── common.hh      # 通用定义（前置声明）
│   ├── chunk.hh       # 字节码块
│   ├── value.hh       # 值类型
│   ├── object.hh      # 对象系统
│   ├── table.hh       # 哈希表
│   ├── scanner.hh     # 词法分析器
│   ├── compiler.hh    # 编译器
│   ├── vm.hh           # 虚拟机
│   ├── memory.hh       # 内存管理
│   ├── debug.hh        # 调试工具
│   ├── logger.hh       # 日志系统
│   └── import.hh       # 模块导入
├── src/
│   ├── main.cc        # 入口
│   ├── common.cc
│   ├── chunk.cc
│   ├── value.cc
│   ├── object.cc
│   ├── table.cc
│   ├── scanner.cc
│   ├── compiler.cc
│   ├── vm.cc
│   ├── memory.cc
│   ├── debug.cc
│   ├── logger.cc
│   └── import.cc
├── tests/
│   ├── hello.mp
│   ├── function.mp
│   ├── class.mp
│   ├── closure.mp
│   └── import/
├── CMakeLists.txt
└── README.md
```

## 3. 分阶段实现计划

### Phase 1: 基础框架 (可验收)
- [x] CMake 构建配置
- [x] common.hh 通用定义（前置声明解决循环依赖）
- [x] Chunk 字节码块
- [x] Value 值类型系统
- [x] Scanner 词法分析器
- [x] main.cc REPL入口
- **验收**: REPL启动并能输入简单表达式 ✅

### Phase 2: 表达式解析 (可验收)
- [x] Object 对象系统
- [x] Table 哈希表
- [x] Compiler 编译器
- [x] 表达式解析（优先级、括号）
- **验收**: 支持 `1 + 2 * 3` 等表达式计算 ✅

### Phase 3: 变量和语句 (可验收)
- [ ] 变量声明 `var x = 1;`
- [ ] 赋值语句
- [ ] Print语句
- [ ] 全局变量
- **验收**: `var x = 5; print x;` 正常工作

### Phase 4: 控制流 (可验收)
- [ ] if 条件语句
- [ ] while 循环
- [ ] for 循环
- [ ] 逻辑运算符
- **验收**: 阶乘、斐波那契等基本算法

### Phase 5: 函数系统 (可验收)
- [ ] 函数定义 `fun add(a, b) { return a + b; }`
- [ ] 函数调用
- [ ] 本地变量
- [ ] 返回语句
- **验收**: 递归函数正常工作

### Phase 6: 闭包和Upvalue (可验收)
- [ ] 闭包实现
- [ ] Upvalue捕获
- [ ] 匿名函数
- **验收**: 闭包正确捕获外部变量

### Phase 7: 类和对象 (可验收)
- [ ] 类定义
- [ ] init构造函数
- [ ] 方法
- [ ] this关键字
- [ ] 继承
- **验收**: 面向对象程序正常运行

### Phase 8: 垃圾回收器 (可验收)
- [ ] 标记-清除GC
- [ ] 弱引用表
- [ ] 性能优化
- **验收**: 长时间运行无内存泄漏

### Phase 9: 扩展类型 (可验收)
- [ ] List类型 `[]`, `list[0]`
- [ ] Map类型 `{}`, `map["key"]`
- [ ] 索引赋值 `list[0] = 1`
- **验收**: 容器类型操作正常

### Phase 10: 模块系统 (可验收)
- [ ] Import语句
- [ ] 模块路径解析
- [ ] 导出/导入绑定
- **验收**: 多文件项目正常运行

## 4. 语言特性

### 4.1 数据类型
- nil, bool, number, string
- function, class, instance, closure
- list, map (扩展)

### 4.2 语法特性
- 变量声明: `var x = 1;`
- 函数: `fun add(a, b) { return a + b; }`
- 类: `class Animal { init(name) { this.name = name; } }`
- 继承: `class Dog < Animal { }`
- 模块导入: `import module; from module import fn as alias;`

## 5. 验收标准

- [ ] Windows MSVC 编译通过
- [ ] Linux GCC 编译通过
- [ ] REPL 模式正常
- [ ] 文件执行正常
- [ ] 基础功能测试通过
- [ ] import 模块导入正常
- [ ] 彩色日志正常输出
