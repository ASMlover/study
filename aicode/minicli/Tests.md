# Task Test Methods

## Purpose
- Record test methods and evidence after each task is implemented.
- Make testing repeatable across Windows and Linux.

## Update Rules
- Update this file immediately after completing each `Txx`.
- Include command, environment, expected result, and actual result.
- Attach failure notes and fix reference when a test fails.

## Global Test Baseline
- Unit tests: include normal, error, and boundary cases.
- Integration tests: verify module interactions for the task scope.
- E2E smoke: run at key flow points (`startup`, `chat`, `slash`, `run`).
- Coverage target:
  - Core code introduced by the task: `>= 90%`
  - Overall project: `>= 80%`

## Command Template
```bash
# type check
npm run typecheck

# unit
npm run test:unit

# integration
npm run test:integration

# e2e smoke
npm run test:e2e:smoke

# coverage
npm run test:coverage
```

## Task Test Record Template
Use this section structure for each task:

```md
### Txx - Task Name
- Date:
- Env: (Windows/Linux, Node version)
- Scope:
- Unit Cases:
  - [ ] case-1
  - [ ] case-2
- Integration Cases:
  - [ ] case-1
- E2E Smoke:
  - [ ] case-1
- Commands:
  - `...`
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result: PASS/FAIL
- Notes:
```

## Task Test Entries

### T01 - 初始化 CLI 工程骨架
- Date:
- Env:
- Scope: CLI init, entrypoint, `--version`
- Unit Cases:
  - [ ] 版本字符串格式正确
  - [ ] 参数解析默认值正确
  - [ ] 非法参数返回错误码
  - [ ] 空参数行为正确
  - [ ] 平台分支判断正确
  - [ ] `--version` 输出非空
- Integration Cases:
  - [ ] `minicli --version` 退出码为 0
- E2E Smoke:
  - [ ] 启动后二进制可执行
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T02 - REPL 空循环
- Date:
- Env:
- Scope: REPL loop, echo, Ctrl+C exit
- Unit Cases:
  - [ ] 输入解析
  - [ ] 空行忽略
  - [ ] 超长输入处理
  - [ ] EOF 处理
  - [ ] 中断信号处理
  - [ ] 输出缓冲处理
- Integration Cases:
  - [ ] 子进程输入输出回显一致
- E2E Smoke:
  - [ ] 启动 -> 输入 -> 退出
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T03 - /help 与 /exit
- Date:
- Env:
- Scope: command routing, help, exit
- Unit Cases:
  - [ ] 命令匹配
  - [ ] 未知命令提示
  - [ ] help 内容完整
  - [ ] exit 退出标志
  - [ ] 空白字符处理
  - [ ] 大小写策略
- Integration Cases:
  - [ ] `/help -> /exit` 链路
- E2E Smoke:
  - [ ] 交互退出成功
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T04 - 双层配置加载
- Date:
- Env:
- Scope: global/project config merge
- Unit Cases:
  - [ ] 全局路径解析
  - [ ] 项目路径解析
  - [ ] 覆盖优先级
  - [ ] 缺失文件降级
  - [ ] 空配置处理
  - [ ] 注释行处理
- Integration Cases:
  - [ ] 双配置覆盖结果正确
- E2E Smoke:
  - [ ] 启动时加载配置成功
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T05 - 配置校验与报错
- Date:
- Env:
- Scope: schema validation and errors
- Unit Cases:
  - [ ] 必填缺失
  - [ ] 字段类型错误
  - [ ] 非法枚举值
  - [ ] 未知字段处理
  - [ ] 默认值注入
  - [ ] 错误文案可读
- Integration Cases:
  - [ ] 损坏配置时可提示并继续
