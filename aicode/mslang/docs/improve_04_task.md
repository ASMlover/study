# Maple Language — 内存与性能优化实施计划

> Version: 1.1 | Date: 2026-03-27

本文档是对 Maple 语言运行时（mslang）的内存布局与执行性能进行系统性优化的实施计划。
所有任务按优先级分级，每个任务均独立可实现、可构建、可测试。

---

## 任务总览

| 任务 ID | 标题 | 优先级 | 工作量 | 依赖 | 状态 |
|---------|------|--------|--------|------|------|
| OPT-P0-01 | 移除 Object 虚函数表（含虚析构） | P0 | 大 | — | `[ ]` |
| OPT-P0-02 | ObjString 内联字符数据 | P0 | 大 | OPT-P0-01 | `[ ]` |
| OPT-P0-03 | ObjUpvalue/ObjBoundMethod 固定池分配器 | P0 | 中 | OPT-P0-01 | `[ ]` |
| OPT-P1-01 | ObjInstance 内联字段数组（SBO） | P1 | 大 | OPT-P0-01 | `[ ]` |
| OPT-P1-02 | NativeFn 改为裸函数指针 | P1 | 小 | — | `[ ]` |
| OPT-P1-03 | 常量池去重 | P1 | 小 | — | `[ ]` |
| OPT-P1-04 | GETPROP/SETPROP 直接编码 IC 索引 | P1 | 中 | — | `[ ]` |
| OPT-P1-05 | ObjMap 迭代 O(n²) 修复 | P1 | 小 | — | `[ ]` |
| OPT-P1-06 | ASCII 单字符 ObjString 缓存 | P1 | 小 | — | `[ ]` |
| OPT-P1-07 | ObjClosure 内联 upvalue 数组 | P1 | 中 | OPT-P0-01 | `[ ]` |
| OPT-P2-01 | ObjClass 稀用 Table 按需分配 | P2 | 小 | — | `[ ]` |
| OPT-P2-02 | Shape 小型线性映射替代 unordered_map | P2 | 小 | — | `[ ]` |
| OPT-P2-03 | Remembered Set 去重 | P2 | 小 | — | `[ ]` |
| OPT-P2-04 | ObjMap 改为开放地址哈希表 | P2 | 中 | OPT-P1-05 | `[ ]` |
| OPT-P2-05 | Coroutine SavedCallFrame 直接保存/恢复 | P2 | 小 | — | `[ ]` |
| OPT-P2-06 | Peephole NOP 指令压缩 | P2 | 中 | — | `[ ]` |
| OPT-P3-01 | CallFrame deferred 延迟分配 | P3 | 小 | — | `[ ]` |
| OPT-P3-02 | Quickening deopt 计数器 | P3 | 小 | — | `[ ]` |
| OPT-P3-03 | 编译器局部变量数组动态化 | P3 | 小 | — | `[ ]` |
| OPT-P3-04 | Coroutine 独立栈段 | P3 | 大 | OPT-P2-05 | `[ ]` |

---

## P0 — 核心收益（优先实现）

---

### `[ ]` OPT-P0-01：移除 Object 虚函数表（含虚析构）

**目标**

`Object` 基类因 `stringify()`、`trace_references()`、`size()` 三个虚函数以及 `virtual ~Object()`
共携带 8 字节 vtable 指针。GC 热路径每次都需虚函数分派，带来不可预测的间接跳转和 I-cache 压力。
**彻底去虚函数化**（包括析构）后每个对象节省 8 字节，Object 从 ≥24 字节缩减为 16 字节。

> ⚠️ **关键约束**：只要 `Object` 保留任何 `virtual` 方法（含虚析构），编译器就会生成 vtable，
> vtable 指针仍然占用 8B，内存节省目标完全落空。必须同时移除虚析构。

**背景数据**

- `trace_references()` 调用点：`VMGC.cc:113`（主追踪循环）、`VMGC.cc:157`（remembered set）、`VMGC.cc:422`（增量 GC）
- `size()` 调用点：`VMGC.cc:142,185,212,452`（sweep 路径）
- `stringify()` 调用点：`VMGC.cc:110,139,191,209,419`（调试日志）+ `Value::stringify()` + `OP_PRINT` + 错误报告，不在热路径
- `delete obj` 调用点（sweep 路径）：`VMGC.cc:143,215,453,495,504` — 移除虚析构后必须全部替换
- 16 个子类均覆写这 3 个方法（ObjString/ObjNative/ObjStringBuilder/ObjFile/ObjWeakRef 未覆写 `trace_references`）

**需修改的文件**

- `src/Object.hh` — 移除所有 virtual（含析构），添加 4 个 dispatch 函数声明
- `src/Object.cc` — 实现 4 个 dispatch 函数
- `src/VMGC.cc` — 替换 trace/size 调用点，替换所有 `delete obj` 为 `object_destroy`
- `src/Memory.cc` — 替换 `stringify` 调试日志调用
- `src/VM.cc` — 替换 `stringify` 调用点；`free_objects()` 中的 `delete` 同样替换
- `src/Value.cc` — `Value::stringify()` 通过 `object_stringify()` 调用
- `src/VMBuiltins.cc`, `src/VMNatives.cc` — stringify 调用点
- `src/Debug.cc` — disassembler stringify 调用点

**实施步骤**

1. **`src/Object.hh`**：移除四个 virtual 方法（三个虚方法 + 虚析构），将析构函数改为普通非虚：
   ```cpp
   // 删除这四行：
   virtual ~Object() = default;
   virtual str_t stringify() const noexcept = 0;
   virtual void  trace_references() noexcept {}
   virtual sz_t  size() const noexcept = 0;
   // 改为：
   ~Object() = default;   // 非虚析构
   ```
   在 `Object` 类外声明**四个** dispatch 函数：
   ```cpp
   str_t   object_stringify(const Object* obj) noexcept;
   void    object_trace(Object* obj) noexcept;
   sz_t    object_size(const Object* obj) noexcept;
   void    object_destroy(Object* obj) noexcept;  // 替换 delete obj
   ```

2. **`src/Object.cc`**：实现四个 dispatch 函数，内部用 `switch(obj->type())` 分发：
   - `object_trace`：对有引用追踪逻辑的子类分支执行追踪，无引用的 `default: break;`
   - `object_size`：每个分支返回 `sizeof(SubType)`（ObjString/ObjInstance/ObjClosure 需含 FAM 大小）
   - `object_stringify`：每个分支调用子类的 `static str_t do_stringify(const SubType*)` 方法
   - `object_destroy`：每个分支显式调用子类析构，如 `static_cast<ObjString*>(obj)->~ObjString()`
   各子类原来的 `override` 方法重命名为 `static` 私有方法（如 `ObjString::do_stringify`）。

3. **`src/VMGC.cc`**：
   - `VMGC.cc:113,157,422`：`obj->trace_references()` → `object_trace(obj)`
   - `VMGC.cc:142,185,212,452`：`obj->size()` → `object_size(obj)`
   - `VMGC.cc:143,215,453,495,504`：`delete obj` → `object_destroy(obj); ::operator delete(obj);`

4. **`src/VMGC.cc`** 中所有调试日志的 `->stringify()` → `object_stringify(...)`

5. **`src/VM.cc`**（`free_objects` 等处）：`delete obj` → `object_destroy(obj); ::operator delete(obj);`

6. **`src/Value.cc`**：`as_object()->stringify()` → `object_stringify(as_object())`

7. **`src/VMBuiltins.cc`**、**`src/VMNatives.cc`**、**`src/Debug.cc`**：所有 `->stringify()` → `object_stringify(...)`

8. 添加布局验证：
   ```cpp
   static_assert(!std::is_polymorphic_v<Object>, "Object must not have vtable");
   static_assert(sizeof(Object) == 16, "Object base must be 16B after devirt");
   ```

**验证**

```bash
cmake --build build
ctest --test-dir build --output-on-failure
python tests/run_tests.py
# 重点测试：tests/classes.ms, tests/gc.ms, tests/inline_cache.ms
# 可选：mslang --benchmark 5 benchmarks/binary_trees.ms
```

**风险**

- `object_destroy` 中必须覆盖所有 16 个子类——遗漏任何一个将导致资源泄漏（ObjFile 未关闭、Shape 未递归释放等）。
  建议在 switch 的 `default:` 中加 `MAPLE_UNREACHABLE()` 或 `assert(false)` 确保不会遗漏
