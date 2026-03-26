# AGENTS.md

## Scope

This repository defines and will implement `mslangc`, a pure C scripting language runtime and toolchain.

## Source of Truth

- Language and architecture: [mslangc-design.md](/C:/Workspace/Repositories/study/aicode/mslangx-c/mslangc-design.md)
- Implementation task breakdown: `docs/01-task-*.md` through `docs/15-task-*.md`

Do not restate architecture decisions in new docs unless necessary. Link to the design doc instead.

## Working Rules

- Use English for all new documentation, code comments, commit messages, and developer-facing text unless a task explicitly requires another language.
- Keep documents concise. Prefer links to existing design/task docs over duplicating content.
- Treat `mslangc-design.md` as the architecture baseline.
- Treat the `docs/` task files as the execution plan and acceptance baseline.
- When changing design, update `mslangc-design.md` first, then adjust affected task docs.
- When implementing code, preserve the pure C direction. Do not introduce C++-only mechanisms.
- Follow the Linux kernel coding style as the default C coding style reference:
  https://www.kernel.org/doc/html/v4.10/process/coding-style.html
- All text files must use UTF-8 encoding and LF line endings.
- Trailing whitespace must be removed automatically before completion.
- Implement all tasks with TDD: write tests first or alongside the implementation, then make the code pass.
- For language features, add complete `.ms` test scripts where appropriate, not only C unit tests.
- A task may be marked complete only when the implementation builds, tests pass, and the feature runs correctly end to end.

## Git Workflow

- If the user input is exactly `git`, stage modified and newly created files under the current working directory, then run `git commit`.
- Do not stage files outside the current working directory when handling the `git` shortcut.
- The `git commit` message must include a gitmoji prefix.

## Document Placement

- Architecture and language rules belong in [mslangc-design.md](/C:/Workspace/Repositories/study/aicode/mslangx-c/mslangc-design.md).
- Task-specific implementation scope belongs in `docs/`.
- Avoid creating extra design documents when a link to existing material is sufficient.

## Expected Output Style

- Be direct.
- Be minimal.
- Prefer checkable acceptance criteria.
- Prefer file references over long prose.
