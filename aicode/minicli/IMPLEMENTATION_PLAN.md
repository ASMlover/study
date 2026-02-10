# MiniCLI 跨平台实现方案（Windows & Linux，默认 GLM-4.7）

## 摘要
目标是实现一个类似 Claude Code 的轻量 CLI Agent：支持多轮对话、受控工具执行、项目上下文读写、25+ `/` 命令体系，并在用户输入 `/` 时自动补全。  
已锁定偏好：`Node.js/TypeScript`、`GLM 走 OpenAI-compatible API`、`本地 SQLite`、`内置命令 + JSON Schema 注册`、`默认受控执行+确认`、`独立可执行文件分发`。

## 架构设计（决策完备）
- 运行时与构建：
  - Node 22 + TypeScript 5。
  - CLI 框架：`commander`（入口命令）+ 自研 REPL 循环（交互态）。
  - 打包：`esbuild` 产物 + `pkg` 生成 `minicli-win-x64.exe`、`minicli-linux-x64`。
- 核心分层：
  - `src/cli/`：启动、参数解析、交互循环。
  - `src/core/`：会话编排、上下文拼装、token 预算、权限策略。
  - `src/providers/`：模型供应商适配（默认 `GLMProvider`，接口兼容 OpenAI chat）。
  - `src/slash/`：命令注册表、参数 schema、补全引擎、执行器。
  - `src/tools/`：文件系统、shell、grep、git 等工具适配层（统一风险分级）。
  - `src/store/`：SQLite（会话、消息、命令历史、配置快照）。
  - `src/security/`：危险命令检测、确认网关、审计日志。
- 数据流：
  - 用户输入 -> 输入分类（slash/自然语言）-> 上下文组装 -> LLM 调用 -> 工具提议 -> 权限确认 -> 工具执行 -> 回写会话。
- 跨平台执行：
  - Windows 用 `powershell.exe -NoLogo -NoProfile -Command`。
  - Linux 用 `/bin/bash -lc`。
  - 路径统一用 Node `path`，行结束符统一处理。

## 对外接口与类型
- 配置文件：
  - 全局：`~/.minicli/config.toml`
  - 项目：`.minicli/config.toml`（覆盖全局）
- 关键配置项：
  - `model.default = "glm-4.7"`
  - `provider.base_url`, `provider.api_key`
  - `execution.mode = "guarded"`（默认）
  - `approval.high_risk = true`
- 核心 TypeScript 接口：
  - `LLMProvider.chat(req): Promise<ChatResponse>`
  - `Tool.execute(input, ctx): Promise<ToolResult>`
  - `SlashCommand.run(ctx, args): Promise<CommandResult>`
  - `CompletionEngine.suggest(prefix, cursor): Suggestion[]`
- 命令注册 schema（JSON Schema）字段：
  - `name`, `aliases`, `description`, `args`, `riskLevel`, `handlerId`, `examples`

## Slash 命令体系（首版 27 条）
- 会话类：`/new`, `/sessions`, `/switch`, `/rename`, `/clear`, `/history`, `/export`
- 模型类：`/model`, `/models`, `/temp`, `/max-tokens`, `/system`
- 上下文类：`/add`, `/drop`, `/files`, `/tree`, `/grep`
- 工具类：`/run`, `/tool`, `/approve`, `/deny`, `/sandbox`
- 项目类：`/init`, `/status`, `/plan`, `/apply`, `/diff`
- 配置类：`/config`, `/login`, `/logout`, `/help`, `/version`
- 说明：交互输入框检测到首字符 `/` 时，触发补全候选；支持 `Tab` 接受、上下键切换、模糊匹配（前缀优先，编辑距离次之）。

## 自动补全实现细节
- 索引结构：Trie + Fuzzy scorer（`prefixBoost + levenshtein + usageFrequency`）。
- 触发策略：
  - 输入 `/` 后 50ms 防抖计算候选。
  - 输入 `/mo` 返回 `/model`、`/models`。
  - 输入 `/config set` 后进入参数级补全（基于命令 schema）。
- 交互行为：
  - `Tab`：接受当前候选。
  - `Shift+Tab`：反向切换候选。
  - `Esc`：关闭候选面板。
- 学习机制：
  - 每次命令执行更新 `usageFrequency`，常用命令优先展示。

## 安全与权限模型
- 风险分级：
  - `low`（只读查询）直接执行。
  - `medium`（文件写入）二次确认。
  - `high`（删除、网络、批量改写）强制确认并展示 diff/影响摘要。
- 命令黑名单与规则：
  - 默认阻止明显破坏性片段（如 `rm -rf /`、`del /s /q` 模式匹配）。
- 审计：
  - SQLite 记录 `who/when/command/result/approval`，可 `/history --audit` 查询。

## 目录与里程碑
- 目录：
  - `src/cli`, `src/core`, `src/providers`, `src/slash`, `src/tools`, `src/store`, `src/security`, `test/`
- 里程碑（4 周）：
  - 第 1 周：REPL、Provider、SQLite、基础会话。
  - 第 2 周：slash 注册系统、补全引擎、10 条核心命令。
  - 第 3 周：工具执行+权限网关、扩展到 27 条命令。
  - 第 4 周：跨平台打包、稳定性测试、发布流水线。

## 测试方案与验收
- 单元测试（Vitest）：
  - 命令解析、补全排序、风险分级、配置覆盖优先级。
- 集成测试：
  - 伪造 OpenAI-compatible 服务，验证多轮对话与工具回路。
- 端到端测试：
  - Windows/Linux 各执行一次：启动、`/` 补全、`/run` 确认、会话切换、导出。
- 验收标准：
  - 首次安装后 1 分钟内可完成 `login -> /new -> 对话 -> /run(确认) -> /export`。
  - `/` 输入到候选出现的 P95 < 120ms（命令数 30 以内）。
  - 高风险命令 100% 触发确认弹窗，无绕过路径。

## 假设与默认值
- 假设用户持有可用 GLM API Key，且其网关兼容 OpenAI chat 接口。
- 默认模型固定 `glm-4.7`，可通过 `/model` 切换。
- 默认执行模式为 `guarded`，不会启用全自动高风险执行。
- 首版不做第三方 npm 插件加载，仅支持 JSON Schema 注册的内置扩展。
- 分发主形态为独立可执行文件，同时保留 npm 安装作为回退通道（仅文档说明，不作为首发主链路）。