- E2E Smoke:
  - [ ] 错配配置下启动行为可预测
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T06 - LLMProvider 接口与 Mock
- Date:
- Env:
- Scope: provider contract and mock
- Unit Cases:
  - [ ] 接口契约校验
  - [ ] 请求结构校验
  - [ ] 返回结构校验
  - [ ] 异常传递
  - [ ] 空消息处理
  - [ ] 多轮消息拼装
- Integration Cases:
  - [ ] REPL 调用 mock provider 成功
- E2E Smoke:
  - [ ] mock 聊天流程可运行
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T07 - GLM OpenAI-compatible 通路
- Date:
- Env:
- Scope: glm provider http path
- Unit Cases:
  - [ ] 请求头构造
  - [ ] URL 拼接
  - [ ] 默认模型值
  - [ ] 响应映射
  - [ ] 空 choices 处理
  - [ ] 网络错误包装
- Integration Cases:
  - [ ] 请求 mock API 成功
- E2E Smoke:
  - [ ] provider 全链路可运行
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T08 - /login 与 /model
- Date:
- Env:
- Scope: key storage and model inspect
- Unit Cases:
  - [ ] key 保存读取
  - [ ] key 掩码显示
  - [ ] 空 key 拒绝
  - [ ] 模型回显
  - [ ] 配置优先级
  - [ ] 非法模型名处理
- Integration Cases:
  - [ ] `/login` 后 `/model` 可用
- E2E Smoke:
  - [ ] 登录后问答前置条件满足
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T09 - 单轮问答闭环
- Date:
- Env:
- Scope: one-turn chat path
- Unit Cases:
  - [ ] 输入分类
  - [ ] 请求构造
  - [ ] 回复渲染
  - [ ] 空回复降级
  - [ ] 多行输入处理
  - [ ] 输出截断处理
- Integration Cases:
  - [ ] 登录后问答成功
- E2E Smoke:
  - [ ] `login -> ask -> answer`
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T10 - 超时与错误映射
- Date:
- Env:
- Scope: timeout + 401/429/5xx handling
- Unit Cases:
  - [ ] 超时触发
  - [ ] 401 映射
  - [ ] 429 映射
  - [ ] 5xx 映射
  - [ ] 重试次数边界
  - [ ] 不可重试分支
- Integration Cases:
  - [ ] 429 重试后恢复
- E2E Smoke:
  - [ ] 异常提示可读
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T11 - SQLite 与 migration v1
- Date:
- Env:
- Scope: db init and schema
- Unit Cases:
  - [ ] migration 幂等
  - [ ] 表结构存在
  - [ ] 索引存在
  - [ ] 连接失败处理
  - [ ] 权限错误处理
  - [ ] 文件锁处理
- Integration Cases:
  - [ ] 首次启动自动建库
- E2E Smoke:
  - [ ] 启动后 DB 文件可用
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T12 - 消息持久化仓储
- Date:
- Env:
- Scope: session/message repository
- Unit Cases:
  - [ ] 新建会话
  - [ ] 写消息
  - [ ] 排序读取
  - [ ] 分页边界
  - [ ] 空结果处理
  - [ ] 事务回滚
- Integration Cases:
  - [ ] 写后读一致
- E2E Smoke:
  - [ ] 重启后消息仍可读取
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T13 - /new
- Date:
- Env:
- Scope: create and switch session
- Unit Cases:
  - [ ] 默认命名
  - [ ] 当前指针更新
  - [ ] 重名处理
  - [ ] 空标题处理
  - [ ] 时间戳生成
  - [ ] 参数解析
- Integration Cases:
  - [ ] `/new` 后消息写入新会话
- E2E Smoke:
  - [ ] 创建会话后可对话
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T14 - /sessions
- Date:
- Env:
- Scope: list sessions
- Unit Cases:
  - [ ] 排序规则
  - [ ] 空列表提示
  - [ ] 当前会话标记
  - [ ] 分页参数
  - [ ] 输出格式
  - [ ] 过滤条件
- Integration Cases:
  - [ ] 多会话展示正确