- 需确保所有子类的原 override 方法改为 `static` 后，返回类型和行为完全一致
- `trace_references` 的空实现（ObjString 等未覆写的子类）在 switch 中对应 `default: break;` 即可

---

### `[ ]` OPT-P0-02：ObjString 内联字符数据

**目标**

`ObjString::value_`（`std::string`）占用 32 字节固定头部，且字符数据在堆上二次分配。
改为 flexible array member 后，每个字符串是单次连续分配，
消除 `std::string` 头部开销和二次间接寻址。

**背景数据**

- `ObjString::value()` 返回 `const str_t&`，有约 65+ 个调用点
- 最热调用点：`Table.cc:161`（字符串 intern 查找）、`VMCall.cc:36`（字符串拼接）、`VM.cc:762`（OP_ADD_SS）
- 字符串分配路径：`VM::copy_string`（`VM.cc:182`）、`VM::take_string`（`VM.cc:194`）
- `Serializer.cc:191,246` 读取字符串内容用于序列化

**依赖**：OPT-P0-01（需先移除 vtable，确认 Object 布局稳定）

**需修改的文件**

- `src/Object.hh` — 修改 ObjString 类定义
- `src/Object.cc` — 修改 ObjString 构造/size/静态方法
- `src/VM.cc` — copy_string、take_string 分配逻辑
- `src/Table.cc` — find_string（需要 length + hash + char* 比较）
- `src/Serializer.cc` — 序列化读写字符串内容
- 所有调用 `string_obj->value()` 的文件（返回 `strv_t` 代替 `const str_t&`）

**实施步骤**

1. **`src/Object.hh`**：修改 `ObjString`：
   ```cpp
   class ObjString final : public Object {
     u32_t hash_{0};
     u32_t length_{0};
     char  data_[];   // flexible array member，跟随对象末尾
   public:
     // 禁止拷贝/移动（布局特殊）
     ObjString(const ObjString&) = delete;
     ObjString& operator=(const ObjString&) = delete;

     strv_t value() const noexcept { return {data_, length_}; }
     u32_t  hash()  const noexcept { return hash_; }
     u32_t  length() const noexcept { return length_; }
     cstr_t c_str() const noexcept { return data_; }

     static ObjString* create(cstr_t chars, u32_t length, u32_t hash) noexcept;
     static u32_t hash_string(cstr_t chars, sz_t length) noexcept;
   };
   ```
   移除 `str_t value_` 成员和原有构造函数。

2. **`src/Object.cc`**：实现 `ObjString::create`：
   ```cpp
   ObjString* ObjString::create(cstr_t chars, u32_t length, u32_t hash) noexcept {
     // 单次分配：ObjString 头部 + length+1 字节
     void* mem = ::operator new(sizeof(ObjString) + length + 1);
     ObjString* s = ::new(mem) ObjString();  // placement new，空构造
     s->hash_   = hash;
     s->length_ = length;
     std::memcpy(s->data_, chars, length);
     s->data_[length] = '\0';
     return s;
   }
   // size 实现
   sz_t ObjString::do_size() const noexcept {
     return sizeof(ObjString) + length_ + 1;
   }
   ```

3. **`src/VM.cc`** — `copy_string`（`VM.cc:~180`）和 `take_string`（`VM.cc:~194`）：
   改为调用 `ObjString::create`，不再构造 `str_t`；
   `take_string` 同样调用 `create`（名字语义不变，只是来源是已有字符缓冲）。

4. **`src/Table.cc`** — `find_string`（`Table.cc:~155`）：
   原本是 `entry->key->value() == str_t(chars, length)`；
   改为 `entry->key->length() == length && std::memcmp(entry->key->c_str(), chars, length) == 0`。

5. **所有 `->value()` 调用点**：返回类型从 `const str_t&` 变为 `strv_t`。
   大多数调用（字符串比较）直接接受 `strv_t`，拼接路径需要特殊处理。
   重点检查：
   - `VMCall.cc:36`（字符串拼接热路径）：不要写 `str_t(a->value()) + str_t(b->value())`（两次构造）；
     应预分配缓冲区一次写入：
     ```cpp
     str_t result;
     result.reserve(a->length() + b->length());
     result.append(a->c_str(), a->length());
     result.append(b->c_str(), b->length());
     // 再调用 copy_string(result.c_str(), result.size())
     ```
     或添加双串版本 `copy_string(a, b)` 直接分配 `sizeof(ObjString) + a->length() + b->length() + 1` 并 memcpy 两段。
   - `VM.cc:762`（OP_ADD_SS）和 `VM.cc:1973`（OP_ADD_SS quickened）：同样使用预分配方案
   - `Serializer.cc:191,246`：写入字符串内容，用 `obj->c_str()` + `obj->length()`

6. **`src/Serializer.cc`**：反序列化时用 `ObjString::create` 替代原来的 `copy_string`（保持 intern 语义）。

**验证**

```bash
cmake --build build
ctest --test-dir build --output-on-failure
python tests/run_tests.py
# 重点：tests/strings.ms, tests/string_methods.ms, tests/string_interpolation.ms
# 确认序列化：运行有 import 的测试（模块缓存读写包含字符串）
```

**风险**

- flexible array member 不能有普通构造函数参数（需 placement new）；`ObjString` 析构时用 `::operator delete` 而非 `delete`，注意与 GC sweep `delete object` 的兼容。
  解决方案：在析构函数中不做任何事（数据是 POD），GC 的 `object_destroy(obj)` 函数中对 ObjString 调用 `::operator delete(obj)` 而非 `delete obj`。
- `strv_t` 生命周期必须不超过 `ObjString` 对象本身——GC 可能在下次分配时回收 ObjString，请勿在 GC 可能触发的代码路径上长期持有裸 `strv_t`。

---

### `[ ]` OPT-P0-03：ObjUpvalue / ObjBoundMethod 固定池分配器

**目标**

`ObjUpvalue`（~40B）和 `ObjBoundMethod`（~40B）是调用密集型代码中最频繁分配的两类对象
（每次创建闭包捕获变量时分配 ObjUpvalue，每次方法查找时分配 ObjBoundMethod）。
为这两类固定大小对象引入 slab/freelist 池分配器，
将分配从系统 malloc 降为链表出队（1-2 条指令）。

**需修改的文件**

- `src/Memory.hh` — 新增 `ObjectPool<T>` 模板
- `src/Memory.cc` — 实现 `ObjectPool`
- `src/VM.hh` — 在 VM 中持有两个 pool 实例
- `src/VM.cc` — `allocate<ObjUpvalue>` 和 `allocate<ObjBoundMethod>` 走 pool 路径
- `src/VMGC.cc` — sweep 时归还到 pool 而非 `delete`

**实施步骤**

1. **`src/Memory.hh`**：声明 `ObjectPool<T>`：
   ```cpp
   // 固定大小对象的 slab 池，每个 slab 64 个对象
   template <typename T>
   class ObjectPool {
     static constexpr sz_t kSlabSize = 64;
     struct Slab { alignas(T) char data[sizeof(T) * kSlabSize]; };
     struct FreeNode { FreeNode* next; };

     std::vector<std::unique_ptr<Slab>> slabs_;
     FreeNode* free_list_{nullptr};
     sz_t      allocated_{0};
   public:
     T*   alloc() noexcept;    // 从 freelist 出队；空时新建 slab
     void free(T* obj) noexcept;  // 归还到 freelist（不调用析构）
     void destroy_all() noexcept; // shutdown 时销毁所有 slab
     sz_t size_bytes() const noexcept; // 当前占用字节数（用于 GC 统计）
   };
   ```

2. **`src/Memory.cc`**：实现 `alloc` 和 `free`：
   - `alloc`：若 `free_list_` 非空，pop 并返回；否则创建新 slab，将其 64 个槽全部链入 `free_list_`，再 pop 一个。
   - `free`：将对象指针强转为 `FreeNode*` 并 push 到 `free_list_` 头部（对象内存复用存储链指针）。

3. **`src/VM.hh`**：添加两个 pool 成员：
   ```cpp
   ObjectPool<ObjUpvalue>     upvalue_pool_;
   ObjectPool<ObjBoundMethod> bound_method_pool_;
   ```

