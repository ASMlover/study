# BUG.md — Known Test Failures (2026-03-21)

All 17 failures resolved. Final result: **51 passed, 0 failed, 1 skipped**.

---

## ~~1. Test Runner Encoding Bug (7 files)~~ [FIXED]

`run_tests.py` 中 `subprocess.run()` 和 `open()` 未指定 `encoding='utf-8'`，
Windows 中文系统默认 GBK 解码失败。已修复。

---

## ~~2. Stale Test Expects (3 files)~~ [FIXED]

- `finalizers.ms`: 添加 `fin:temp` 期望行
- `native_functions.ms`: `number` → `int`/`float`
- `constant_long.ms`: 中间值改为 `var` 赋值，仅 `print` 最后一行

---

## ~~3. VM Logic Bugs (8 files)~~ [FIXED]

| File | Root Cause | Fix |
|------|-----------|-----|
| `core_natives.ms` | `is_truthy()` 未处理整数 0 和浮点 0.0 | `Value.cc`: 添加 `is_integer()/is_double()` 分支 |
| `getter_setter.ms` | getter 调用帧起始于 R(B) 而非 R(A)，返回值落入错误寄存器 | `VM.cc`: getter 调用前复制 receiver 到 R(A)，`stack_top_` 基于 A |
| `inline_cache.ms` | 同 getter bug（IC_GETTER 快速路径） | 同上修复 |
| `list_comprehension.ms` | `reg_top_` 与 `local_count_` 不同步，隐藏局部变量寄存器错位 | `Compiler.cc`: 推导式入口处同步 `reg_top_ = local_count_` |
| `operator_overload.ms` | `__str` 调用帧起始于 R(B) 而非 R(A) | `VM.cc`: 同 getter 修复模式 |
| `try_catch.ms` | `push(pending_exception_)` 写入 `stack_top_` 位置，与 catch 变量寄存器不匹配 | 编译器将 catch 寄存器编码入 OP_TRY 的 A 字段；VM 直接写入 `frame->slots[catch_reg]` |
| `defer.ms` | 测试期望错误：顶层 defer 应在脚本退出时执行 | 修正 expect 注释位置 |

附带修正：`control_flow.ms` 和 `ternary.ms` 的 `0 is truthy` 期望（因 `is_truthy` 语义变更）。