- E2E Smoke:
  - [ ] 会话列表可查看
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T15 - /switch
- Date:
- Env:
- Scope: switch by id/index
- Unit Cases:
  - [ ] id 切换
  - [ ] index 切换
  - [ ] 目标不存在
  - [ ] 重复切换
  - [ ] 参数缺失
  - [ ] 边界索引
- Integration Cases:
  - [ ] 切换后写入目标会话
- E2E Smoke:
  - [ ] 会话切换后上下文正确
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T16 - /history
- Date:
- Env:
- Scope: history with limits
- Unit Cases:
  - [ ] 顺序正确
  - [ ] limit 生效
  - [ ] 空历史提示
  - [ ] 角色显示
  - [ ] 超长截断
  - [ ] 非法 limit
- Integration Cases:
  - [ ] 会话切换历史隔离
- E2E Smoke:
  - [ ] 历史查询成功
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T17 - 命令注册中心
- Date:
- Env:
- Scope: registry and metadata
- Unit Cases:
  - [ ] 注册成功
  - [ ] 别名解析
  - [ ] 重名冲突
  - [ ] 查无命令
  - [ ] 批量注册
  - [ ] 顺序稳定
- Integration Cases:
  - [ ] 路由器按注册执行
- E2E Smoke:
  - [ ] 命令表可展示
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T18 - / 补全 v1
- Date:
- Env:
- Scope: prefix completion
- Unit Cases:
  - [ ] 前缀匹配
  - [ ] 空前缀返回
  - [ ] 无匹配处理
  - [ ] 别名检索
  - [ ] 排序稳定
  - [ ] 大小写策略
- Integration Cases:
  - [ ] `/mo` 候选包含 `/model`
- E2E Smoke:
  - [ ] 输入 `/` 显示候选
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T19 - Tab 接受补全
- Date:
- Env:
- Scope: tab accept flow
- Unit Cases:
  - [ ] 单候选接受
  - [ ] 多候选接受当前项
  - [ ] 无候选保持不变
  - [ ] 光标位置保持
  - [ ] 尾随空格处理
  - [ ] 连续 Tab 行为
- Integration Cases:
  - [ ] `/mo + Tab -> /model`
- E2E Smoke:
  - [ ] 补全后可执行命令
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T20 - 候选导航与取消
- Date:
- Env:
- Scope: arrow navigation and esc close
- Unit Cases:
  - [ ] 下方向循环
  - [ ] 上方向循环
  - [ ] Esc 清空候选
  - [ ] 无候选时按键忽略
  - [ ] 焦点切换
  - [ ] 状态复位
- Integration Cases:
  - [ ] 导航后 Tab 命中正确项
- E2E Smoke:
  - [ ] 导航与取消流程可用
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T21 - 频次排序
- Date:
- Env:
- Scope: usage frequency ranking
- Unit Cases:
  - [ ] 频次累加
  - [ ] 首次命令初始化
  - [ ] 同频稳定排序
  - [ ] 频次持久化
  - [ ] 重启后排序保留
  - [ ] 上限保护
- Integration Cases:
  - [ ] 高频命令前置
- E2E Smoke:
  - [ ] 连续使用后补全变化
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T22 - /run 只读执行器
- Date:
- Env:
- Scope: read-only run command
- Unit Cases:
  - [ ] 平台命令拼接
  - [ ] 白名单校验
  - [ ] 输出捕获
  - [ ] 退出码映射
  - [ ] stderr 处理
  - [ ] 长输出截断
- Integration Cases:
  - [ ] 执行只读命令成功
- E2E Smoke:
  - [ ] `/run` 可返回输出
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T23 - 风险分级器
- Date:
- Env:
- Scope: risk level classification
- Unit Cases:
  - [ ] low 判定
  - [ ] medium 判定
  - [ ] high 判定
  - [ ] 混合命令判定
  - [ ] 大小写/空白处理
  - [ ] 绕过写法拦截