4. **`src/VM.cc`** — `allocate<T>` 模板（`VM.cc:~218`）：
   对 `ObjUpvalue` 和 `ObjBoundMethod` 特化（或在调用前判断类型），改为 pool alloc + placement new：
   ```cpp
   // 特化版本
   template<>
   ObjUpvalue* VM::allocate<ObjUpvalue>(Value* slot) {
     void* mem = upvalue_pool_.alloc();
     ObjUpvalue* obj = ::new(mem) ObjUpvalue(slot);
     // 链入 young_objects_，accounting 同现有逻辑
     young_bytes_ += sizeof(ObjUpvalue);
     bytes_allocated_ += sizeof(ObjUpvalue);
     obj->set_generation(GcGeneration::YOUNG);
     obj->set_next(young_objects_);
     young_objects_ = obj;
     return obj;
   }
   ```

5. **`src/VMGC.cc`** — sweep 路径：
   OPT-P0-01 完成后，sweep 路径已无 `delete obj`，统一走 `object_destroy(obj) + ::operator delete(obj)`。
   在 `object_destroy` 的 switch-case 中，对 ObjUpvalue 和 ObjBoundMethod 分支改为 pool 归还：
   ```cpp
   case ObjectType::OBJ_UPVALUE: {
     auto* uv = static_cast<ObjUpvalue*>(obj);
     uv->~ObjUpvalue();                  // 显式析构（trivial，但保持正确性）
     upvalue_pool_.free(uv);             // 归还内存，不调用 ::operator delete
     return;                             // 不执行后续的 ::operator delete
   }
   case ObjectType::OBJ_BOUND_METHOD: {
     auto* bm = static_cast<ObjBoundMethod*>(obj);
     bm->~ObjBoundMethod();
     bound_method_pool_.free(bm);
     return;
   }
   ```
   在 sweep 调用点的 `::operator delete(obj)` 改为仅在 `object_destroy` 返回后执行——pool 对象在
   `object_destroy` 内已归还，外层的 `::operator delete` 需要跳过。
   简化方案：让 `object_destroy` 对 pool 对象内部完成全部释放（含 free），返回 `false` 表示无需外部释放；
   对普通对象析构后返回 `true`，调用方再执行 `::operator delete`。
   同时在 `Memory.hh` 添加编译期守卫：
   ```cpp
   static_assert(std::is_trivially_destructible_v<ObjUpvalue>,
                 "ObjUpvalue pool free skips destructor side effects");
   static_assert(std::is_trivially_destructible_v<ObjBoundMethod>,
                 "ObjBoundMethod pool free skips destructor side effects");
   ```
   在 `free_objects()`（VM shutdown）中调用 `upvalue_pool_.destroy_all()` 和 `bound_method_pool_.destroy_all()`。

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
ctest --test-dir build --output-on-failure
python tests/run_tests.py
# STRESS_GC 模式下每次分配都触发 GC，快速发现 pool/GC 交互的 bug
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
mslang --benchmark 5 benchmarks/method_dispatch.ms   # ObjBoundMethod 热路径
mslang --benchmark 5 benchmarks/binary_trees.ms      # ObjUpvalue + 闭包
```

**风险**

- Pool 中的对象内存被 freelist 链表节点复用：`FreeNode*` 存储在对象起始地址处。确认 `sizeof(FreeNode*) <= sizeof(T)` 且 T 的 alignment 满足（ObjUpvalue ~40B，ObjBoundMethod ~40B，均远大于 8B 指针，安全）。
- 若未来 ObjUpvalue 或 ObjBoundMethod 添加非 trivial 成员（如 `std::vector`），`static_assert` 会立即编译失败，强制开发者更新 pool 释放逻辑。

---

## P1 — 重要改进

---

### `[ ]` OPT-P1-01：ObjInstance 内联字段数组（SBO）

**目标**

`ObjInstance::fields_`（`std::vector<Value>`）有 24 字节头部开销，且字段数据是堆上的第二次分配。
采用 Small Buffer Optimization（SBO）方案：内联存储最多 `kInlineFields` 个字段，
超出时使用 `overflow_` 指针指向堆上的 Value 数组。
消除绝大多数实例（字段数 ≤8）的 vector 头部和堆分配开销，同时**不改变指针语义**（无悬垂指针风险）。

> ⚠️ **不采用纯 FAM + realloc 方案**：realloc 后旧指针作废，需更新所有持有旧指针的栈槽、上值、
> 全局变量、GC 链表等，在无转发指针的 mark-sweep GC 中不可安全实现。

**背景数据**

- `ObjInstance` 在 `VMCall.cc:133` 分配
- 字段访问快速路径：`Object.hh:335`（`get_field(u32_t slot)`）和 `Object.hh:336`（`set_field(u32_t slot, val)`）
- Shape 的 `slot_count()` 在分配时已知；新属性追加时调用 `shape_->add_transition()` + `fields_.push_back`

**依赖**：OPT-P0-01（Object 布局稳定后再改 ObjInstance）

**需修改的文件**

- `src/Object.hh` — 修改 ObjInstance 类定义，移除 `std::vector<Value> fields_`
- `src/Object.cc` — 修改字段访问、追加、GC trace、size
- `src/VMCall.cc` — 分配时预留初始容量

**实施步骤**

1. **`src/Object.hh`** — 修改 `ObjInstance`：
   ```cpp
   class ObjInstance final : public Object {
     static constexpr u32_t kInlineFields = 8;
     ObjClass* klass_{nullptr};
     Shape*    shape_{nullptr};
     u32_t     field_count_{0};
     u32_t     capacity_{kInlineFields};
     Value     inline_fields_[kInlineFields]{};  // 内联字段槽
     Value*    overflow_{nullptr};               // 仅超过 8 字段时分配
   public:
     Value  get_field(u32_t slot) const noexcept;
     void   set_field(u32_t slot, Value val) noexcept;
     bool   get_field(ObjString* name, Value* out) const noexcept;
     void   add_field(ObjString* name, Value val) noexcept;  // shape 转换 + 追加
     ObjClass* klass() const noexcept { return klass_; }
     Shape*    shape() const noexcept { return shape_; }
     u32_t     field_count() const noexcept { return field_count_; }
   private:
     Value* fields_ptr() noexcept {
       return overflow_ ? overflow_ : inline_fields_;
     }
   };
   ```

2. **`src/Object.cc`** — 实现字段操作：
   - `get_field(u32_t slot)`：`return fields_ptr()[slot]`
   - `set_field(u32_t slot, Value val)`：`fields_ptr()[slot] = val`
   - `add_field(ObjString* name, Value val)`：
     - 调用 `shape_->add_transition(name)` 获取新 shape 和新 slot_index
     - 若 `field_count_ < capacity_`：写入 `fields_ptr()[field_count_++]`
     - 否则：`capacity_ *= 2`，重新分配 `overflow_` 并从旧位置复制（`overflow_` 指针本身不变，仅值被移动，ObjInstance 指针不变）

3. **`src/VMCall.cc:133`** — 无需特殊修改（默认构造即可）；内联缓冲自动初始化为 nil。

4. **`trace_references`**：遍历 `fields_ptr()[0..field_count_-1]`，调用 `mark_value`。
   注意：`overflow_` 是裸指针，不是 GC 对象，仅需在析构时 `delete[] overflow_`。

5. **`size`**：`sizeof(ObjInstance) + (overflow_ ? capacity_ * sizeof(Value) : 0)`。

6. **析构**：`ObjInstance` 的析构（在 `object_destroy` 中）调用 `delete[] overflow_;`（若非 null）。

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# 重点：tests/classes.ms, tests/inline_cache.ms, tests/gc.ms
mslang --benchmark 5 benchmarks/field_access.ms    # 字段访问性能
mslang --benchmark 5 benchmarks/binary_trees.ms    # 实例分配压力
```

**风险**

- `overflow_` 是非 GC 管理的裸指针——GC 不追踪它，只追踪其指向的 Value 内容。确保 `object_destroy` 中 `delete[] overflow_`，避免内存泄漏。
- `kInlineFields=8` 是调优参数，可在实测后调整；每个 ObjInstance 增加 `8 * 8 = 64B` 内联存储，需权衡内存占用与分配次数。

---

### `[ ]` OPT-P1-02：NativeFn 改为裸函数指针

**目标**

