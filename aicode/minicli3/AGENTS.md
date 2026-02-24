# MiniCLI3 Agent Guidelines / MiniCLI3 代理协作规范

## 1. Repository Contract / 仓库契约
- Project must remain fully standalone. Do not import or copy runtime code from sibling projects.
- Runtime requirement: Python 3.14 or newer.
- All text/code files must use UTF-8 encoding and LF line endings.
- Remove trailing whitespace in edited files.
- Local runtime data must stay under `./.minicli3/`.

## 2. Subagents Model / 子代理模型
This repository uses a role-based subagents workflow. One delivery round should explicitly map work to these roles:

1. `architect-agent`
   - Owns package boundaries, types, and dependency choices.
2. `cli-repl-agent`
   - Owns CLI entrypoints, REPL loop, and terminal interaction.
3. `provider-agent`
   - Owns GLM-5 provider integration, streaming, retries, error mapping.
4. `commands-completion-agent`
   - Owns slash command design and tab-completion behavior.
5. `tools-safety-agent`
   - Owns local tools and restricted safety policy.
6. `qa-release-agent`
   - Owns tests, quality gates, and release/readme completeness.

## 3. Handoff Protocol / 交接协议
Each role handoff must include:
- Input assumptions / 输入假设
- Decisions made / 决策内容
- Changed files / 变更文件
- Risks and rollback points / 风险与回滚点
- Test evidence / 测试证据

## 4. Definition of Done / 完成定义
A milestone can be marked done only when:
- Feature behavior is implemented and manually validated.
- Unit/integration tests are present for critical paths.
- `Plans.md` milestone status is updated.
- User-facing docs are updated when commands/config behavior changes.

## 5. Runtime Policy / 运行策略
- Only GLM-5 is supported (`model` must be `glm-5`).
- Streaming output is enabled by default.
- Default permission mode is `restricted`.
- Tool execution must enforce path and shell allowlist checks.

## 6. Review Gate / 评审门禁
Before finalizing a delivery round:
- Run tests (`pytest -q`).
- Verify CLI startup (`python -m minicli3.cli --help` or equivalent).
- Verify REPL slash help and completion manually.