- Integration Cases:
  - [ ] `/run` 接入风险分类
- E2E Smoke:
  - [ ] 高风险命令路径被识别
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T24 - 确认流程
- Date:
- Env:
- Scope: medium/high approval gate
- Unit Cases:
  - [ ] 确认通过执行
  - [ ] 确认拒绝阻断
  - [ ] 超时默认拒绝
  - [ ] 非法输入重试
  - [ ] high 风险提示
  - [ ] 状态回收
- Integration Cases:
  - [ ] 未确认不能执行高风险
- E2E Smoke:
  - [ ] `reject -> no execution`
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T25 - 审计记录
- Date:
- Env:
- Scope: audit trail and query
- Unit Cases:
  - [ ] 审计入库
  - [ ] 审批状态记录
  - [ ] 结果字段记录
  - [ ] 时间戳格式
  - [ ] 查询过滤
  - [ ] 分页
- Integration Cases:
  - [ ] `/history --audit` 可查询
- E2E Smoke:
  - [ ] 执行后审计可见
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T26 - /add
- Date:
- Env:
- Scope: add context files
- Unit Cases:
  - [ ] 路径规范化
  - [ ] 重复去重
  - [ ] 文件不存在
  - [ ] 目录输入处理
  - [ ] 二进制排除
  - [ ] 编码异常处理
- Integration Cases:
  - [ ] 添加后列表可见
- E2E Smoke:
  - [ ] 添加后提问可引用上下文
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T27 - /drop
- Date:
- Env:
- Scope: remove context files
- Unit Cases:
  - [ ] 按路径移除
  - [ ] 按索引移除
  - [ ] 不存在项处理
  - [ ] 空集合处理
  - [ ] 批量移除
  - [ ] 排序保持
- Integration Cases:
  - [ ] 移除后请求不含该文件
- E2E Smoke:
  - [ ] 移除后上下文更新
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T28 - /files
- Date:
- Env:
- Scope: list context files
- Unit Cases:
  - [ ] 空列表提示
  - [ ] 排序规则
  - [ ] 路径缩略显示
  - [ ] 条数限制
  - [ ] 过滤规则
  - [ ] 输出格式
- Integration Cases:
  - [ ] `/add` 后 `/files` 正确
- E2E Smoke:
  - [ ] 列表可视化输出
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T29 - /grep
- Date:
- Env:
- Scope: read-only grep
- Unit Cases:
  - [ ] pattern 校验
  - [ ] ignore 生效
  - [ ] 行号格式
  - [ ] 空结果提示
  - [ ] 限流处理
  - [ ] 非法正则处理
- Integration Cases:
  - [ ] 项目目录检索成功
- E2E Smoke:
  - [ ] 检索结果可返回
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T30 - /tree
- Date:
- Env:
- Scope: project tree browsing
- Unit Cases:
  - [ ] 树构建
  - [ ] 深度裁剪
  - [ ] ignore 规则
  - [ ] 符号链接处理
  - [ ] 权限错误处理
  - [ ] 输出格式稳定
- Integration Cases:
  - [ ] 指定目录输出正确
- E2E Smoke:
  - [ ] tree 命令可运行
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T31 - 上下文组装器
- Date:
- Env:
- Scope: context assembly to prompt
- Unit Cases:
  - [ ] 拼接顺序
  - [ ] 角色标记
  - [ ] 重复去重
  - [ ] 空上下文直通
  - [ ] 元信息填充
  - [ ] 编码统一
- Integration Cases:
  - [ ] 请求载荷包含上下文
- E2E Smoke:
  - [ ] 上下文问答路径成功
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T32 - Token 预算裁剪
- Date:
- Env:
- Scope: token budget and trimming
- Unit Cases:
  - [ ] 预算计算
  - [ ] 边界等于预算
  - [ ] 超预算截断
  - [ ] 优先级保留
  - [ ] 极小预算降级
  - [ ] 提示文案