`NativeFn = std::function<Value(int,Value*)>` 在 capture 超出 small buffer 时会触发堆分配。
所有 23 个 native 函数都捕获 `[this]`（VM 指针），需改为传入 `VM&` 参数。
改为裸函数指针后，消除 `std::function` 的类型擦除分发开销和潜在堆分配。

**背景数据**

- 类型定义：`Object.hh:236`
- 唯一调用点：`VMCall.cc:124-127`
- 所有 native 定义：`VMNatives.cc`（23 个 lambda，均捕获 `[this]`）
- `VMImport.cc:56`：io 模块中也有一处 NativeFn 构造

**需修改的文件**

- `src/Object.hh` — 修改 `NativeFn` 类型别名，`ObjNative` 构造函数
- `src/VMCall.cc` — 调用点：传入 `*this`（VM）
- `src/VMNatives.cc` — 所有 lambda 改为 `[](VM& vm, int argc, Value* args)` 形式
- `src/VMImport.cc` — io 模块 native 构造

**实施步骤**

1. **`src/Object.hh`**：
   ```cpp
   // 前向声明 VM
   class VM;
   using NativeFn = Value(*)(VM& vm, int arg_count, Value* args);
   ```

2. **`src/VMCall.cc:124-127`**：
   ```cpp
   // 原来
   Value result = native(arg_count, stack_top_ - arg_count);
   // 改为
   Value result = native(*this, arg_count, stack_top_ - arg_count);
   ```

3. **`src/VMNatives.cc`**：将所有 `[this](int argc, Value* args)` lambda 改为接受 `VM& vm` 的普通函数或静态 lambda：
   ```cpp
   // 原来
   define_native("strlen", [this](int argc, Value* args) { ... use this-> ... });
   // 改为（静态/无捕获 lambda 可隐式转为函数指针）
   define_native("strlen", [](VM& vm, int argc, Value* args) { ... use vm. ... });
   ```
   将所有 `this->runtime_error(...)` 替换为 `vm.runtime_error(...)`，
   将所有 `this->copy_string(...)` 替换为 `vm.copy_string(...)`，以此类推。

4. **`src/VMImport.cc:56`**：同样转换为无捕获 lambda（传入 `VM&`）。

5. **验证**：无捕获的 lambda 可隐式转换为函数指针，所有 `define_native` 调用应该直接编译通过。
   若某个 native 确实需要捕获额外状态（检查后无此情况），可在 `ObjNative` 中添加 `void* userdata` 字段。

**验证**

```bash
cmake --build build
python tests/run_tests.py
# native 函数用于所有测试，任何错误都会立刻暴露
```

**风险**

- 低风险：仅为函数签名变更，行为不变
- 编译器若对无捕获 lambda → 函数指针转换有疑虑，可以将 lambda 改为 `static` 文件级函数

---

### `[ ]` OPT-P1-03：常量池去重

**目标**

`Chunk::add_constant()` 是无条件 push_back，同一标识符在函数体内被引用 N 次就产生 N 个常量池条目
（虽然底层 `ObjString*` 是同一个 intern 对象，但槽位被重复分配）。
添加去重后，常量池尺寸减少，bytecode 更紧凑，I-cache 效率更高。

**背景数据**

- `Chunk::add_constant`：`Chunk.cc:42`
- `Compiler::make_constant`：`Compiler.cc:478`
- `Compiler::identifier_constant`：`Compiler.cc:491-494`（最常见的重复来源）

**需修改的文件**

- `src/CompilerImpl.hh` — 添加 string 常量去重缓存成员
- `src/Compiler.cc` — 在 `make_constant` 或 `identifier_constant` 中查缓存

**实施步骤**

1. **`src/CompilerImpl.hh`**：在 `Compiler` 类中添加：
   ```cpp
   std::unordered_map<ObjString*, int> str_const_cache_;  // intern ptr → 常量池 index
   ```

2. **`src/Compiler.cc`** — `identifier_constant`（`Compiler.cc:491`）：
   ```cpp
   int Compiler::identifier_constant(const Token& name) noexcept {
     ObjString* interned = VM::get_instance().copy_string(
         name.lexeme.data(), name.lexeme.length());
     // 查缓存
     auto it = str_const_cache_.find(interned);
     if (it != str_const_cache_.end()) return it->second;
     // 未命中：正常添加
     int idx = make_constant(Value::make_object(interned));
     str_const_cache_[interned] = idx;
     return idx;
   }
   ```
   注意：数字字面量（integer/double）不需要去重（通常本就不重复），可以不处理。
   字符串字面量（`"foo"` 这种 literal）也可按同样方式加缓存，但收益较小。

3. 编译器的 `str_const_cache_` 在 `end_compiler()` 时自然随 `Compiler` 对象销毁，无需额外清理。
   注意：嵌套函数的 `Compiler` 实例各自独立，缓存只在同一函数作用域内有效（这正是期望行为）。

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 所有测试通过，保证去重不改变行为
# 可选：在编译期打印常量池大小，确认数量减少
# 测试重点：tests/globals.ms, tests/functions.ms（大量全局变量引用）
```

**风险**

- 极低风险：只影响常量池 slot 数量，不影响语义
- 需注意：相同 slot 被多处引用时，如果有 patch 需求（如 jump offset backpatch），需确保 patch 逻辑不依赖 slot 唯一性（检查确认：Chunk 的 backpatch 是对 code 数组打补丁，与常量池无关）

---

### `[ ]` OPT-P1-04：GETPROP/SETPROP/INVOKE 直接编码 IC 索引

**目标**

当前每次属性访问编译为 2 条指令：`OP_GETPROP(A,B)` + `OP_EXTRAARG(ic_slot)`。
`OP_EXTRAARG` 唯一作用是传递 IC 槽位索引（0-255 范围足够）。
将 IC 索引编码进 `OP_GETPROP` 的 C 字段，减少指令总数约 50%（对 class-heavy 代码）。

**背景数据**

- `OP_GETPROP` 当前格式：iABC，A=dest reg，B=object reg，C=name constant idx（RK encoded）
- EXTRAARG 消费：`VM.cc:550-551`（GETPROP）、`VM.cc:656-657`（SETPROP）、`VM.cc:1176-1177`（INVOKE）
- IC 槽位数量：`ic_.size()` 通常远小于 255（每个 function 独立计数）
- 编译器发射：`CompilerExpr.cc`（dot 访问）、`CompilerStmt.cc`（method call）

**需修改的文件**

- `src/CompilerExpr.cc` — 修改 dot_access 和方法调用的 emit 逻辑
- `src/CompilerStmt.cc` — 修改 method call 的 emit 逻辑
- `src/VM.cc` — GETPROP/SETPROP/INVOKE handlers：从 C 字段读 IC index，删除 READ_INSTR()
- `src/Debug.cc` — 反汇编器：更新 GETPROP/SETPROP/INVOKE 显示格式
- `src/Serializer.cc` — 若序列化逐字存储指令，则行为自动正确（无需改动）

**实施步骤**

1. **确认指令格式可行**：iABC 中 C 字段 8 位（值 0-255），当前 C 字段存放 name 的 RK 编码（RK: 0-127 = 寄存器，128-255 = 常量池）。
   因此 C 字段已被 name 占用，不能直接放 IC index。
   **调整方案**：将 name 移到 Bx 字段（16 bit），使用 iABx 格式（A=dest，Bx 高 8 位=name const idx，低 8 位=IC slot）：
   ```
   OP_GETPROP: iABx   [Bx:16 = (name_const:8 | ic_slot:8)][A:8][Op:8]
   ```
   这样 name 常量池 index 限制为 256（通常函数内 name 种类远小于此），IC slot 0-255。
   若 name index 超过 255，回退到旧的双指令格式（用 ic_slot=0xFF 作为"使用 EXTRAARG"标志）。

2. **`src/CompilerExpr.cc`**（dot access emit）：
   ```cpp
   // 发射 GETPROP：将 name_const (8bit) 和 ic_slot (8bit) 打包进 Bx
   int ic_slot = current_function->add_ic();  // 注册新 IC slot
   if (name_const <= 0xFF && ic_slot <= 0xFE) {
     u16_t bx = (static_cast<u16_t>(name_const) << 8) | static_cast<u16_t>(ic_slot);
     emit_ABx(OpCode::OP_GETPROP, dest, bx);
   } else {
     // 回退：旧双指令格式
     emit_ABC(OpCode::OP_GETPROP, dest, name_rk, 0xFF);
     emit_ABx(OpCode::OP_EXTRAARG, 0, ic_slot);
   }
   ```

3. **`src/VM.cc`** — GETPROP handler（`VM.cc:~548`）：
   ```cpp
   // 原来：读 EXTRAARG 获取 ic_slot
   // Instruction extra = READ_INSTR(); u8_t ic_slot = decode_Bx(extra);
   // 改为：从 Bx 的低 8 位提取 ic_slot
   u16_t bx = decode_Bx(instr);
   u8_t name_const = static_cast<u8_t>(bx >> 8);
   u8_t ic_slot    = static_cast<u8_t>(bx & 0xFF);
   if (ic_slot == 0xFF) {
     // 旧格式回退：读 EXTRAARG
     Instruction extra = READ_INSTR();
     ic_slot = static_cast<u8_t>(decode_Bx(extra));
   }
   ```
   SETPROP 和 INVOKE 同样处理。

4. **`src/Debug.cc`**：更新 GETPROP/SETPROP/INVOKE 的反汇编输出，从 Bx 中解析 name 和 ic_slot 并分别显示。

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_PRINT=ON && cmake --build build
# 运行 class 测试，观察反汇编输出，确认 GETPROP 不再紧跟 EXTRAARG
mslang tests/inline_cache.ms   # 应正确通过
python tests/run_tests.py
mslang --benchmark 5 benchmarks/field_access.ms    # 预期有明显提速
mslang --benchmark 5 benchmarks/method_dispatch.ms
# 回退路径压力测试：构造一个函数内有 >256 个不同属性名的访问（自动生成代码场景）
# 确认 ic_slot=0xFF 回退路径正确生成双指令格式
```

**风险**

- 指令格式变化需要同步更新序列化（Serializer）；已有的 `.msc` 缓存文件使用旧格式，必须递增版本号：
  在 `Serializer.hh` 中将 `kMSC_VERSION_MINOR` +1（当前为 1.1），使旧缓存自动失效。
- Bx 中 name_const 限 8 位（0-255）——若单个函数访问超过 255 个不同属性名，自动回退到双指令格式
  （`ic_slot=0xFF` 为标记，下一条 EXTRAARG 携带真实 ic_slot）。需验证回退路径在压力测试中正确工作。

---

## P1（新增） — 重要改进（补充任务）

---

### `[ ]` OPT-P1-05：ObjMap 迭代 O(n²) 修复

**目标**

`OP_FORITER`（`VM.cc:1737-1747`）对 `ObjMap` 的迭代使用 `std::advance(it, idx)` 模式，
每次迭代都从头部遍历 `idx` 步，导致总时间复杂度 O(n²)。在 P2-04（ObjMap 开放地址）实施前，
提供一个独立的临时修复。

**背景数据**

- `VM.cc:1743-1744`：`auto it = entries.begin(); std::advance(it, idx);`
- `ObjMap::entries()` 返回 `std::unordered_map<Value,Value,...>&`，迭代器只支持 forward traversal
- 对 1000 元素的 map，总迭代 500K 步（O(n²)），性能极差

**需修改的文件**

- `src/Object.hh` — 在 `ObjMap` 中添加 `keys_` 快照辅助成员（懒建）
- `src/VM.cc` — `OP_FORITER` OBJ_MAP 分支

**实施步骤**

1. **`src/Object.hh`** — 在 `ObjMap` 中添加迭代辅助：
   ```cpp
   class ObjMap final : public Object {
     ValueMap entries_;
     // 迭代器快照：首次 FORITER 时构建，map 修改后失效
     mutable std::vector<Value> iter_keys_;
     mutable bool iter_dirty_{true};
   public:
     const std::vector<Value>& iter_snapshot() const noexcept {
       if (iter_dirty_) {
         iter_keys_.clear();
         for (auto& [k, _] : entries_) iter_keys_.push_back(k);
         iter_dirty_ = false;
       }
       return iter_keys_;
     }
     void mark_dirty() noexcept { iter_dirty_ = true; }
   };
   ```
   所有修改 map 的操作（`map_set`, `map_del`，`VMBuiltins.cc`）调用 `map->mark_dirty()`。

2. **`src/VM.cc`** — `OP_FORITER` OBJ_MAP 分支（`VM.cc:1737`）：
   ```cpp
   const auto& keys = map->iter_snapshot();
   if (idx >= static_cast<int>(keys.size())) {
     frame->ip += sBx;
   } else {
     frame->slots[A + 2] = keys[static_cast<sz_t>(idx)];  // O(1) 随机访问
     index_val = Value(static_cast<double>(idx + 1));
   }
   ```

**注意**：OPT-P2-04（ObjMap 开放地址）完成后，此临时方案可移除——开放地址表支持直接下标访问，
迭代自然为 O(n)。P2-04 的验证步骤中应确认 map 迭代不再是 O(n²)。

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 添加迭代大型 map 的性能测试脚本（1000 个键），确认时间接近 O(n)
```

---

### `[ ]` OPT-P1-06：ASCII 单字符 ObjString 缓存

**目标**

字符串迭代（`VM.cc:1733-1734`）每次调用 `copy_string(char_ptr, 1)` 分配一个新 ObjString。
虽然 intern 机制确保相同字符只存一份，但**首次**遍历 ASCII 字符串仍会分配最多 128 个单字符对象，
每次 GC 都需追踪它们，并在第一次迭代时触发大量小对象分配。
预缓存 128 个 ASCII 单字符 ObjString，字符串迭代变为 O(1) 无分配。

**需修改的文件**

- `src/VM.hh` — 添加 `ascii_char_cache_[128]` 成员
- `src/VM.cc` — VM 初始化时创建缓存；`copy_string` 对 length==1 && char<128 直接返回缓存

**实施步骤**

1. **`src/VM.hh`**：
   ```cpp
   ObjString* ascii_char_cache_[128]{};  // ASCII 单字符 ObjString 缓存
   ```

2. **`src/VM.cc`** — VM 构造（或 `init()` 函数）中：
   ```cpp
   for (int c = 0; c < 128; ++c) {
     char buf[1] = {static_cast<char>(c)};
     ascii_char_cache_[c] = copy_string(buf, 1);  // intern + 分配，永久驻留
   }
   ```
   这 128 个对象作为 GC 根（永久 old gen），在 `mark_roots` 中标记。

3. **`src/VM.cc`** — `copy_string` 快速路径：
   ```cpp
   ObjString* VM::copy_string(cstr_t chars, sz_t length) noexcept {
     if (length == 1 && static_cast<u8_t>(chars[0]) < 128 && ascii_char_cache_[chars[0]])
       return ascii_char_cache_[chars[0]];
     // ...原有逻辑
   }
   ```

4. **`src/VMGC.cc`** — `mark_roots`：
   ```cpp
   for (ObjString* s : ascii_char_cache_)
     if (s) mark_object(s);
   ```

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 验证字符串迭代（for c in "hello"）正常工作
# 验证 ASCII 缓存对象不被 GC 回收
```

---

### `[ ]` OPT-P1-07：ObjClosure 内联 upvalue 数组

**目标**

`ObjClosure::upvalues_`（`std::vector<ObjUpvalue*>`）的元素数量在编译期已知
（`function->upvalue_count()`），但仍使用 vector（24B 头部 + 堆分配）。
改为 flexible array member，每次创建闭包消除一次堆分配。

**背景数据**

- `Object.hh:271`：`std::vector<ObjUpvalue*> upvalues_`
- `ObjClosure` 构造：`Object.cc` — `upvalues_.resize(function->upvalue_count())`
- 访问模式：`upvalues_[i]` 随机访问，无扩容需求

**依赖**：OPT-P0-01（Object 布局稳定后再改 ObjClosure）

**需修改的文件**

- `src/Object.hh` — 修改 ObjClosure 类定义，移除 `std::vector<ObjUpvalue*>`
- `src/Object.cc` — 修改构造、trace_references、size；添加 `ObjClosure::create`
- `src/VM.cc` — 分配调用改为 `ObjClosure::create(function)`

**实施步骤**