- Integration Cases:
  - [ ] 长上下文请求不超限
- E2E Smoke:
  - [ ] 超限时仍可完成请求
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T33 - JSON Schema 命令注册
- Date:
- Env:
- Scope: schema-driven registration
- Unit Cases:
  - [ ] schema 通过
  - [ ] 缺字段报错
  - [ ] 参数类型校验
  - [ ] handler 缺失报错
  - [ ] alias 冲突报错
  - [ ] examples 解析
- Integration Cases:
  - [ ] schema 注册后命令可执行
- E2E Smoke:
  - [ ] schema 命令出现在 help
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T34 - 命令扩展到 18 条
- Date:
- Env:
- Scope: expand command set to 18
- Unit Cases:
  - [ ] 参数解析
  - [ ] 帮助文案
  - [ ] 权限标记
  - [ ] 默认参数
  - [ ] 错误提示
  - [ ] 别名行为
- Integration Cases:
  - [ ] 18 条命令可枚举可执行
- E2E Smoke:
  - [ ] 多命令路径可运行
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T35 - 扩展到 27 条 + 参数补全
- Date:
- Env:
- Scope: full command set + arg completion
- Unit Cases:
  - [ ] 参数候选生成
  - [ ] 上下文相关候选
  - [ ] 无效参数过滤
  - [ ] 子命令补全
  - [ ] 补全与执行一致
  - [ ] 光标位置处理
- Integration Cases:
  - [ ] `/config set` 参数补全
- E2E Smoke:
  - [ ] 命令与参数补全全链路
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T36 - /config
- Date:
- Env:
- Scope: read and set config
- Unit Cases:
  - [ ] 读取配置键
  - [ ] 设置配置键
  - [ ] 类型转换
  - [ ] 只读键保护
  - [ ] 未知键报错
  - [ ] 写回持久化
- Integration Cases:
  - [ ] 重启后配置生效
- E2E Smoke:
  - [ ] 动态修改配置路径可用
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T37 - /export
- Date:
- Env:
- Scope: export session to json/md
- Unit Cases:
  - [ ] 导出格式生成
  - [ ] 文件名规范
  - [ ] 空会话处理
  - [ ] 权限错误处理
  - [ ] 覆盖策略
  - [ ] 编码一致性
- Integration Cases:
  - [ ] 导出文件可读取
- E2E Smoke:
  - [ ] 导出后文件存在且内容正确
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T38 - Shell 统一适配层
- Date:
- Env:
- Scope: cross-platform shell adapter
- Unit Cases:
  - [ ] 平台检测
  - [ ] 命令转义
  - [ ] 环境变量注入
  - [ ] 工作目录切换
  - [ ] 超时终止
  - [ ] 信号处理
- Integration Cases:
  - [ ] Win/Linux 适配行为一致
- E2E Smoke:
  - [ ] 跨平台命令执行通过
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T39 - 打包发布产物
- Date:
- Env:
- Scope: build distributable binaries
- Unit Cases:
  - [ ] 构建配置解析
  - [ ] 版本号注入
  - [ ] 产物命名规则
  - [ ] 缺依赖报错
  - [ ] 入口校验
  - [ ] 参数边界
- Integration Cases:
  - [ ] 产物 `--version` 可执行
- E2E Smoke:
  - [ ] 下载后启动 smoke 成功
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T40 - CI 质量门禁
- Date:
- Env:
- Scope: CI pipeline and quality gates
- Unit Cases:
  - [ ] CI 配置语法
  - [ ] 覆盖率阈值策略
  - [ ] 任务顺序依赖
  - [ ] 失败重试策略
  - [ ] 缓存策略
  - [ ] 工件归档策略
- Integration Cases:
  - [ ] 模拟 PR 流程通过
- E2E Smoke:
  - [ ] 黄金路径在 CI 成功
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