1. **`src/Object.hh`** — 修改 `ObjClosure`：
   ```cpp
   class ObjClosure final : public Object {
     ObjFunction* function_{nullptr};
     int upvalue_count_{0};
     // upvalues_[] 紧跟末尾（flexible array member 模式）
   public:
     static ObjClosure* create(ObjFunction* function) noexcept;
     ObjUpvalue*& upvalue(int idx) noexcept {
       return reinterpret_cast<ObjUpvalue**>(this + 1)[idx];
     }
     ObjUpvalue* upvalue(int idx) const noexcept {
       return reinterpret_cast<ObjUpvalue* const*>(this + 1)[idx];
     }
     int upvalue_count() const noexcept { return upvalue_count_; }
     ObjFunction* function() const noexcept { return function_; }
   };
   ```

2. **`src/Object.cc`** — 实现 `ObjClosure::create`：
   ```cpp
   ObjClosure* ObjClosure::create(ObjFunction* function) noexcept {
     int n = function->upvalue_count();
     void* mem = ::operator new(sizeof(ObjClosure) + n * sizeof(ObjUpvalue*));
     ObjClosure* c = ::new(mem) ObjClosure();
     c->function_ = function;
     c->upvalue_count_ = n;
     auto** upvalues = reinterpret_cast<ObjUpvalue**>(c + 1);
     for (int i = 0; i < n; ++i) upvalues[i] = nullptr;
     return c;
   }
   ```

3. **`src/Object.cc`** — `trace_references`：遍历 `upvalue(0..upvalue_count_-1)`，调用 `mark_object`

4. **`size`**：`sizeof(ObjClosure) + upvalue_count_ * sizeof(ObjUpvalue*)`

5. **`src/VM.cc`** — 将所有 `allocate<ObjClosure>(function)` 替换为 `ObjClosure::create(function)`

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# 重点：tests/closures.ms, tests/upvalues.ms
mslang --benchmark 5 benchmarks/binary_trees.ms  # 闭包密集场景
```

---

## P2 — 中等优先级

---

### `[ ]` OPT-P2-01：ObjClass 稀用 Table 按需分配

**目标**

`ObjClass` 持有 5 张 `Table`：`methods_`、`static_methods_`、`getters_`、`setters_`、`abstract_methods_`。
其中 `getters_`、`setters_`、`abstract_methods_` 在绝大多数用户定义类中为空。
将这三个 Table 改为 `std::unique_ptr<Table>`，空时仅占 8 字节（指针），减少约 72 字节/类。

**需修改的文件**

- `src/Object.hh` — 修改 ObjClass 三个成员类型
- `src/Object.cc` — 修改构造/析构/trace_references/size
- `src/VM.cc` — 通过 getter/setter/abstract 的访问点需要 null check（或懒初始化）
- `src/VMCall.cc` — 同上

**实施步骤**

1. **`src/Object.hh`**：
   ```cpp
   class ObjClass final : public Object {
     ObjString* name_{nullptr};
     Table methods_;
     Table static_methods_;
     std::unique_ptr<Table> getters_;        // 懒分配
     std::unique_ptr<Table> setters_;        // 懒分配
     std::unique_ptr<Table> abstract_methods_; // 懒分配
     ...
   public:
     Table& getters() noexcept {
       if (!getters_) getters_ = std::make_unique<Table>();
       return *getters_;
     }
     bool has_getters() const noexcept { return getters_ != nullptr; }
     // setters_, abstract_methods_ 同样方式处理
   };
   ```

2. **`src/VM.cc`** — 所有访问 `klass->getters()` / `klass->setters()` 的地方：
   读操作前先检查 `klass->has_getters()`（避免懒初始化带来的空 Table 创建）；
   写操作（`OP_METHOD` 注册 getter 时）直接调用 `klass->getters()`（触发懒初始化）。

3. **`src/Object.cc`** — `trace_references`：
   ```cpp
   if (getters_) getters_->mark_table();
   if (setters_) setters_->mark_table();
   if (abstract_methods_) abstract_methods_->mark_table();
   ```

4. **`size()`**：
   ```cpp
   return sizeof(ObjClass)
     + (getters_ ? getters_->capacity() * sizeof(Table::Entry) : 0)
     + ...;
   ```

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 重点：tests/getter_setter.ms, tests/abstract_methods.ms, tests/classes.ms
```

---

### `[ ]` OPT-P2-02：Shape 使用小型线性映射替代 unordered_map

**目标**

`Shape::slots_` 和 `Shape::transitions_` 均为 `std::unordered_map`，
对典型的 2-5 字段类，每个 map entry 有 ~50 字节节点开销。
对 ≤8 条目的情形改为线性扫描的 flat array，利用 ObjString 指针 intern 保证 O(1) 指针比较。

**需修改的文件**

- `src/Object.hh` — 修改 Shape 类定义
- `src/Object.cc` — 修改 find_slot、add_transition、mark_keys、析构

**实施步骤**

1. **`src/Object.hh`** — 定义 SmallMap 辅助结构并替换 Shape 成员：
   ```cpp
   template <typename K, typename V, int N = 8>
   struct SmallMap {
     struct Entry { K key; V value; };
     Entry  data[N]{};
     int    count{0};

     // 超过 N 条目时切换到 heap map；用 unique_ptr 确保自动析构，无泄漏风险
     std::unique_ptr<std::unordered_map<K,V>> extra;

     bool overflow() const noexcept { return extra != nullptr; }

     V* find(K key) noexcept { /* 线性扫描 data[0..count-1]，overflow 时查 *extra */ }
     void insert(K key, V val) noexcept {
       /* count < N：写入 data；count == N：extra = make_unique<...>，移入所有 data，再插入 */
     }
     void mark_keys() noexcept { /* 遍历所有 key，调用 mark_object */ }
     // 默认析构即可：unique_ptr 自动释放 extra
   };

   class Shape {
     u32_t id_;
     SmallMap<ObjString*, u32_t>  slots_;        // 属性名 → slot index
     SmallMap<ObjString*, Shape*> transitions_;  // 属性名 → 子 Shape
     u32_t slot_count_{0};
   };
   ```
   > Shape 不是 GC 管理对象，由 ObjClass 析构时递归 delete。SmallMap 使用 `unique_ptr` 确保
   > overflow 堆内存在 Shape 析构时自动释放，无需手动管理。

2. **`src/Object.cc`**：
   - `find_slot`：调用 `slots_.find(name)`
   - `add_transition`：调用 `transitions_.insert(name, child)`
   - `mark_keys`：调用 `slots_.mark_keys()` 和 `transitions_.mark_keys()`

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 重点：tests/classes.ms, tests/inline_cache.ms
mslang --benchmark 5 benchmarks/field_access.ms
```

---

### `[ ]` OPT-P2-03：Remembered Set 去重

**目标**

`VM::remember(Object*)` 在 `VMGC.cc:98` 直接 push_back，不检重复。
反复修改的老生代对象（如全局 list 频繁 append）会被多次加入 remembered set，
minor GC 时重复追踪相同对象，浪费时间。

**需修改的文件**

- `src/Object.hh` — 在 Object 基类中添加 `in_remembered_` 位（利用现有 padding）
- `src/VMGC.cc` — `remember()` 中检查该位；`remembered_set_.clear()` 时重置所有位

**实施步骤**

1. **`src/Object.hh`**：在 Object 中，`age_`（u8_t）后面有 padding。
   添加 `bool in_remembered_{false};`（利用现有对齐空间，不增加 Object 大小）：
   ```cpp
   class Object {
     ObjectType type_;          // 4B
     bool is_marked_{false};    // 1B
     bool finalized_{false};    // 1B
     GcGeneration generation_;  // 1B
     u8_t age_{0};              // 1B
     bool in_remembered_{false};// 1B（现有 padding 中）
     // 3B padding
     Object* next_{nullptr};    // 8B
   };
   ```
   添加访问器：`bool in_remembered() const noexcept { return in_remembered_; }`
   和 `void set_in_remembered(bool v) noexcept { in_remembered_ = v; }`

2. **`src/VMGC.cc`** — `VM::remember(Object* object)`（`VMGC.cc:98`）：
   ```cpp
   void VM::remember(Object* object) noexcept {
     if (object->in_remembered()) return;  // 已在集合中
     object->set_in_remembered(true);
     remembered_set_.push_back(object);
   }
   ```

3. **`src/VMGC.cc`** — minor GC 末尾 `remembered_set_.clear()`（`VMGC.cc:345`）：
   在 clear 之前重置所有对象的标志：
   ```cpp
   for (Object* obj : remembered_set_)
     obj->set_in_remembered(false);
   remembered_set_.clear();
   ```

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# 重点：tests/gc.ms（老生代写屏障测试）
```

---

### `[ ]` OPT-P2-04：ObjMap 改为开放地址哈希表

**目标**

`ObjMap` 使用 `std::unordered_map<Value,Value,...>`，链式哈希，每个 entry 独立堆分配节点。
替换为开放地址哈希（参考项目内已有的 `Table` 实现），连续内存布局，
GC trace 变为线性扫描，cache 友好。

**需修改的文件**

- `src/Object.hh` — 定义 `ValueTable`（开放地址）替换 `ValueMap`
- `src/Object.cc` — 实现 `ValueTable` 操作，修改 ObjMap
- `src/VMBuiltins.cc` — 所有 ObjMap 操作的调用点（set/get/del/keys/values/len/contains）

**实施步骤**

1. **`src/Object.hh`**：定义 `ValueTable`（类似 `Table` 但 key/value 均为 `Value`）：
   ```cpp
   struct ValueEntry { Value key{}; Value value{}; bool tombstone{false}; };

   class ValueTable {
     std::vector<ValueEntry> entries_;
     int count_{0};
     static constexpr double kMAX_LOAD = 0.75;
   public:
     bool   get(const Value& key, Value* out) const noexcept;
     bool   set(const Value& key, Value val) noexcept;   // 返回 true=新增
     bool   del(const Value& key) noexcept;
     void   mark_entries() noexcept;  // GC：遍历标记 key/value
     int    count() const noexcept { return count_; }
   private:
     ValueEntry* find_entry(const Value& key) noexcept;
     void   grow() noexcept;
   };
   ```

2. **`src/Object.cc`**：实现 `ValueTable`，使用 `ValueHash`（已有）计算 hash，
   线性探测，`del` 使用 tombstone（或 backward-shift），`grow` 触发 2x 扩容重建。

3. **`src/Object.hh`** — 修改 `ObjMap`：
   ```cpp
   class ObjMap final : public Object {
     ValueTable entries_;   // 替换 ValueMap
   public:
     ValueTable& entries() noexcept { return entries_; }
     sz_t len() const noexcept { return entries_.count(); }
   };
   ```

4. **`src/VMBuiltins.cc`**：更新所有 ObjMap 操作调用，从 `entries_.find/insert/erase/size`
   改为 `entries_.get/set/del/count`。

5. **`trace_references`**：调用 `entries_.mark_entries()`（线性扫描）。

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 重点：tests/maps.ms（若存在），测试 set/get/del/keys/values/len
```

---

### `[ ]` OPT-P2-05：Coroutine SavedCallFrame 直接保存/恢复

**目标**

当前 yield/resume 时，`CallFrame`（含 `Value* slots` 绝对指针）被保存为 `SavedCallFrame`
（含 `ptrdiff_t slots_offset` 相对偏移）。每次恢复需要将相对偏移重新计算为绝对指针，
且 `SavedCallFrame::deferred` 是 `std::vector<ObjClosure*>` 的完整拷贝。

本任务消除 `slots_offset` ↔ 绝对指针的转换开销，并消除 deferred vector 的拷贝：
直接用 `CallFrame` 的 raw bytes（固定大小）序列化到 `saved_frames_`，
恢复时直接 memcpy 回来并修正 `slots` 指针（基地址 + 已保存的 offset）。

**背景数据**

- `SavedCallFrame` 定义：`Object.hh:498-505`，含 slots_offset、deferred vector
- OP_YIELD 保存：`VM.cc:1826-1848`（逐字段构造 SavedCallFrame）
- OP_RESUME 恢复：`VM.cc:1906+`（逐字段还原 CallFrame）

**需修改的文件**

- `src/Object.hh` — 简化 `SavedCallFrame`，直接存储 slots_offset（u32_t）而非 ptrdiff_t 冗余类型
- `src/VM.cc` — OP_YIELD / OP_RESUME：简化保存/恢复逻辑，删除 deferred vector 的拷贝

**实施步骤**

1. **`src/Object.hh`** — 简化 `SavedCallFrame`（不再需要整个 vector 拷贝）：
   ```cpp
   struct SavedCallFrame {
     ObjClosure*  closure{nullptr};
     Instruction* ip{nullptr};
     u32_t        slots_offset{0};   // stack_.data() 的下标，恢复时 stack_.data() + offset
     Value        pending_return{};
     bool         returning{false};
     // deferred 不拷贝——yield 时 deferred 必须为空（Maple 语义保证 defer 在 return 前执行）
   };
   ```

2. **`src/VM.cc`** — OP_YIELD（`VM.cc:1826`）：
   ```cpp
   SavedCallFrame sf;
   sf.closure  = f.closure;
   sf.ip       = f.ip;
   sf.slots_offset = static_cast<u32_t>(f.slots - stack_.data());
   sf.pending_return = f.pending_return;
   sf.returning = f.returning;
   // assert(f.deferred.empty()); defer 必须已执行完
   coro->saved_frames().push_back(sf);
   ```

3. **`src/VM.cc`** — OP_RESUME 恢复（`VM.cc:1906`）：
   ```cpp
   CallFrame& f = frames_[frame_count_++];
   f.closure  = sf.closure;
   f.ip       = sf.ip;
   f.slots    = stack_.data() + sf.slots_offset;
   f.pending_return = sf.pending_return;
   f.returning = sf.returning;
   f.deferred.clear();   // 已执行完，无需恢复
   ```

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# 重点：tests/coroutines.ms
```

**注意**：Coroutine 独立栈段（彻底的 O(1) yield/resume）移至 **OPT-P3-04**（低优先级），
因为它需要将 VM 主栈和 coroutine 栈分离，影响数百处栈访问路径，复杂度极高。

---

### `[ ]` OPT-P2-06：Peephole NOP 指令压缩

**目标**

`Optimize.cc` 的 peephole 优化器用 `OP_NOP` 替换死代码，但不移除这些 NOP。
NOP 残留在字节码中，每次经过 dispatch 循环时仍需解码和跳过，浪费 I-cache。
添加 compaction pass，移除所有 NOP 并修正跳转偏移。

**背景数据**

- NOP 产生位置：`Optimize.cc:84,105,117,150,171`
- VM 的 NOP handler：`VM.cc:1803`（`VM_CASE(OP_NOP) { VM_DISPATCH(); }`）
- Chunk 中跳转指令使用相对偏移（sBx 字段），NOP 移除后偏移需修正

**需修改的文件**

- `src/Chunk.hh` — 添加 `compact_nops()` 方法声明
- `src/Chunk.cc` — 实现 compaction pass
- `src/Optimize.cc` — 在 peephole pass 后调用 `chunk.compact_nops()`

**实施步骤**

1. **`src/Chunk.cc`** — 实现 `compact_nops()`：
   - 第一遍：构建 `offset_map[old_offset] = new_offset`（跳过所有 NOP）
   - 第二遍：复制非 NOP 指令到新序列；对所有跳转指令（OP_JUMP, OP_JUMP_IF_FALSE, OP_FORITER 等）
     重新计算 sBx = `offset_map[target] - current_new_offset - 1`
   - 同步更新 `lines_`（RLE 行信息）：NOP 指令对应的 run 计数减少

2. **IC slot 偏移问题**：IC slot 存储在 `ObjFunction::ic_` 中，通过 ic_index 而非字节码偏移访问，
   不受 compaction 影响。确认：`OP_GETPROP/SETPROP/INVOKE` 的 ic_slot 是 ic_ 向量的索引，
   与指令在 code_ 中的位置无关。

3. **需要处理的跳转指令**：扫描 `Opcode.hh`，找出所有使用相对跳转（sBx 字段）的指令
   （OP_JUMP, OP_JUMP_IF_FALSE, OP_JUMP_IF_TRUE, OP_FORITER, OP_LOOP 等），
   在 compaction 时修正其 sBx。

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_PRINT=ON && cmake --build build
# 运行任意脚本，确认反汇编输出中不再含 NOP（或极少出现）
# 确认跳转目标正确（如 if/else/while/for 循环）
python tests/run_tests.py
# 重点：tests/control_flow.ms, tests/loops.ms, tests/coroutines.ms
```

---

## P3 — 低优先级改进

---

### `[ ]` OPT-P3-01：CallFrame deferred 延迟分配

**目标**

`CallFrame::deferred`（`std::vector<ObjClosure*>`）在绝大多数帧中为空，但占 24 字节固定成本。
64 个预分配帧共浪费 1.5 KB。改为延迟分配。

**需修改的文件**

- `src/VM.hh` — 修改 CallFrame 结构
- `src/VM.cc` — OP_DEFER 和 OP_RETURN 处理
- `src/VMGC.cc` — mark_roots 中遍历 deferred
- `src/VMCall.cc` — clear deferred on new frame

**实施步骤**

1. **`src/VM.hh`**：
   ```cpp
   struct CallFrame {
     ObjClosure*  closure{nullptr};
     Instruction* ip{nullptr};
     Value*       slots{nullptr};
     Value        pending_return{};
     bool         returning{false};
     // 替换 std::vector<ObjClosure*> deferred
     ObjClosure** deferred_buf{nullptr};   // heap 分配，按需创建
     u8_t         deferred_count{0};
     u8_t         deferred_capacity{0};
   };
   ```

2. **`src/VM.cc`** — OP_DEFER（`VM.cc:1796`）：
   若 `deferred_buf == nullptr`，`new ObjClosure*[4]`（初始容量 4）；
   若已满，realloc（容量 *2）。

3. **VMGC.cc** — mark_roots：改为遍历 `frames_[i].deferred_buf[0..deferred_count-1]`。

4. **VMCall.cc** — reset frame 时：`delete[] frame.deferred_buf; frame.deferred_buf = nullptr; frame.deferred_count = 0;`

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 重点：tests/defer.ms（若存在 defer 语法测试）
```

---

### `[ ]` OPT-P3-02：Quickening deopt 计数器

**目标**

当前 quickened 指令（`OP_ADD_II` 等）在类型不符时直接 revert 到 generic 形式，
下次执行又会 quicken。多态调用点无限循环 quicken→deopt→quicken。
添加 deopt 计数器，超过阈值（3 次）后永久锁定为 generic。

**背景数据**

- 无 deopt 计数器（VMGC.cc/VM.cc 确认）
- deopt 发生点：`VM.cc:1949,1963,1978,2002,2014,2026,2038,2050,2062,2074`（各 quickened handler）

**需修改的文件**

- `src/Object.hh` — 在 `ICEntry` 中添加 `deopt_count` 字段（复用现有结构，零额外分配）
- `src/VM.cc` — deopt 点递增计数并在阈值时锁定

**实施步骤**

1. **`src/Object.hh`** — 在 `ICEntry` 中添加（利用现有 padding，零内存增加）：
   ```cpp
   struct ICEntry {
     ObjClass* klass{nullptr};
     ICKind    kind{ICKind::UNINITIALIZED};
     Value     cached{};
     u32_t     shape_id{0};
     u32_t     slot_index{0};
     u8_t      deopt_count{0};  // 新增：deopt 计数，≥3 时锁定为 generic
     // padding 对齐不变（原本有 padding）
   };
   ```
   在 `ObjFunction` 中添加方法：
   ```cpp
   u8_t increment_deopt(u8_t ic_slot) noexcept {
     if (ic_slot < ic_.size()) return ++ic_[ic_slot].deopt_count;
     return 255;  // 超出范围，视为已锁定
   }
   ```
   注意：quickened 指令（OP_ADD_II 等）不使用 IC slot，需从字节码偏移映射到一个专用 slot 或
   使用独立的 `std::vector<u8_t> arith_deopt_` 数组（每字节码位置一个计数，按需分配）。

2. **`src/VM.cc`** — 所有 deopt 点（如 `VM.cc:1949`）：
   ```cpp
   // 原来：直接 revert opcode
   // const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
   // 改为：
   u32_t instr_offset = static_cast<u32_t>(
       frame->ip - 1 - frame->closure->function()->chunk().code().data());
   u8_t count = frame->closure->function()->increment_arith_deopt(instr_offset);
   if (count < 3) {
     // 未锁定：revert，下次可再 quicken
     const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
   }
   // count >= 3：不 revert，保持 quickened 形式但执行 generic 逻辑
   // 后续每次执行此 quickened 指令都会 deopt 并执行 generic 逻辑（稳定状态）
   ```
   在 `ObjFunction` 中添加 `std::vector<u8_t> arith_deopt_`（按 bytecode offset 索引，懒分配）。

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 重点：tests/quickening.ms
# 可用混合类型调用测试多态场景
```

---

### `[ ]` OPT-P3-03：编译器局部变量数组动态化

**目标**

`CompilerImpl.hh:148/150` 中 `Local locals_[256]` 和 `Upvalue upvalues_[256]` 是固定栈数组。
每层嵌套函数作用域在 C++ 调用栈上消耗约 10 KB。
深度嵌套（10+ 层）消耗 100 KB+，存在栈溢出风险。
改为 `std::vector`，按需增长，空间占用从 10 KB 降到 48 字节（两个空 vector）。

**需修改的文件**

- `src/CompilerImpl.hh` — 修改 Local/Upvalue 数组类型
- `src/Compiler.cc` — 所有 `locals_[i]` / `upvalues_[i]` 访问，local_count_ 相关逻辑

**实施步骤**

1. **`src/CompilerImpl.hh`**：
   ```cpp
   // 原来
   Local   locals_[kUINT8_COUNT];
   Upvalue upvalues_[kUINT8_COUNT];
   // 改为
   std::vector<Local>   locals_;    // constructor: locals_.reserve(16)
   std::vector<Upvalue> upvalues_;  // constructor: upvalues_.reserve(8)
   ```

2. **`src/Compiler.cc`** — 所有 `locals_[local_count_]` 改为 `locals_.emplace_back()` 或直接 index 访问；
   删除边界检查（`local_count_ == 256`）改为 `locals_.size() == 256` 仍然保留语义错误报告；
   所有 `upvalues_[upvalue_count_]` 同样处理。

3. 构造函数中：`locals_.reserve(16); upvalues_.reserve(8);`

**验证**

```bash
cmake --build build
python tests/run_tests.py
# 所有测试通过即可；可用深度嵌套 closure 脚本验证不再栈溢出
```

---

### `[ ]` OPT-P3-04：Coroutine 独立栈段（O(1) yield/resume）

**目标**

在 OPT-P2-05 基础上进一步优化：给每个 coroutine 分配独立的栈缓冲区，
yield/resume 变为指针交换，实现真正的 O(1)。

**依赖**：OPT-P2-05（SavedCallFrame 已简化）

**背景数据**

- 当前 yield 仍需拷贝 `saved_stack_`（`std::vector<Value>`）和 `saved_frames_`
- 独立栈方案：每个 coroutine 持有自己的 `Value stack_[kCoroStackSize]` 和
  `CallFrame frames_[kCoroFrameMax]`，VM 切换时只交换几个指针

**需修改的文件**

- `src/Object.hh` — 修改 ObjCoroutine，添加独立栈缓冲区
- `src/VM.cc` — OP_YIELD / OP_RESUME：指针交换而非数据拷贝
- `src/VMGC.cc` — mark_roots 需追踪 coroutine 独立栈上的 Value

**⚠️ 重要提示**

独立栈方案需要 VM 的 `stack_top_`、`frames_` 等指针在 yield 时切换到不同缓冲区。
所有通过 `stack_top_` 访问栈的代码路径（数百处）必须能透明地操作当前活跃的栈缓冲区。
建议实现前先确保 coroutine 相关的测试覆盖率达 100%（`tests/coroutines.ms` + 边界场景）。

**验证**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# 重点：tests/coroutines.ms
# 性能对比：深度 coroutine 嵌套 yield/resume 基准测试
```

---

## 附录：验证命令速查

```bash
# 构建（Debug）
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build

# 构建（Release，用于性能对比）
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

# GC 压力测试
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build

# 完整测试
ctest --test-dir build --output-on-failure
python tests/run_tests.py

# 单个脚本
./build/mslang tests/classes.ms
./build/mslang tests/gc.ms
./build/mslang tests/coroutines.ms

# 基准测试
./build/mslang --benchmark 5 benchmarks/binary_trees.ms
./build/mslang --benchmark 5 benchmarks/field_access.ms
./build/mslang --benchmark 5 benchmarks/method_dispatch.ms
```
